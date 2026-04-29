#include <fstream>
#include <sstream>

#include <fmt/core.h>
#include <fmt/std.h>

#include "file_reading.hh"

std::optional<std::string> read_file(const std::string& file_path)
{
    std::ifstream file(file_path);

    if (!file.is_open()) {
        fmt::println(stderr, "error: Could not read {}.", file_path);
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    return content;
}
