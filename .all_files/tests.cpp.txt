#include "buffer.cpp"
#include <iostream>

int main() {
    std::cout << "--- Starting Piece Table Tests ---\n";

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

    std::cout << "--- All Tests Passed! ---\n";
    return 0;
}
