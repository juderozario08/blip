#include <blip/buffer/table.hpp>
#include <fstream>

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
    if (length == 0 || index >= total_length)
        return;
    if (index + length > total_length) {
        length = total_length - index;
    }

    while (length > 0) {
        size_t curr_length = 0;
        int p_idx = -1;

        for (size_t i = 0; i < pieces.size(); i++) {
            if (curr_length + pieces[i].length > index) {
                p_idx = i;
                break;
            }
            curr_length += pieces[i].length;
        }

        if (p_idx == -1)
            break;

        size_t offset_in_piece = index - curr_length;
        size_t chars_in_piece = pieces[p_idx].length - offset_in_piece;
        size_t delete_amount = std::min(length, chars_in_piece);

        if (delete_amount == pieces[p_idx].length) {
            pieces.erase(pieces.begin() + p_idx);
        } else if (offset_in_piece == 0) {
            pieces[p_idx].start += delete_amount;
            pieces[p_idx].length -= delete_amount;
        } else if (offset_in_piece + delete_amount == pieces[p_idx].length) {
            pieces[p_idx].length -= delete_amount;
        } else {
            Piece right_part = {pieces[p_idx].source, pieces[p_idx].start + offset_in_piece + delete_amount,
                                pieces[p_idx].length - offset_in_piece - delete_amount};
            pieces[p_idx].length = offset_in_piece;
            pieces.insert(pieces.begin() + p_idx + 1, right_part);
        }

        total_length -= delete_amount;
        length -= delete_amount;
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
        }
        curr_length += p.length;
    }
    return std::nullopt;
}

void PieceTable::writeToFile(const std::string &filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        return;
    }

    for (const auto &piece : pieces) {
        if (piece.length == 0) {
            continue;
        }
        const std::string &source_buf = (piece.source == BufType::ORIGINAL) ? original_buffer : add_buffer;
        out.write(source_buf.data() + piece.start, piece.length);
    }
    out.close();
}
}
