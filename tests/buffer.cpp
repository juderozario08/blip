#pragma once
#include <blip/buffer/buffer.hpp>
#include <cassert>
#include <iostream>

void test_initialization() {
    std::cout << "Running test_initialization... ";

    std::string initial_content = "Hello World";
    buffer::PieceTable pt(initial_content);

    assert(pt.getText() == "Hello World");

    std::cout << "PASSED\n";
}

void test_empty_initialization() {
    std::cout << "Running test_empty_initialization... ";

    buffer::PieceTable pt;
    assert(pt.getText() == "");

    std::cout << "PASSED\n";
}

void test_insert_end() {
    std::cout << "Running test_insert_end...";
    buffer::PieceTable pt("Hello");
    pt.insert(5, " World");
    assert(pt.getText() == "Hello World");
    std::cout << "PASSED" << std::endl;
}

void test_insert_middle() {
    std::cout << "Running test_insert_middle...";
    buffer::PieceTable pt("HelloWorld");
    pt.insert(5, " ");
    assert(pt.getText() == "Hello World");
    std::cout << "PASSED" << std::endl;
}

void test_insert_beginning() {
    std::cout << "Running test_insert_beginning...";
    buffer::PieceTable pt("World");
    pt.insert(0, "Hello ");
    assert(pt.getText() == "Hello World");
    std::cout << "PASSED" << std::endl;
}

void test_erase_hole_punch() {
    std::cout << "Running test_erase_hole_punch...";
    buffer::PieceTable pt("Hello World");
    pt.erase(3, 5);
    assert(pt.getText() == "lo World");
    std::cout << "PASSED" << std::endl;
}

void test_erase_spanning_pieces() {
    std::cout << "Running test_erase_spanning_pieces...";
    buffer::PieceTable pt("Hello");
    pt.insert(5, " Beautiful");
    pt.insert(15, " World");
    pt.erase(17, 14);
    assert(pt.getText() == "Helorld");
    std::cout << "PASSED" << std::endl;
}

void test_erase_swallow_piece() {
    std::cout << "Running test_erase_swallow_piece...";
    buffer::PieceTable pt("A");
    pt.insert(1, "B");
    pt.insert(2, "C");
    pt.erase(2, 1);
    assert(pt.getText() == "AC");
    std::cout << "PASSED" << std::endl;
}

void test_consecutive_inserts() {
    std::cout << "Running test_consecutive_inserts...";
    buffer::PieceTable pt("World");
    pt.insert(0, "H");
    pt.insert(1, "e");
    pt.insert(2, "l");
    pt.insert(3, "l");
    pt.insert(4, "o");
    pt.insert(5, " ");

    assert(pt.getText() == "Hello World");
    assert(pt.getPieceCount() == 2);

    std::cout << "PASSED" << std::endl;
}

void test_undo_redo() {
    std::cout << "Running test_undo_redo... ";

    // Empty Original Buffer
    buffer::EditorBuffer buffer_empty;

    buffer_empty.insertText("hello");
    assert(buffer_empty.getText() == "hello");

    buffer_empty.undo();
    assert(buffer_empty.getText() == "");

    buffer_empty.redo();
    assert(buffer_empty.getText() == "hello");

    // Filled Original Buffer
    buffer::EditorBuffer buffer_org("Hello");

    buffer_org.setCursor(5);
    buffer_org.insertText(" World");
    assert(buffer_org.getText() == "Hello World");

    buffer_org.undo();
    assert(buffer_org.getText() == "Hello");

    buffer_org.redo();
    assert(buffer_org.getText() == "Hello World");

    buffer_org.commit();

    buffer_org.setCursor(5);
    buffer_org.insertText(" Beautiful");
    assert(buffer_org.getText() == "Hello Beautiful World");

    buffer_org.undo();
    assert(buffer_org.getText() == "Hello World");

    buffer_org.redo();
    assert(buffer_org.getText() == "Hello Beautiful World");

    std::cout << "PASSED" << std::endl;
}

void test_get_character_from_cursor() {
    std::cout << "Runnning test_get_character_from_cursor...";

    buffer::PieceTable pt;

    std::optional<char> t;
    t = pt.getCharacterFromCursor(0, 5);
    assert(t == std::nullopt);

    pt.insert(0, "Hello World");

    t = pt.getCharacterFromCursor(0, 5);
    assert(t == ' ');

    t = pt.getCharacterFromCursor(5, -3);
    assert(t == 'l');

    t = pt.getCharacterFromCursor(5, 0);
    assert(t == ' ');

    t = pt.getCharacterFromCursor(5, -1);
    assert(t == 'o');

    std::cout << "PASSED" << std::endl;
}
