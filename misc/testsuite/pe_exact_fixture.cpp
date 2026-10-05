#include <windows.h>
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
        std::strcmp(mode, "zero-gap") != 0 && std::strcmp(mode, "raw-tail") != 0)
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
    if (std::strcmp(mode, "zero-gap") == 0) {
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

    IMAGE_DATA_DIRECTORY security = {}, debug = {};
    const size_t security_offset = directories + IMAGE_DIRECTORY_ENTRY_SECURITY * sizeof(security);
    if (!read_struct(bytes, security_offset, security) ||
        !read_struct(bytes, directories + IMAGE_DIRECTORY_ENTRY_DEBUG * sizeof(debug), debug))
        return fail("invalid data directories");
    if (security.VirtualAddress >= boundary) {
        security.VirtualAddress = static_cast<DWORD>(security.VirtualAddress + shift);
        std::memcpy(output.data() + security_offset, &security, sizeof(security));
    }
    if (debug.Size) {
        const size_t debug_offset = rva_to_offset(sections, debug.VirtualAddress);
        if (!debug_offset || debug.Size % sizeof(IMAGE_DEBUG_DIRECTORY) != 0)
            return fail("invalid debug directory");
        for (size_t i = 0; i < debug.Size; i += sizeof(IMAGE_DEBUG_DIRECTORY)) {
            IMAGE_DEBUG_DIRECTORY entry = {};
            if (!read_struct(bytes, debug_offset + i, entry))
                return fail("invalid debug entry");
            if (entry.PointerToRawData >= boundary)
                entry.PointerToRawData = static_cast<DWORD>(entry.PointerToRawData + shift);
            const size_t offset = debug_offset + i;
            const size_t destination = static_cast<size_t>(static_cast<LONGLONG>(offset) +
                                                           (offset >= boundary ? shift : 0));
            std::memcpy(output.data() + destination, &entry, sizeof(entry));
        }
    }
    if (header.PointerToSymbolTable >= boundary) {
        header.PointerToSymbolTable = static_cast<DWORD>(header.PointerToSymbolTable + shift);
        std::memcpy(output.data() + header_offset, &header, sizeof(header));
    }
    for (size_t i = 0; i < sections.size(); i++) {
        auto &section = sections[i];
        if (section.PointerToRawData >= boundary)
            section.PointerToRawData = static_cast<DWORD>(section.PointerToRawData + shift);
        if (section.PointerToRelocations >= boundary)
            section.PointerToRelocations = static_cast<DWORD>(section.PointerToRelocations + shift);
        if (section.PointerToLinenumbers >= boundary)
            section.PointerToLinenumbers = static_cast<DWORD>(section.PointerToLinenumbers + shift);
        std::memcpy(output.data() + section_offset + i * sizeof(section), &section,
                    sizeof(section));
    }
    return write_fixture(argv[2], output);
}
