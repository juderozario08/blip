#pragma once

#include <SDL.h>
#include <blip/input/action.hpp>
#include <string>
#include <vector>

namespace input {

enum class VimMode { NORMAL, INSERT, VISUAL, REPLACE, COMMAND };

class VimEngine {
  public:
    VimEngine();

    std::vector<Action> handleKeyDown(const SDL_Event &event);
    std::vector<Action> handleTextInput(const std::string &text);

    VimMode getMode() const;

  private:
    VimMode mode;
    std::string command_buffer;

    std::vector<Action> handleNormalMode(const SDL_Event &event);
    std::vector<Action> handleInsertMode(const SDL_Event &event);
    std::vector<Action> handleVisualMode(const SDL_Event &event);
    std::vector<Action> handleCommandMode(const SDL_Event &event);
};
}
