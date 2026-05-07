#include <blip/buffer/table.hpp>

namespace buffer {

PieceTable::PieceTable(const std::string &initial_text) : original_buffer(initial_text) {
    if (!initial_text.empty()) {
        pieces.push_back({BufType::ORIGINAL, 0, initial_text.length()});
        total_length = initial_text.length();
    }
}

void PieceTable::insert(size_t index, const std::string &text) {
    if (text.empty())
        return;

    if (total_length < index) {
        index = total_length;
    }

    size_t add_start = add_buffer.length();
    add_buffer += text;

    if (pieces.empty()) {
        pieces.push_back(Piece{BufType::ADD, add_start, text.length()});
        total_length += text.length();
        return;
    }

    size_t curr_length = 0;
    for (size_t i = 0; i < pieces.size(); i++) {
        if (curr_length + pieces[i].length >= index) {
            size_t piece_index = index - curr_length;
            if (piece_index == 0) {
                Piece new_piece = {BufType::ADD, add_start, text.length()};
                pieces.insert(pieces.begin() + i, new_piece);
            } else if (pieces[i].source == BufType::ADD && pieces[i].start + pieces[i].length == add_start &&
                       piece_index == pieces[i].length) {
                pieces[i].length += text.length();
            } else {
                Piece right_half = {pieces[i].source, pieces[i].start + piece_index, pieces[i].length - piece_index};
                Piece new_piece = {BufType::ADD, add_start, text.length()};
                pieces[i] = {pieces[i].source, pieces[i].start, piece_index};

                pieces.insert(pieces.begin() + i + 1, new_piece);
                pieces.insert(pieces.begin() + i + 2, right_half);
            }
            total_length += text.length();
            break;
        }
        curr_length += pieces[i].length;
    }
}

void PieceTable::erase(size_t index, size_t length) {
    if (length < 1)
        return;

    if (length > index) {
        length = index;
    }

    if (index > total_length)
        index = total_length;

    size_t curr_length = 0;
    int i = 0;

    for (; i < pieces.size(); i++) {
        if (curr_length + pieces[i].length >= index)
            break;
        curr_length += pieces[i].length;
    }

    while (i >= 0 && length > 0) {
        size_t chars_available = index - curr_length;
        size_t delete_amount = std::min(length, chars_available);
        if (delete_amount == pieces[i].length) {
            pieces.erase(pieces.begin() + i);
        } else if (chars_available == pieces[i].length) {
            pieces[i].length -= delete_amount;
        } else if (chars_available == delete_amount) {
            pieces[i].start += delete_amount;
            pieces[i].length -= delete_amount;
        } else {
            Piece right_part = {pieces[i].source, pieces[i].start + chars_available, pieces[i].length - chars_available};
            pieces[i].length = chars_available - delete_amount;
            pieces.insert(pieces.begin() + i + 1, right_part);
        }
        total_length -= delete_amount;
        length -= delete_amount;
        index -= delete_amount;
        if (length == 0)
            break;
        if (--i >= 0) {
            curr_length -= pieces[i].length;
        }
    }
}

size_t PieceTable::getTotalLength() const { return total_length; }

std::string PieceTable::getText() const {
    std::string final_text;
    final_text.reserve(total_length);
    for (const auto &p : pieces) {
        if (p.source == BufType::ORIGINAL) {
            final_text += original_buffer.substr(p.start, p.length);
        } else {
            final_text += add_buffer.substr(p.start, p.length);
        }
    }
    return final_text;
}

size_t PieceTable::getPieceCount() const { return pieces.size(); }

PieceTable::State PieceTable::getState() const { return State{pieces, total_length}; }

void PieceTable::restoreState(const State &state) {
    this->pieces = state.pieces;
    this->total_length = state.total_length;
}

std::optional<char> PieceTable::getCharacterFromCursor(size_t index, int offset) const {
    if (total_length == 0 || pieces.empty())
        return std::nullopt;

    size_t target_index;
    if (offset < 0) {
        size_t left = static_cast<size_t>(-offset);
        if (left > index) [[unlikely]] {
            target_index = 0;
        } else {
            target_index = index - left;
        }
    } else {
        target_index = index + offset;
    }
    if (target_index >= total_length) {
        target_index = total_length - 1;
    }

    size_t curr_length = 0;
    for (const auto &p : pieces) {
        if (curr_length + p.length > target_index) {
            size_t piece_offset = target_index - curr_length;
            if (p.source == BufType::ORIGINAL) {
                return original_buffer[p.start + piece_offset];
            } else {
                return add_buffer[p.start + piece_offset];
            }
            break;
        }
        curr_length += p.length;
    }
    return std::nullopt;
}
}
