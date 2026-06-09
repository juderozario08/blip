#pragma once
#include <SDL.h>
#include <blip/buffer/table.hpp>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace buffer {

struct EditRecord {
    PieceTable::State table_state;
    size_t cursor_position;
    std::vector<size_t> line_starts;
    size_t desired_col;
};

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
    void clampVimNormal();
    void insertNewLineNext();
    void insertNewLinePrev();
    size_t getNumberOfLines();

    void cursorForward(const std::string &delimiter);
    void cursorBack(const std::string &delimiter);

    void insertBlankLineAboveStay();
    void insertBlankLineBelowStay();

    void deleteChar();
    void moveToStartOfLine();
    void moveToEndOfLine();
    void moveToStartOfFile();
    void moveToEndOfFile();
    void saveToFile(const std::string &filename);

    void setVisualAnchor();
    void clearVisualAnchor();
    bool hasSelection() const;

    std::pair<size_t, size_t> getSelectionRange() const;
    std::string getSelectedText() const;

    std::string getLineText(size_t lineIndex) const;
    size_t getLineStartByte(size_t lineIndex) const;

  private:
    PieceTable table;
    size_t cursor_pos;
    std::vector<size_t> line_starts;
    size_t desired_col = 0;
    std::optional<size_t> visual_anchor = std::nullopt;

    void adjustLineStartsOnInsert(size_t index, const std::string &text);
    void adjustLineStartsOnErase(size_t index, size_t amount);

    std::vector<EditRecord> undo_stack;
    std::vector<EditRecord> redo_stack;
};
}
