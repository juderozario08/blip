#include <SDL.h>
#include <SDL_ttf.h>
#include <blip/buffer/buffer.hpp>
#include <blip/text/font_manager.hpp>
#include <cassert>
#include <iostream>

void test_font_manager_initialization() {
    std::cout << "Running test_font_manager_initialization... ";

    text::FontManager fonts;

    assert(fonts.getFont(text::FontStyles::Regular) == nullptr);
    assert(fonts.getFont(text::FontStyles::Bold) == nullptr);
    assert(fonts.getFont(text::FontStyles::Italic) == nullptr);
    assert(fonts.getFont(text::FontStyles::BoldItalic) == nullptr);

    std::cout << "PASSED\n";
}

void test_load_valid_font() {
    std::cout << "Running test_load_valid_font... ";

    text::FontManager fonts;

    bool success = fonts.updateFontFamily("Arial", 14);

    assert(success == true);

    assert(fonts.getFont(text::FontStyles::Regular) != nullptr);
    assert(fonts.getFont(text::FontStyles::Bold) != nullptr);

    std::cout << "PASSED\n";
}

void test_fallback_font_loading() {
    std::cout << "Running test_fallback_font_loading... ";

    text::FontManager fonts;

    bool success = fonts.updateFontFamily("SuperFakeFontThatDoesNotExist", 14);

    assert(success == true);

    assert(fonts.getFont(text::FontStyles::Regular) != nullptr);

    std::cout << "PASSED\n";
}

void test_font_cache_hit() {
    std::cout << "Running test_font_cache_hit... ";

    text::FontManager fonts;

    bool first_load = fonts.updateFontFamily("Arial", 14);
    assert(first_load == true);

    bool second_load = fonts.updateFontFamily("Arial", 14);
    assert(second_load == false);

    bool third_load = fonts.updateFontFamily("Arial", 16);
    assert(third_load == true);

    std::cout << "PASSED\n";
}
