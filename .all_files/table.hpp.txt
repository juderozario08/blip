#pragma once
#include <string>
#include <vector>
#include <optional>

namespace buffer {

enum class BufType { ORIGINAL, ADD };

typedef struct {
    BufType source;
    size_t start;
    size_t length;
} Piece;

class PieceTable {
  public:
    typedef struct {
        std::vector<Piece> pieces;
        size_t total_length;
    } State;

    explicit PieceTable(const std::string &initial_text = "");

    void insert(size_t index, const std::string &text);
    void erase(size_t index, size_t length);

    std::string getText() const;
    size_t getTotalLength() const;
    size_t getPieceCount() const;
    std::optional<char> getCharacterFromCursor(size_t index, int offset = 0) const;

    State getState() const;
    void restoreState(const State &state);

  private:
    std::string original_buffer;
    std::string add_buffer;
    std::vector<Piece> pieces;
    size_t total_length = 0;
};
}
