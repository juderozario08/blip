#include <blip/config/editor.hpp>
#include <blip/platform/system.hpp>
#include <blip/text/font_manager.hpp>
#include <iostream>

namespace text {
FontManager::FontManager() : current_font(nullptr), current_size(config::defaults::font::SIZE) {}
FontManager::~FontManager() {
    if (current_font) {
        TTF_CloseFont(current_font);
    }
}

bool FontManager::updateFont(std::string &family, std::string &style, int size) {
    if (family == current_family && style == current_style && size == current_size) {
        return false;
    }

    std::string path = platform::getTTFPath(family, style);
    TTF_Font *new_font = TTF_OpenFont(path.c_str(), size);

    if (new_font == NULL) {
        std::cerr << "Could not open font " << TTF_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (current_font) {
        TTF_CloseFont(current_font);
    }

    current_font = new_font;
    current_family = family;
    current_style = style;
    current_size = size;
    return true;
}

TTF_Font *FontManager::getFont() const { return current_font; }
}
