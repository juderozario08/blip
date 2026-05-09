#include <SDL.h>
#include <SDL_ttf.h>
#include <blip/buffer/buffer.hpp>
#include <blip/text/font_manager.hpp>
#include <cassert>
#include <iostream>

void test_font_manager_initialization() {
    std::cout << "Running test_font_manager_initialization... ";

    text::FontManager fonts;

    // A fresh manager should have absolutely no fonts loaded
    assert(fonts.getFont(text::FontStyles::Regular) == nullptr);
    assert(fonts.getFont(text::FontStyles::Bold) == nullptr);
    assert(fonts.getFont(text::FontStyles::Italic) == nullptr);
    assert(fonts.getFont(text::FontStyles::BoldItalic) == nullptr);

    std::cout << "PASSED\n";
}

void test_load_valid_font() {
    std::cout << "Running test_load_valid_font... ";

    text::FontManager fonts;

    // Loading a standard system font should succeed
    bool success = fonts.updateFontFamily("Arial", 14);

    assert(success == true);

    // The regular font and bold font should be successfully mapped
    assert(fonts.getFont(text::FontStyles::Regular) != nullptr);
    assert(fonts.getFont(text::FontStyles::Bold) != nullptr);

    std::cout << "PASSED\n";
}

void test_fallback_font_loading() {
    std::cout << "Running test_fallback_font_loading... ";

    text::FontManager fonts;

    // Requesting a font that does not exist.
    // Thanks to CoreText/Fontconfig fallbacks, this should NOT fail.
    bool success = fonts.updateFontFamily("SuperFakeFontThatDoesNotExist", 14);

    // The manager should successfully receive the system's safe default font
    assert(success == true);

    // The pointer must NOT be null. The OS should have caught us!
    assert(fonts.getFont(text::FontStyles::Regular) != nullptr);

    std::cout << "PASSED\n";
}

void test_font_cache_hit() {
    std::cout << "Running test_font_cache_hit... ";

    text::FontManager fonts;

    // First load
    bool first_load = fonts.updateFontFamily("Arial", 14);
    assert(first_load == true);

    // Exact same parameters should hit the cache and return false (no update needed)
    bool second_load = fonts.updateFontFamily("Arial", 14);
    assert(second_load == false);

    // Different size should invalidate the cache and return true (update triggered)
    bool third_load = fonts.updateFontFamily("Arial", 16);
    assert(third_load == true);

    std::cout << "PASSED\n";
}
