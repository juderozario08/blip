#pragma once
#include <SDL_stdinc.h>
#include <blip/buffer/table.hpp>
#include <string>
#include <utility>
#include <vector>

namespace buffer {

typedef struct {
    PieceTable::State table_state;
    size_t cursor_position;
    std::vector<size_t> line_starts;
} EditRecord;

class EditorBuffer {
  public:
    explicit EditorBuffer(const std::string &initial_text = "");

    void insertText(const std::string &text);
    void backspace(size_t amount = 1);

    void commit();
    void undo();
    void redo();

    std::string getText() const;
    size_t getCursor() const;
    size_t getTotalLength() const;
    void setCursor(Sint64 new_pos);
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    std::pair<size_t, size_t> getCursorPosition2D() const;
    size_t getCursorPositionFrom2D(size_t row, size_t col) const;
    void setCursorToBeginningColumn();
    void setCursorToEndingColumn();

  private:
    PieceTable table;
    size_t cursor_pos;
    std::vector<size_t> line_starts;
    size_t desired_col = 0;
    void recomputeAllLines();
    void updateLineStarts(const std::string &text);

    std::vector<EditRecord> undo_stack;
    std::vector<EditRecord> redo_stack;
};
}
