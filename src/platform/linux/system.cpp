#include <blip/platform/system.hpp>
#include <fontconfig/fontconfig.h>
#include <fstream>
#include <iostream>

namespace platform {
bool readFile(const char *fname, std::string &lines) {
    std::string filename{fname};
    std::fstream f{filename};

    if (!f.is_open()) {
        std::cout << "Could not open file " << filename << std::endl;
        return false;
    }

    std::string line = "";
    while (std::getline(f, line)) {
        lines += line + "\n";
    }
    return true;
}

std::string getTTFPath(const std::string &a_family, const std::string &a_style) {
    static bool fc_initialized = false;
    if (!fc_initialized) {
        FcInit();
        fc_initialized = true;
    }

    FcPattern *pat = FcNameParse((const FcChar8 *)(a_family + ":" + a_style).c_str());
    if (pat == NULL) {
        perror("Could not create pattern");
        return "";
    }

    FcConfigSubstitute(NULL, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcResult bestMatch;
    FcPattern *bestPattern = FcFontMatch(NULL, pat, &bestMatch);

    FcChar8 *family, *file, *style;
    std::string resultPath = "";

    if (bestPattern) {
        if (FcPatternGetString(bestPattern, FC_FAMILY, 0, &family) == FcResultMatch &&
            FcPatternGetString(bestPattern, FC_FILE, 0, &file) == FcResultMatch &&
            FcPatternGetString(bestPattern, FC_STYLE, 0, &style) == FcResultMatch) {

            resultPath = (const char *)file;
        }
        FcPatternDestroy(bestPattern);
    }

    FcPatternDestroy(pat);

    return resultPath;
}
}
