#include <blip/platform/system.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace platform {
std::string getTTFPath(std::string &family, std::string &style) {
    std::string file = "";
    for (const auto c : family) {
        if (c != ' ') {
            file += c;
        }
    }
    file += "-" + style + ".ttf";
    // TODO: Create OS Specific implementations for ttf paths
    std::string ttf_path = "/Users/juderozario/Library/Fonts/" + file;
    if (!std::filesystem::exists(ttf_path)) {
        ttf_path = "/Library/Fonts/Arial Unicode.ttf";
    }
    return ttf_path;
}

void readFile(const char *fname, std::string &lines) {
    std::string filename{fname};
    std::fstream f{filename};

    if (!f.is_open()) {
        std::cout << "Could not open file " << filename << std::endl;
        exit(EXIT_FAILURE);
    }

    std::string line = "";
    while (std::getline(f, line)) {
        lines += line + "\n";
    }
}
}
