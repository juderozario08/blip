#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>
#include <blip/platform/system.hpp>
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
    std::string resultPath = "";

    CFStringRef familyName = CFStringCreateWithCString(kCFAllocatorDefault, a_family.c_str(), kCFStringEncodingUTF8);

    CTFontSymbolicTraits traits = 0;
    if (a_style == "Bold") {
        traits |= kCTFontTraitBold;
    } else if (a_style == "Italic") {
        traits |= kCTFontTraitItalic;
    } else if (a_style == "Bold Italic") {
        traits |= (kCTFontTraitBold | kCTFontTraitItalic);
    }

    CFMutableDictionaryRef attributes =
        CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionaryAddValue(attributes, kCTFontFamilyNameAttribute, familyName);

    if (traits != 0) {
        CFMutableDictionaryRef traitsDict =
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFNumberRef symTraits = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &traits);
        CFDictionaryAddValue(traitsDict, kCTFontSymbolicTrait, symTraits);
        CFDictionaryAddValue(attributes, kCTFontTraitsAttribute, traitsDict);

        CFRelease(symTraits);
        CFRelease(traitsDict);
    }

    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attributes);

    CTFontRef matchedFont = CTFontCreateWithFontDescriptor(descriptor, 0.0, NULL);

    if (matchedFont != NULL) {
        CFURLRef url = (CFURLRef)CTFontCopyAttribute(matchedFont, kCTFontURLAttribute);

        if (url != NULL) {
            UInt8 path[PATH_MAX];
            if (CFURLGetFileSystemRepresentation(url, true, path, PATH_MAX)) {
                resultPath = reinterpret_cast<char *>(path);
            }
            CFRelease(url);
        }
        CFRelease(matchedFont);
    }

    CFRelease(descriptor);
    CFRelease(attributes);
    CFRelease(familyName);

    return resultPath;
}
}
