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
    if (argc != 3)
        return fail("usage: pe_exact_fixture input output");
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
    if (second == MAXDWORD || second > bytes.size() ||
        first < section_offset + sections.size() * sizeof(IMAGE_SECTION_HEADER))
        return fail("fixture needs two raw-backed sections");
    const size_t gap = (static_cast<size_t>(first) / 4096 + 1) * 4096;
    if (gap > MAXDWORD - bytes.size())
        return fail("fixture is too large");
    Bytes output(bytes.size() + gap);
    std::memcpy(output.data(), bytes.data(), second);
    for (size_t i = 0; i < gap; i++)
        output[second + i] = static_cast<unsigned char>((37 * i + 11) & 255);
    std::memcpy(output.data() + second + gap, bytes.data() + second, bytes.size() - second);

    IMAGE_DATA_DIRECTORY security = {}, debug = {};
    const size_t security_offset = directories + IMAGE_DIRECTORY_ENTRY_SECURITY * sizeof(security);
    if (!read_struct(bytes, security_offset, security) ||
        !read_struct(bytes, directories + IMAGE_DIRECTORY_ENTRY_DEBUG * sizeof(debug), debug))
        return fail("invalid data directories");
    if (security.VirtualAddress >= second) {
        security.VirtualAddress += static_cast<DWORD>(gap);
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
            if (entry.PointerToRawData >= second)
                entry.PointerToRawData += static_cast<DWORD>(gap);
            const size_t offset = debug_offset + i;
            std::memcpy(output.data() + offset + (offset >= second ? gap : 0), &entry,
                        sizeof(entry));
        }
    }
    if (header.PointerToSymbolTable >= second) {
        header.PointerToSymbolTable += static_cast<DWORD>(gap);
        std::memcpy(output.data() + header_offset, &header, sizeof(header));
    }
    for (size_t i = 0; i < sections.size(); i++) {
        auto &section = sections[i];
        if (section.PointerToRawData >= second)
            section.PointerToRawData += static_cast<DWORD>(gap);
        if (section.PointerToRelocations >= second)
            section.PointerToRelocations += static_cast<DWORD>(gap);
        if (section.PointerToLinenumbers >= second)
            section.PointerToLinenumbers += static_cast<DWORD>(gap);
        std::memcpy(output.data() + section_offset + i * sizeof(section), &section,
                    sizeof(section));
    }
    return write_fixture(argv[2], output);
}
