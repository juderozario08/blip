#include <algorithm>
#include <blip/buffer/buffer.hpp>

namespace buffer {

EditorBuffer::EditorBuffer(const std::string &initial_text) : table(initial_text), cursor_pos(0) {
    adjustLineStartsOnInsert(0, initial_text);
    commit();
}

void EditorBuffer::adjustLineStartsOnInsert(size_t index, const std::string &text) {
    if (line_starts.empty()) {
        line_starts.push_back(0);
        for (size_t i = 0; i < text.length(); i++) {
            if (text[i] == '\n')
                line_starts.push_back(i + 1);
        }
        return;
    }

    auto it = std::upper_bound(line_starts.begin(), line_starts.end(), index);
    size_t insert_idx = std::distance(line_starts.begin(), it);

    for (size_t i = insert_idx; i < line_starts.size(); i++) {
        line_starts[i] += text.length();
    }

    size_t pushed = 0;
    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == '\n') {
            line_starts.insert(line_starts.begin() + insert_idx + pushed, index + i + 1);
            pushed++;
        }
    }
}

void EditorBuffer::adjustLineStartsOnErase(size_t index, size_t amount) {
    if (line_starts.empty()) {
        return;
    }

    auto it_start = std::upper_bound(line_starts.begin(), line_starts.end(), index);
    auto it_end = std::upper_bound(line_starts.begin(), line_starts.end(), index + amount);

    it_start = line_starts.erase(it_start, it_end);

    for (auto it = it_start; it != line_starts.end(); ++it) {
        *it -= amount;
    }
}

void EditorBuffer::commit() {
    if (!redo_stack.empty()) {
        redo_stack.clear();
    }
    undo_stack.push_back(EditRecord{table.getState(), cursor_pos, line_starts});
}

void EditorBuffer::undo() {
    if (undo_stack.empty())
        return;
    redo_stack.push_back(EditRecord{table.getState(), cursor_pos, line_starts});
    EditRecord record = std::move(undo_stack.back());
    table.restoreState(record.table_state);
    setCursor(record.cursor_position);
    line_starts = record.line_starts;
    undo_stack.pop_back();
}

void EditorBuffer::redo() {
    if (redo_stack.empty())
        return;
    undo_stack.push_back(EditRecord{table.getState(), cursor_pos, line_starts});
    EditRecord record = std::move(redo_stack.back());
    table.restoreState(record.table_state);
    setCursor(record.cursor_position);
    line_starts = record.line_starts;
    redo_stack.pop_back();
}

std::string EditorBuffer::getText() const { return table.getText(); }

size_t EditorBuffer::getCursor() const { return cursor_pos; }

size_t EditorBuffer::getTotalLength() const { return table.getTotalLength(); }

void EditorBuffer::setCursorToBeginningColumn() {
    auto [row, _] = getCursorPosition2D();
    setCursor(line_starts[row]);
}

void EditorBuffer::setCursorToEndingColumn() {
    auto [row, _] = getCursorPosition2D();
    if (row == line_starts.size() - 1) {
        setCursor(table.getTotalLength());
    } else {
        setCursor(line_starts[row + 1] - 1);
    }
}

void EditorBuffer::setCursor(Sint64 new_pos) {
    if (new_pos < 0) {
        cursor_pos = 0;
    } else if (static_cast<size_t>(new_pos) > table.getTotalLength()) {
        cursor_pos = table.getTotalLength();
    } else {
        cursor_pos = new_pos;
    }
}

void EditorBuffer::insertText(const std::string &text) {
    if (text.empty()) {
        return;
    }
    adjustLineStartsOnInsert(cursor_pos, text);
    table.insert(cursor_pos, text);
    setCursor(cursor_pos + text.length());
    auto [_, col] = getCursorPosition2D();
    desired_col = col;
}

void EditorBuffer::backspace(size_t amount) {
    if (amount == 0 || cursor_pos == 0) {
        return;
    }
    if (cursor_pos < amount) {
        amount = cursor_pos;
    }

    table.erase(cursor_pos - amount, amount);
    adjustLineStartsOnErase(cursor_pos - amount, amount);

    setCursor(cursor_pos - amount);
    auto [_, col] = getCursorPosition2D();
    desired_col = col;
}

void EditorBuffer::moveLeft() {
    if (cursor_pos == 0) {
        return;
    }
    auto [row, col] = getCursorPosition2D();
    if (col > 0) {
        cursor_pos--;
        desired_col = col - 1;
    }
}

void EditorBuffer::moveRight() {
    if (cursor_pos >= table.getTotalLength()) {
        return;
    }
    auto curr_ch = table.getCharacterFromCursor(cursor_pos);
    if (curr_ch && *curr_ch != '\n') {
        cursor_pos++;
        auto [row, col] = getCursorPosition2D();
        desired_col = col;
    }
}
void EditorBuffer::moveUp() {
    auto [row, _] = getCursorPosition2D();
    if (row == 0) {
        return;
    }
    setCursor(getCursorPositionFrom2D(row - 1, desired_col));
}
void EditorBuffer::moveDown() {
    auto [row, _] = getCursorPosition2D();
    if (row == line_starts.size() - 1) {
        return;
    }
    setCursor(getCursorPositionFrom2D(row + 1, desired_col));
}

size_t EditorBuffer::getCursorPositionFrom2D(size_t row, size_t col) const {
    if (line_starts.empty()) {
        return 0;
    }
    if (row >= line_starts.size()) {
        row = line_starts.size() - 1;
    }
    size_t line_start_index = line_starts[row];
    size_t next_line_start;
    if (row + 1 < line_starts.size()) {
        next_line_start = line_starts[row + 1];
    } else {
        next_line_start = table.getTotalLength() + 1;
    }
    size_t target_index = line_start_index + col;
    if (target_index >= next_line_start) {
        target_index = next_line_start - 1;
    }
    if (target_index > table.getTotalLength()) {
        target_index = table.getTotalLength();
    }
    return target_index;
}

std::pair<size_t, size_t> EditorBuffer::getCursorPosition2D() const {
    if (line_starts.empty()) {
        return {0, 0};
    }
    auto it = std::upper_bound(line_starts.begin(), line_starts.end(), cursor_pos);
    size_t row = std::distance(line_starts.begin(), it) - 1;
    size_t col = cursor_pos - line_starts[row];
    return {row, col};
}

void EditorBuffer::clampVimNormal() {
    std::string text = getText();
    if (text.empty())
        return;

    bool has_trailing_newline = (text.back() == '\n');
    size_t max_cursor = has_trailing_newline ? text.length() : text.length() - 1;

    if (cursor_pos > max_cursor)
        cursor_pos = max_cursor;
    if (cursor_pos == text.length())
        return;

    if (text[cursor_pos] == '\n') {
        bool is_blank_line = (cursor_pos == 0) || (text[cursor_pos - 1] == '\n');
        if (!is_blank_line) {
            cursor_pos--;
        }
    }
}

void EditorBuffer::insertNewLineNext() {
    size_t len = table.getTotalLength();
    while (cursor_pos < len) {
        auto ch = table.getCharacterFromCursor(cursor_pos);
        if (ch && *ch == '\n')
            break;
        cursor_pos++;
    }
    if (cursor_pos < len)
        cursor_pos++;
    setCursor(cursor_pos);
    size_t new_line_pos = cursor_pos;
    insertText("\n");
    if (new_line_pos < len)
        setCursor(new_line_pos);
}

void EditorBuffer::insertNewLinePrev() {
    while (cursor_pos > 0) {
        auto ch = table.getCharacterFromCursor(cursor_pos - 1);
        if (ch && *ch == '\n')
            break;
        cursor_pos--;
    }
    setCursor(cursor_pos);
    size_t new_line_pos = cursor_pos;
    insertText("\n");
    setCursor(new_line_pos);
}

void EditorBuffer::cursorForward(const std::string &delimiter) {
    size_t len = table.getTotalLength();
    if (len == 0 || cursor_pos >= len - 1)
        return;

    auto getChar = [&](size_t pos) -> char {
        auto ch = table.getCharacterFromCursor(pos);
        return ch ? *ch : '\0';
    };
    auto getClass = [&](char c) -> int {
        if (c == '\0')
            return -1;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            return 0;
        if (delimiter.find(c) != std::string::npos)
            return 1;
        return 2;
    };

    int start_class = getClass(getChar(cursor_pos));

    while (cursor_pos < len) {
        int current_class = getClass(getChar(cursor_pos));
        if (current_class == 0 || current_class != start_class)
            break;
        cursor_pos++;
    }
    while (cursor_pos < len) {
        if (getClass(getChar(cursor_pos)) != 0)
            break;
        cursor_pos++;
    }
    if (cursor_pos >= len)
        cursor_pos = len - 1;
    setCursor(cursor_pos);
}

void EditorBuffer::cursorBack(const std::string &delimiter) {
    if (cursor_pos == 0)
        return;

    auto getChar = [&](size_t pos) -> char {
        auto ch = table.getCharacterFromCursor(pos);
        return ch ? *ch : '\0';
    };
    auto getClass = [&](char c) -> int {
        if (c == '\0')
            return -1;
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
            return 0;
        if (delimiter.find(c) != std::string::npos)
            return 1;
        return 2;
    };

    cursor_pos--;
    while (cursor_pos > 0 && getClass(getChar(cursor_pos)) == 0)
        cursor_pos--;

    int target_class = getClass(getChar(cursor_pos));
    while (cursor_pos > 0) {
        char left_char = getChar(cursor_pos - 1);
        if (getClass(left_char) != target_class || getClass(left_char) == 0)
            break;
        cursor_pos--;
    }
    setCursor(cursor_pos);
}

void EditorBuffer::moveToStartOfLine() {
    while (cursor_pos > 0) {
        auto ch = table.getCharacterFromCursor(cursor_pos - 1);
        if (ch && *ch == '\n')
            break;
        cursor_pos--;
    }
    setCursor(cursor_pos);
}

void EditorBuffer::moveToEndOfLine() {
    size_t len = table.getTotalLength();
    while (cursor_pos < len) {
        auto ch = table.getCharacterFromCursor(cursor_pos);
        if (ch && *ch == '\n')
            break;
        cursor_pos++;
    }
    setCursor(cursor_pos);
}

void EditorBuffer::moveToStartOfFile() { setCursor(0); }
void EditorBuffer::moveToEndOfFile() { setCursor(table.getTotalLength()); }

void EditorBuffer::deleteChar() {
    size_t len = table.getTotalLength();
    if (cursor_pos >= len)
        return;
    auto ch = table.getCharacterFromCursor(cursor_pos);
    if (ch && *ch == '\n')
        return;
    setCursor(cursor_pos + 1);
    backspace(1);
}

void EditorBuffer::insertBlankLineAboveStay() {
    size_t original_cursor = cursor_pos;
    moveToStartOfLine();
    insertText("\n");
    setCursor(original_cursor + 1);
}

void EditorBuffer::insertBlankLineBelowStay() {
    size_t original_cursor = cursor_pos;
    moveToEndOfLine();
    if (cursor_pos < table.getTotalLength())
        cursor_pos++;
    insertText("\n");
    setCursor(original_cursor);
}

size_t EditorBuffer::getNumberOfLines() { return line_starts.size(); }
}
