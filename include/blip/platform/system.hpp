#pragma once
#include <string>

namespace platform {
std::string getTTFPath(const std::string &a_family, const std::string &a_style);
bool readFile(const char *filename, std::string &lines);
}
