#include <blip/input/vim_engine.hpp>
#include <iostream>

namespace input {

VimEngine::VimEngine() : mode(VimMode::NORMAL) {}

VimMode VimEngine::getMode() const { return mode; }

bool VimEngine::handleTextInput(const std::string &text, buffer::EditorBuffer &buffer) {
    if (mode == VimMode::INSERT) {
        buffer.insertText(text);
        return true;
    }

    if (mode == VimMode::NORMAL) {
        if (text == ":") {
            mode = VimMode::COMMAND;
            return true;
        } else if (text == "i") {
            mode = VimMode::INSERT;
            return true;
        } else if (text == "I") {
            mode = VimMode::INSERT;
            buffer.setCursorToBeginningColumn();
            return true;
        } else if (text == "a") {
            mode = VimMode::INSERT;
            buffer.setCursor(buffer.getCursor() + 1);
            return true;
        } else if (text == "A") {
            mode = VimMode::INSERT;
            buffer.setCursorToEndingColumn();
            return true;
        } else if (text == "o") {
            mode = VimMode::INSERT;
            buffer.insertNewLineNext();
            return true;
        } else if (text == "O") {
            mode = VimMode::INSERT;
            buffer.insertNewLinePrev();
            return true;
        } else if (text == "v") {
            mode = VimMode::VISUAL;
            return true;
        }
    }
    return false;
}

bool VimEngine::handleKeyDown(const SDL_Event &event, buffer::EditorBuffer &buffer) {
    switch (mode) {
    case VimMode::NORMAL:
        return handleNormalMode(event, buffer);
    case VimMode::INSERT:
        return handleInsertMode(event, buffer);
    case VimMode::VISUAL:
        return handleVisualMode(event, buffer);
    case VimMode::COMMAND:
        return handleCommandMode(event, buffer);
    default:
        return false;
    }
}

bool VimEngine::handleNormalMode(const SDL_Event &event, buffer::EditorBuffer &buffer) {
    bool dirty = false;
    bool isCtrl = (event.key.keysym.mod & KMOD_CTRL);
    bool isShift = (event.key.keysym.mod & KMOD_SHIFT);

    if (isCtrl) {
        if (event.key.keysym.sym == SDLK_d) {
            for (int i = 0; i < 15; i++)
                buffer.moveDown();
            dirty = true;
        } else if (event.key.keysym.sym == SDLK_u) {
            for (int i = 0; i < 15; i++)
                buffer.moveUp();
            dirty = true;
        } else if (event.key.keysym.sym == SDLK_r) {
            buffer.redo();
            dirty = true;
        }
        if (dirty)
            buffer.clampVimNormal();
        return dirty;
    }

    if (event.key.keysym.sym == SDLK_g && !isShift) {
        if (command_buffer == "g") {
            buffer.moveToStartOfFile();
            command_buffer = "";
            dirty = true;
        } else {
            command_buffer = "g";
            return false;
        }
    } else {
        command_buffer = "";
    }

    std::string delimiters = isShift ? " " : " .@+-/:(){}[]&,;";

    switch (event.key.keysym.sym) {
    case SDLK_h:
    case SDLK_LEFT:
        if (isShift)
            buffer.insertBlankLineAboveStay();
        else
            buffer.moveLeft();
        dirty = true;
        break;

    case SDLK_l:
    case SDLK_RIGHT:
        if (isShift)
            buffer.insertBlankLineBelowStay();
        else
            buffer.moveRight();
        dirty = true;
        break;

    case SDLK_j:
    case SDLK_DOWN:
        buffer.moveDown();
        dirty = true;
        break;

    case SDLK_k:
    case SDLK_UP:
        buffer.moveUp();
        dirty = true;
        break;

    case SDLK_b:
        buffer.cursorBack(delimiters);
        dirty = true;
        break;
    case SDLK_w:
        buffer.cursorForward(delimiters);
        dirty = true;
        break;

    case SDLK_u:
        buffer.undo();
        dirty = true;
        break;
    case SDLK_x:
        buffer.deleteChar();
        dirty = true;
        break;

    case SDLK_0:
        buffer.moveToStartOfLine();
        dirty = true;
        break;
    case SDLK_4:
        if (isShift) {
            buffer.moveToEndOfLine();
            dirty = true;
        }
        break;
    case SDLK_g:
        if (isShift) {
            buffer.moveToEndOfFile();
            dirty = true;
        }
        break;
    }

    if (dirty)
        buffer.clampVimNormal();
    return dirty;
}

bool VimEngine::handleInsertMode(const SDL_Event &event, buffer::EditorBuffer &buffer) {
    bool dirty = false;
    if (event.key.keysym.sym == SDLK_w) {
        std::cout << "Window prefix <C-w> triggered!\n";
    }
    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
        mode = VimMode::NORMAL;
        buffer.clampVimNormal();
        dirty = true;
        break;
    case SDLK_SPACE:
        buffer.commit();
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        buffer.commit();
        buffer.insertText("\n");
        dirty = true;
        break;
    case SDLK_BACKSPACE:
        if (buffer.getCursor() > 0) {
            buffer.commit();
            buffer.backspace(1);
            dirty = true;
        }
        break;
    }
    return dirty;
}

bool VimEngine::handleVisualMode(const SDL_Event &event, buffer::EditorBuffer &buffer) {
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        mode = VimMode::NORMAL;
        buffer.clampVimNormal();
        return true;
    }
    return false;
}

bool VimEngine::handleCommandMode(const SDL_Event &event, buffer::EditorBuffer &buffer) {
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        mode = VimMode::NORMAL;
        buffer.clampVimNormal();
        return true;
    }
    return false;
}

}
