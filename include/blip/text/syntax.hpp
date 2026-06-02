#pragma once
#include <SDL.h>
#include <blip/config/editor.hpp>
#include <string>
#include <tree_sitter/api.h>

namespace text {

class SyntaxEngine {
  public:
    SyntaxEngine();
    ~SyntaxEngine();

    SyntaxEngine(const SyntaxEngine &) = delete;
    SyntaxEngine &operator=(const SyntaxEngine &) = delete;

    void parse(const std::string &sourceText);
    SDL_Color getColorForByte(uint32_t byteIndex, const config::Theme &theme);

  private:
    TSParser *parser = nullptr;
    TSTree *tree = nullptr;
};

}
