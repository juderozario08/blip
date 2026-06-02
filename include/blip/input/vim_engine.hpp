#pragma once

#include <SDL.h>
#include <blip/buffer/buffer.hpp>
#include <string>

namespace input {

enum class VimMode { NORMAL, INSERT, VISUAL, REPLACE, COMMAND };

class VimEngine {
  public:
    VimEngine();

    bool handleKeyDown(const SDL_Event &event, buffer::EditorBuffer &buffer);
    bool handleTextInput(const std::string &text, buffer::EditorBuffer &buffer);

    VimMode getMode() const;

  private:
    VimMode mode;
    std::string command_buffer;

    bool handleNormalMode(const SDL_Event &event, buffer::EditorBuffer &buffer);
    bool handleInsertMode(const SDL_Event &event, buffer::EditorBuffer &buffer);
    bool handleVisualMode(const SDL_Event &event, buffer::EditorBuffer &buffer);
    bool handleCommandMode(const SDL_Event &event, buffer::EditorBuffer &buffer);
};

}
