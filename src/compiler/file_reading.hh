#ifndef FILE_READING_HH_
#define FILE_READING_HH_

#include <optional>
#include <string>

std::optional<std::string> read_file(const std::string& file_path);
bool write_file(const std::string& file_path, const std::string& file_contents);

#endif // FILE_READING_HH_
