#include <blip/config/editor.hpp>
#include <blip/platform/system.hpp>
#include <blip/text/font_manager.hpp>
#include <fontconfig/fontconfig.h>
#include <iostream>

namespace text {
FontManager::FontManager() : current_size(config::defaults::font::SIZE) {
    for (int i = 0; i < static_cast<int>(FontStyles::Count); i++) {
        fonts[i] = nullptr;
    }
}
FontManager::~FontManager() {
    for (int i = 0; i < static_cast<int>(FontStyles::Count); i++) {
        if (fonts[i] != nullptr) {
            TTF_CloseFont(fonts[i]);
            fonts[i] = nullptr;
        }
    }
}

void FontManager::loadStyle(FontStyles style) {
    std::string styleStr;
    int sdlStyleFlag = TTF_STYLE_NORMAL;

    switch (style) {
    case FontStyles::Regular:
        styleStr = "Regular";
        sdlStyleFlag = TTF_STYLE_NORMAL;
        break;
    case FontStyles::Bold:
        styleStr = "Bold";
        sdlStyleFlag = TTF_STYLE_BOLD;
        break;
    case FontStyles::Italic:
        styleStr = "Italic";
        sdlStyleFlag = TTF_STYLE_ITALIC;
        break;
    case FontStyles::BoldItalic:
        styleStr = "Bold Italic";
        sdlStyleFlag = TTF_STYLE_BOLD | TTF_STYLE_ITALIC;
        break;
    default:
        return;
    }

    int index = static_cast<int>(style);

    std::string path = platform::getTTFPath(current_family, styleStr);

    if (!path.empty()) {
        TTF_Font *font = TTF_OpenFont(path.c_str(), current_size);
        if (font != nullptr) {
            fonts[index] = font;
            return;
        }
    }

    if (style == FontStyles::Regular) {
        fonts[index] = nullptr;
        return;
    }

    std::string regPath = platform::getTTFPath(current_family, "Regular");
    if (!regPath.empty()) {
        TTF_Font *font = TTF_OpenFont(regPath.c_str(), current_size);
        if (font != nullptr) {
            TTF_SetFontStyle(font, sdlStyleFlag);
            fonts[index] = font;
            return;
        }
    }

    fonts[index] = nullptr;
}

bool FontManager::updateFontFamily(const std::string &family, int size) {
    if (family == current_family && size == current_size && fonts[0] != nullptr) {
        return false;
    }

    for (int i = 0; i < static_cast<int>(FontStyles::Count); i++) {
        if (fonts[i] != nullptr) {
            TTF_CloseFont(fonts[i]);
            fonts[i] = nullptr;
        }
    }

    current_family = family;
    current_size = size;

    loadStyle(FontStyles::Regular);
    loadStyle(FontStyles::Bold);
    loadStyle(FontStyles::Italic);
    loadStyle(FontStyles::BoldItalic);

    if (fonts[static_cast<int>(FontStyles::Regular)] == nullptr) {
        std::cerr << "CRITICAL: Could not find or synthesize base font for: " << family << std::endl;
        exit(EXIT_FAILURE);
    }

    return true;
}

TTF_Font *FontManager::getFont(FontStyles style) const { return fonts[static_cast<int>(style)]; }
}
