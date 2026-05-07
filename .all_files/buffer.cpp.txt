#include <algorithm>
#include <blip/buffer/buffer.hpp>

namespace buffer {

EditorBuffer::EditorBuffer(const std::string &initial_text) : table(initial_text), cursor_pos(0) {
    updateLineStarts(initial_text);
    commit();
}

void EditorBuffer::updateLineStarts(const std::string &text) {
    if (line_starts.empty()) {
        line_starts.push_back(0);
        for (size_t i = 0; i < text.length(); i++) {
            if (text[i] == '\n') {
                line_starts.push_back(i + 1);
            }
        }
    } else {
        size_t pushed = 0;
        auto it = std::upper_bound(line_starts.begin(), line_starts.end(), cursor_pos);
        size_t push_idx = std::distance(line_starts.begin(), it);

        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '\n') {
                line_starts.insert(line_starts.begin() + push_idx + pushed, cursor_pos + i + 1);
                pushed++;
            }
        }
        for (size_t j = push_idx + pushed; j < line_starts.size(); j++) {
            line_starts[j] += text.length();
        }
    }
}

void EditorBuffer::recomputeAllLines() {
    line_starts.clear();
    updateLineStarts(table.getText());
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
    updateLineStarts(text);
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
    table.erase(cursor_pos, amount);
    setCursor(cursor_pos - amount);
    recomputeAllLines();
    auto [_, col] = getCursorPosition2D();
    desired_col = col;
}

void EditorBuffer::moveLeft() {
    int current_pos = cursor_pos;
    setCursor(cursor_pos - 1);
    if (cursor_pos < current_pos) {
        desired_col--;
    }
}
void EditorBuffer::moveRight() {
    int current_pos = cursor_pos;
    setCursor(cursor_pos + 1);
    if (cursor_pos > current_pos) {
        desired_col++;
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
}
