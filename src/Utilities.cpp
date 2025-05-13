//
// Created by andre on 11/05/2025.
//

#include "Utilities.h"

bool streq(const char* a, const char* b) {
    return std::strcmp(a, b) == 0;
}

std::vector<std::uint8_t> ReadFile(std::filesystem::path shader_path) {
    if (!std::filesystem::exists(shader_path)) return {};
    if (!std::filesystem::is_regular_file(shader_path)) return {};

    std::ifstream file(shader_path, std::ios::binary);
    if (!file.is_open()) return {};

    const std::uint32_t file_size = std::filesystem::file_size(shader_path);
    std::vector<std::uint8_t> buffer(file_size);
    file.read(reinterpret_cast<char*>(buffer.data()), file_size);
    return buffer;
}