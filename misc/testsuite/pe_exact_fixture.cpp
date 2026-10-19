#include <windows.h>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <vector>

using Bytes = std::vector<unsigned char>;

template <typename T>
static bool read_struct(const Bytes &bytes, size_t offset, T &value) {
    if (offset > bytes.size() || sizeof(T) > bytes.size() - offset)
        return false;
    std::memcpy(&value, bytes.data() + offset, sizeof(T));
    return true;
}

static int fail(const char *message) {
    std::fprintf(stderr, "%s\n", message);
    return 1;
}

static int write_fixture(const char *path, const Bytes &bytes) {
    std::ofstream result(path, std::ios::binary);
    if (!result.write(reinterpret_cast<const char *>(bytes.data()), bytes.size()))
        return fail("cannot write fixture");
    return 0;
}

static size_t rva_to_offset(const std::vector<IMAGE_SECTION_HEADER> &sections, DWORD rva) {
    for (const auto &section : sections)
        if (section.PointerToRawData && rva >= section.VirtualAddress &&
            rva - section.VirtualAddress < section.SizeOfRawData)
            return static_cast<size_t>(section.PointerToRawData) + (rva - section.VirtualAddress);
    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3 && argc != 4)
        return fail("usage: pe_exact_fixture input output [mode]");
    const char *mode = argc == 4 ? argv[3] : "gap";
    if (std::strcmp(mode, "gap") != 0 && std::strcmp(mode, "empty-imports") != 0 &&
        std::strcmp(mode, "zero-gap") != 0 && std::strcmp(mode, "raw-tail") != 0 &&
        std::strcmp(mode, "tls-index") != 0 && std::strcmp(mode, "zero-sections") != 0 &&
        std::strcmp(mode, "unordered") != 0 && std::strcmp(mode, "wrap-rva") != 0 &&
        std::strcmp(mode, "overlay") != 0 && std::strcmp(mode, "guard-cf") != 0 &&
        std::strcmp(mode, "cet") != 0 && std::strcmp(mode, "wince") != 0)
        return fail("unknown fixture mode");
    std::ifstream input(argv[1], std::ios::binary | std::ios::ate);
    const std::streamoff size = input.tellg();
    if (size < static_cast<std::streamoff>(sizeof(IMAGE_DOS_HEADER)) || size > MAXDWORD)
        return fail("invalid fixture size");
    Bytes bytes(static_cast<size_t>(size));
    input.seekg(0);
    if (!input.read(reinterpret_cast<char *>(bytes.data()), size))
        return fail("cannot read fixture");

    IMAGE_DOS_HEADER dos = {};
    DWORD signature = 0;
    IMAGE_FILE_HEADER header = {};
    if (!read_struct(bytes, 0, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < 0 ||
        !read_struct(bytes, dos.e_lfanew, signature) || signature != IMAGE_NT_SIGNATURE ||
        !read_struct(bytes, dos.e_lfanew + sizeof(signature), header))
        return fail("invalid PE fixture");
    const size_t header_offset = dos.e_lfanew + sizeof(signature);
    const size_t optional_offset = header_offset + sizeof(header);
    const size_t section_offset = optional_offset + header.SizeOfOptionalHeader;
    std::vector<IMAGE_SECTION_HEADER> sections(header.NumberOfSections);
    DWORD first = MAXDWORD, second = MAXDWORD;
    for (size_t i = 0; i < sections.size(); i++) {
        if (!read_struct(bytes, section_offset + i * sizeof(sections[i]), sections[i]))
            return fail("invalid section table");
        const DWORD offset = sections[i].PointerToRawData;
        if (!offset || !sections[i].SizeOfRawData)
            continue;
        if (offset < first) {
            second = first;
            first = offset;
        } else if (offset < second) {
            second = offset;
        }
    }
    WORD magic = 0;
    if (!read_struct(bytes, optional_offset, magic) ||
        (magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC && magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC))
        return fail("invalid optional header");
    const size_t directories =
        optional_offset + (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC
                               ? offsetof(IMAGE_OPTIONAL_HEADER64, DataDirectory)
                               : offsetof(IMAGE_OPTIONAL_HEADER32, DataDirectory));
    if (std::strcmp(mode, "zero-sections") == 0) {
        header.NumberOfSections = 0;
        std::memcpy(bytes.data() + header_offset, &header, sizeof(header));
        return write_fixture(argv[2], bytes);
    }
    if (std::strcmp(mode, "wrap-rva") == 0) {
        if (sections.empty() || sections.back().SizeOfRawData <= 15)
            return fail("fixture needs raw data for a wrapping RVA");
        auto &last = sections.back();
        last.VirtualAddress = MAXDWORD - 15;
        last.Misc.VirtualSize = 1;
        std::memcpy(bytes.data() + section_offset + (sections.size() - 1) * sizeof(last), &last,
                    sizeof(last));
        return write_fixture(argv[2], bytes);
    }
    if (std::strcmp(mode, "overlay") == 0) {
        if (sections.empty() || bytes.size() > MAXDWORD - 513)
            return fail("invalid overlay fixture size");
        auto &last = sections.back();
        const ULONGLONG raw_end =
            static_cast<ULONGLONG>(last.PointerToRawData) + last.SizeOfRawData;
        DWORD file_alignment = 0;
        if (!read_struct(bytes, optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, FileAlignment),
                         file_alignment) ||
            !file_alignment || !last.PointerToRawData || raw_end > bytes.size() ||
            last.Misc.VirtualSize >= last.SizeOfRawData || (raw_end - 1) % file_alignment == 0)
            return fail("fixture needs last-section padding with an unaligned end");
        last.SizeOfRawData--;
        std::memcpy(bytes.data() + section_offset + (sections.size() - 1) * sizeof(last), &last,
                    sizeof(last));
        for (size_t i = 0; i < 513; i++)
            bytes.push_back(static_cast<unsigned char>((37 * i + 11) & 255));
        return write_fixture(argv[2], bytes);
    }
    if (std::strcmp(mode, "guard-cf") == 0) {
        IMAGE_DATA_DIRECTORY directory = {};
        WORD dll_flags = 0;
        const size_t directory_offset =
            directories + IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG * sizeof(directory);
        const size_t flags_offset =
            optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, DllCharacteristics);
        const size_t pointer_size = magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC ? 8 : 4;
        const size_t guard_offset = 14 * pointer_size + 6 * sizeof(DWORD) + 4 * sizeof(WORD);
        const DWORD config_size = static_cast<DWORD>(guard_offset + sizeof(DWORD));
        if (!read_struct(bytes, directory_offset, directory) ||
            !read_struct(bytes, flags_offset, dll_flags))
            return fail("invalid load configuration");
        size_t offset =
            directory.Size >= config_size ? rva_to_offset(sections, directory.VirtualAddress) : 0;
        if (!offset) {
            // GNU ld emits no load configuration and some linkers stop before GuardFlags.
            // Add a zeroed one in zero raw padding, as the CET fixture does for its debug entries.
            for (size_t i = 0; i < sections.size() && !offset; i++) {
                auto &section = sections[i];
                if (!section.PointerToRawData || !section.Misc.VirtualSize ||
                    (section.Characteristics & IMAGE_SCN_MEM_DISCARDABLE))
                    continue;
                const ULONGLONG padding =
                    (static_cast<ULONGLONG>(section.Misc.VirtualSize) + 7) & ~ULONGLONG(7);
                const ULONGLONG end = padding + config_size;
                const size_t start = static_cast<size_t>(section.PointerToRawData + padding);
                if (end > section.SizeOfRawData || section.VirtualAddress + end > MAXDWORD ||
                    start > bytes.size() || config_size > bytes.size() - start ||
                    std::any_of(bytes.data() + start, bytes.data() + start + config_size,
                                [](unsigned char value) { return value != 0; }))
                    continue;
                directory.VirtualAddress = section.VirtualAddress + static_cast<DWORD>(padding);
                directory.Size = config_size;
                std::memcpy(bytes.data() + start, &config_size, sizeof(config_size));
                std::memcpy(bytes.data() + directory_offset, &directory, sizeof(directory));
                section.Misc.VirtualSize = static_cast<DWORD>(end);
                std::memcpy(bytes.data() + section_offset + i * sizeof(section), &section,
                            sizeof(section));
                offset = start;
            }
            if (!offset)
                return fail("fixture needs raw padding for a load configuration");
        }
        if (offset > bytes.size() || config_size > bytes.size() - offset)
            return fail("invalid load configuration range");
        // The loader rejects CF_INSTRUMENTED unless there is a check-function pointer to patch.
        const unsigned char *check = bytes.data() + offset + guard_offset - 4 * pointer_size;
        const bool instrumented = std::any_of(check, check + pointer_size,
                                              [](unsigned char value) { return value != 0; });
        const DWORD guard_flags = instrumented ? 0x500 : 0x400;
        dll_flags |= IMAGE_DLLCHARACTERISTICS_GUARD_CF;
        std::memcpy(bytes.data() + offset + guard_offset, &guard_flags, sizeof(guard_flags));
        std::memcpy(bytes.data() + flags_offset, &dll_flags, sizeof(dll_flags));
        return write_fixture(argv[2], bytes);
    }
    if (std::strcmp(mode, "cet") == 0) {
        if (bytes.size() > MAXDWORD - sizeof(DWORD))
            return fail("invalid CET fixture size");
        IMAGE_DEBUG_DIRECTORY entries[2] = {};
        entries[1].Type = 20; // IMAGE_DEBUG_TYPE_EX_DLLCHARACTERISTICS
        entries[1].SizeOfData = sizeof(DWORD);
        entries[1].PointerToRawData = static_cast<DWORD>(bytes.size());
        for (size_t i = 0; i < sections.size(); i++) {
            auto &section = sections[i];
            if (!section.PointerToRawData || !section.Misc.VirtualSize)
                continue;
            const ULONGLONG padding =
                (static_cast<ULONGLONG>(section.Misc.VirtualSize) + 3) & ~ULONGLONG(3);
            const ULONGLONG end = padding + sizeof(entries);
            if (end > section.SizeOfRawData)
                continue;
            if (section.PointerToRawData + end > bytes.size() ||
                section.VirtualAddress + end > MAXDWORD)
                return fail("invalid CET directory range");
            IMAGE_DATA_DIRECTORY directory = {};
            directory.VirtualAddress = section.VirtualAddress + static_cast<DWORD>(padding);
            directory.Size = sizeof(entries);
            std::memcpy(bytes.data() + section.PointerToRawData + padding, entries,
                        sizeof(entries));
            std::memcpy(bytes.data() + directories +
                            IMAGE_DIRECTORY_ENTRY_DEBUG * sizeof(directory),
                        &directory, sizeof(directory));
            section.Misc.VirtualSize = static_cast<DWORD>(end);
            std::memcpy(bytes.data() + section_offset + i * sizeof(section), &section,
                        sizeof(section));
            const DWORD flags = 1; // IMAGE_DLLCHARACTERISTICS_EX_CET_COMPAT
            const size_t payload = bytes.size();
            bytes.resize(payload + sizeof(flags));
            std::memcpy(bytes.data() + payload, &flags, sizeof(flags));
            return write_fixture(argv[2], bytes);
        }
        return fail("fixture needs raw padding for CET debug entries");
    }
    if (std::strcmp(mode, "wince") == 0) {
        IMAGE_OPTIONAL_HEADER32 optional = {};
        if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            IMAGE_OPTIONAL_HEADER64 source = {};
            if (!read_struct(bytes, optional_offset, source))
                return fail("invalid source optional header");
            std::memcpy(&optional, &source, offsetof(IMAGE_OPTIONAL_HEADER32, BaseOfData));
            std::memcpy(&optional.SectionAlignment, &source.SectionAlignment,
                        offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfStackReserve) -
                            offsetof(IMAGE_OPTIONAL_HEADER32, SectionAlignment));
            optional.ImageBase = 0x10000;
            optional.SizeOfStackReserve = 0x100000;
            optional.SizeOfStackCommit = 0x1000;
            optional.SizeOfHeapReserve = 0x100000;
            optional.SizeOfHeapCommit = 0x1000;
            optional.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
        } else if (!read_struct(bytes, optional_offset, optional)) {
            return fail("invalid source optional header");
        }
        optional.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
        optional.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_CE_GUI;
        header.Machine = IMAGE_FILE_MACHINE_ARM;
        header.Characteristics |= IMAGE_FILE_32BIT_MACHINE;
        header.SizeOfOptionalHeader = sizeof(optional);
        std::memcpy(bytes.data() + header_offset, &header, sizeof(header));
        std::memcpy(bytes.data() + optional_offset, &optional, sizeof(optional));
        for (size_t i = 0; i < sections.size(); i++)
            std::memcpy(bytes.data() + optional_offset + sizeof(optional) + i * sizeof(sections[i]),
                        &sections[i], sizeof(sections[i]));
        return write_fixture(argv[2], bytes);
    }
    if (std::strcmp(mode, "tls-index") == 0) {
        IMAGE_DATA_DIRECTORY directory = {};
        if (!read_struct(bytes, directories + IMAGE_DIRECTORY_ENTRY_TLS * sizeof(directory),
                         directory) ||
            !directory.VirtualAddress)
            return fail("fixture has no TLS directory");
        size_t offset = rva_to_offset(sections, directory.VirtualAddress);
        ULONGLONG image_base = 0, index_address = 0;
        if (magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
            IMAGE_OPTIONAL_HEADER64 optional = {};
            IMAGE_TLS_DIRECTORY64 tls = {};
            if (!offset || !read_struct(bytes, optional_offset, optional) ||
                !read_struct(bytes, offset, tls))
                return fail("invalid TLS directory");
            image_base = optional.ImageBase;
            index_address = tls.AddressOfIndex;
        } else {
            IMAGE_OPTIONAL_HEADER32 optional = {};
            IMAGE_TLS_DIRECTORY32 tls = {};
            if (!offset || !read_struct(bytes, optional_offset, optional) ||
                !read_struct(bytes, offset, tls))
                return fail("invalid TLS directory");
            image_base = optional.ImageBase;
            index_address = tls.AddressOfIndex;
        }
        if (index_address < image_base || index_address - image_base > MAXDWORD)
            return fail("invalid TLS index address");
        const DWORD rva = static_cast<DWORD>(index_address - image_base);
        offset = rva_to_offset(sections, rva);
        if (!offset) {
            // The linker left the index in BSS or past the raw data of its section, so there is
            // no file byte to retain. Append a raw copy of the section covering the index and
            // keep the old bytes in place, so that no other offset moves.
            DWORD file_alignment = 0;
            if (!read_struct(bytes,
                             optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, FileAlignment),
                             file_alignment) ||
                !file_alignment)
                return fail("invalid fixture alignment");
            const auto align = [=](ULONGLONG value) {
                return (value + file_alignment - 1) / file_alignment * file_alignment;
            };
            const ULONGLONG raw_start = align(bytes.size());
            for (size_t i = 0; i < sections.size() && !offset; i++) {
                auto &section = sections[i];
                if (rva < section.VirtualAddress)
                    continue;
                const ULONGLONG used =
                    static_cast<ULONGLONG>(rva - section.VirtualAddress) + sizeof(DWORD);
                if (used > section.Misc.VirtualSize && used > section.SizeOfRawData)
                    continue;
                const ULONGLONG raw_size = align(used);
                if (raw_start + raw_size > MAXDWORD ||
                    (section.PointerToRawData &&
                     static_cast<ULONGLONG>(section.PointerToRawData) + section.SizeOfRawData >
                         bytes.size()))
                    return fail("invalid raw data for the TLS index section");
                bytes.resize(static_cast<size_t>(raw_start + raw_size));
                if (section.PointerToRawData)
                    std::memcpy(bytes.data() + raw_start, bytes.data() + section.PointerToRawData,
                                section.SizeOfRawData);
                section.PointerToRawData = static_cast<DWORD>(raw_start);
                section.SizeOfRawData = static_cast<DWORD>(raw_size);
                section.Characteristics &= ~IMAGE_SCN_CNT_UNINITIALIZED_DATA;
                section.Characteristics |= IMAGE_SCN_CNT_INITIALIZED_DATA;
                std::memcpy(bytes.data() + section_offset + i * sizeof(section), &section,
                            sizeof(section));
                offset = static_cast<size_t>(raw_start) + (rva - section.VirtualAddress);
            }
            if (!offset)
                return fail("TLS index is not in a section");
        }
        DWORD initializer = 0;
        if (!read_struct(bytes, offset, initializer))
            return fail("TLS index is not raw-backed");
        initializer = 0x55;
        std::memcpy(bytes.data() + offset, &initializer, sizeof(initializer));
        return write_fixture(argv[2], bytes);
    }
    if (std::strcmp(mode, "empty-imports") == 0) {
        IMAGE_DATA_DIRECTORY imports = {};
        const size_t directory_offset =
            directories + IMAGE_DIRECTORY_ENTRY_IMPORT * sizeof(imports);
        if (!read_struct(bytes, directory_offset, imports))
            return fail("invalid import directory");
        const size_t offset = rva_to_offset(sections, imports.VirtualAddress);
        const size_t descriptors_size = 2 * sizeof(IMAGE_IMPORT_DESCRIPTOR);
        if (!offset || offset > bytes.size() || descriptors_size > bytes.size() - offset)
            return fail("invalid import descriptors");
        std::memset(bytes.data() + offset, 0, descriptors_size);
        imports.Size = descriptors_size;
        std::memcpy(bytes.data() + directory_offset, &imports, sizeof(imports));
        HMODULE module = LoadLibraryExA(argv[1], nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (!module)
            return fail("cannot load source fixture");
        FARPROC entry = nullptr;
        for (const char *name : {"DllMain", "DllMain@12", "_DllMain@12"})
            if ((entry = GetProcAddress(module, name)) != nullptr)
                break;
        const ULONG_PTR entry_rva =
            reinterpret_cast<ULONG_PTR>(entry) - reinterpret_cast<ULONG_PTR>(module);
        if (!FreeLibrary(module))
            return fail("cannot unload source fixture");
        if (!entry || entry_rva > MAXDWORD)
            return fail("cannot locate fixture entry point");
        const DWORD address = static_cast<DWORD>(entry_rva);
        std::memcpy(bytes.data() + optional_offset +
                        offsetof(IMAGE_OPTIONAL_HEADER32, AddressOfEntryPoint),
                    &address, sizeof(address));
        for (const unsigned index : {IMAGE_DIRECTORY_ENTRY_TLS, IMAGE_DIRECTORY_ENTRY_IAT}) {
            IMAGE_DATA_DIRECTORY directory = {};
            if (!read_struct(bytes, directories + index * sizeof(directory), directory))
                return fail("invalid data directory");
            directory = {};
            std::memcpy(bytes.data() + directories + index * sizeof(directory), &directory,
                        sizeof(directory));
        }
        return write_fixture(argv[2], bytes);
    }
    if (second == MAXDWORD || second > bytes.size() ||
        first < section_offset + sections.size() * sizeof(IMAGE_SECTION_HEADER))
        return fail("fixture needs two raw-backed sections");
    size_t boundary = second, removed = 0;
    size_t gap = (static_cast<size_t>(first) / 4096 + 1) * 4096;
    DWORD swap_first = 0, swap_second = 0, swap_size = 0;
    if (std::strcmp(mode, "unordered") == 0) {
        for (size_t i = 0; i < sections.size() && !swap_size; i++)
            for (size_t j = i + 1; j < sections.size(); j++) {
                const auto &a = sections[i];
                const auto &b = sections[j];
                if (!a.PointerToRawData || a.PointerToRawData >= b.PointerToRawData ||
                    !a.SizeOfRawData || a.SizeOfRawData != b.SizeOfRawData)
                    continue;
                swap_first = a.PointerToRawData;
                swap_second = b.PointerToRawData;
                swap_size = a.SizeOfRawData;
                if (static_cast<ULONGLONG>(swap_first) + swap_size > swap_second ||
                    static_cast<ULONGLONG>(swap_second) + swap_size > bytes.size())
                    return fail("invalid raw ranges for section swap");
                break;
            }
        if (!swap_size)
            return fail("fixture needs two equal-sized raw-backed sections");
        gap = 0;
    } else if (std::strcmp(mode, "zero-gap") == 0) {
        boundary = first;
        const DWORD end =
            static_cast<DWORD>(section_offset + sections.size() * sizeof(IMAGE_SECTION_HEADER));
        removed = first - end;
        gap = 0;
        std::memcpy(bytes.data() + optional_offset +
                        offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfHeaders),
                    &end, sizeof(end));
    } else if (std::strcmp(mode, "raw-tail") == 0) {
        DWORD alignment = 0, file_alignment = 0, image_size = 0;
        if (!read_struct(bytes,
                         optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, SectionAlignment),
                         alignment) ||
            !read_struct(bytes, optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, FileAlignment),
                         file_alignment) ||
            !read_struct(bytes, optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfImage),
                         image_size) ||
            !alignment || !file_alignment)
            return fail("invalid fixture alignment");
        auto &last = sections.back();
        if (!last.PointerToRawData)
            return fail("fixture needs a raw-backed last section");
        const ULONGLONG virtual_size =
            (static_cast<ULONGLONG>(last.Misc.VirtualSize) + alignment - 1) / alignment * alignment;
        const ULONGLONG raw_size = virtual_size + file_alignment;
        const ULONGLONG image_end =
            (last.VirtualAddress + raw_size + alignment - 1) / alignment * alignment;
        if (raw_size > MAXDWORD || image_end > MAXDWORD || raw_size <= last.SizeOfRawData)
            return fail("invalid raw-tail fixture size");
        boundary = static_cast<size_t>(last.PointerToRawData) + last.SizeOfRawData;
        gap = static_cast<size_t>(raw_size) - last.SizeOfRawData;
        last.SizeOfRawData = static_cast<DWORD>(raw_size);
        if (image_end > image_size)
            image_size = static_cast<DWORD>(image_end);
        std::memcpy(bytes.data() + optional_offset + offsetof(IMAGE_OPTIONAL_HEADER32, SizeOfImage),
                    &image_size, sizeof(image_size));
    }
    if (boundary > bytes.size() || removed > boundary || gap > MAXDWORD - bytes.size())
        return fail("fixture is too large");
    const size_t prefix = boundary - removed;
    const LONGLONG shift = static_cast<LONGLONG>(gap) - static_cast<LONGLONG>(removed);
    Bytes output(bytes.size() + gap - removed);
    std::memcpy(output.data(), bytes.data(), prefix);
    for (size_t i = 0; i < gap; i++)
        output[prefix + i] = static_cast<unsigned char>((37 * i + 11) & 255);
    std::memcpy(output.data() + prefix + gap, bytes.data() + boundary, bytes.size() - boundary);
    if (swap_size)
        std::swap_ranges(output.begin() + swap_first, output.begin() + swap_first + swap_size,
                         output.begin() + swap_second);
    const auto remap = [=](DWORD offset) -> DWORD {
        if (swap_size && offset >= swap_first && offset - swap_first < swap_size)
            return swap_second + (offset - swap_first);
        if (swap_size && offset >= swap_second && offset - swap_second < swap_size)
            return swap_first + (offset - swap_second);
        return offset >= boundary ? static_cast<DWORD>(offset + shift) : offset;
    };

    IMAGE_DATA_DIRECTORY security = {}, debug = {};
    const size_t security_offset = directories + IMAGE_DIRECTORY_ENTRY_SECURITY * sizeof(security);
    if (!read_struct(bytes, security_offset, security) ||
        !read_struct(bytes, directories + IMAGE_DIRECTORY_ENTRY_DEBUG * sizeof(debug), debug))
        return fail("invalid data directories");
    security.VirtualAddress = remap(security.VirtualAddress);
    std::memcpy(output.data() + security_offset, &security, sizeof(security));
    if (debug.Size) {
        const size_t debug_offset = rva_to_offset(sections, debug.VirtualAddress);
        if (!debug_offset || debug.Size % sizeof(IMAGE_DEBUG_DIRECTORY) != 0)
            return fail("invalid debug directory");
        for (size_t i = 0; i < debug.Size; i += sizeof(IMAGE_DEBUG_DIRECTORY)) {
            IMAGE_DEBUG_DIRECTORY entry = {};
            if (!read_struct(bytes, debug_offset + i, entry))
                return fail("invalid debug entry");
            entry.PointerToRawData = remap(entry.PointerToRawData);
            const size_t offset = debug_offset + i;
            const size_t destination = remap(static_cast<DWORD>(offset));
            std::memcpy(output.data() + destination, &entry, sizeof(entry));
        }
    }
    header.PointerToSymbolTable = remap(header.PointerToSymbolTable);
    std::memcpy(output.data() + header_offset, &header, sizeof(header));
    for (size_t i = 0; i < sections.size(); i++) {
        auto &section = sections[i];
        section.PointerToRawData = remap(section.PointerToRawData);
        section.PointerToRelocations = remap(section.PointerToRelocations);
        section.PointerToLinenumbers = remap(section.PointerToLinenumbers);
        std::memcpy(output.data() + section_offset + i * sizeof(section), &section,
                    sizeof(section));
    }
    return write_fixture(argv[2], output);
}
