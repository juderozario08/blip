#include <blip/platform/system.hpp>
#include <fstream>
#include <sstream>

namespace platform {
bool readFile(const char *fname, std::string &lines) {
    std::ifstream f{fname, std::ios::binary};

    if (!f.is_open()) {
        return false;
    }

    std::ostringstream ss;
    ss << f.rdbuf();
    lines = ss.str();

    return true;
}
}
