#ifndef FILE_H
#define FILE_H

#include <string>
#include <stdexcept>
#include <filesystem>

namespace fs = std::filesystem;

std::string readFileAsBytes(const std::string& filename);
void writeFileAsBytes(const std::string& filename, const std::string& content);
bool validateFilePath(const std::string& path, bool checkExists = true);
bool ensureFileExists(std::string& filePath);
std::string readFromConsole();

#endif
