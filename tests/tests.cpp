#include "buffer.cpp"
#include "font_manager.cpp"
#include <iostream>

int main() {
    std::cout << "--- Starting Piece Table Tests ---\n";

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL Failed to init for tests: " << SDL_GetError() << "\n";
        return 1;
    }
    if (TTF_Init() < 0) {
        std::cerr << "SDL_ttf Failed to init for tests: " << TTF_GetError() << "\n";
        return 1;
    }

    std::cout << "[Buffer & Piece Table]\n";
    test_initialization();
    test_empty_initialization();
    test_insert_beginning();
    test_insert_middle();
    test_insert_end();
    test_erase_hole_punch();
    test_erase_spanning_pieces();
    test_erase_swallow_piece();
    test_consecutive_inserts();
    test_undo_redo();
    test_get_character_from_cursor();
    std::cout << "\n";

    std::cout << "[Font Manager]\n";
    test_font_manager_initialization();
    test_load_valid_font();
    test_fallback_font_loading();
    test_font_cache_hit();
    std::cout << "\n";

    TTF_Quit();
    SDL_Quit();

    std::cout << "--- All Tests Passed! ---\n";
    return 0;
}
