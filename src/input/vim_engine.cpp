#include <blip/input/vim_engine.hpp>

namespace input {

VimEngine::VimEngine() : mode(VimMode::NORMAL) {}

VimMode VimEngine::getMode() const { return mode; }

bool VimEngine::handleTextInput(const std::string &text, buffer::EditorBuffer &buffer) {
    if (mode == VimMode::INSERT) {
        buffer.insertText(text);
        return true;
    }

    if (mode == VimMode::NORMAL) {
        if (text == "i") {
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
    default:
        return false;
    }
}

bool VimEngine::handleNormalMode(const SDL_Event &event, buffer::EditorBuffer &buffer) {
    bool dirty = false;

    switch (event.key.keysym.sym) {
    case SDLK_h:
    case SDLK_LEFT:
        buffer.moveLeft();
        dirty = true;
        break;
    case SDLK_l:
    case SDLK_RIGHT:
        buffer.moveRight();
        dirty = true;
        break;
    case SDLK_k:
    case SDLK_UP:
        buffer.moveUp();
        dirty = true;
        break;
    case SDLK_j:
    case SDLK_DOWN:
        buffer.moveDown();
        dirty = true;
        break;
    case SDLK_u:
        buffer.undo();
        dirty = true;
        break;
    case SDLK_r:
        if (event.key.keysym.mod & KMOD_CTRL) {
            buffer.redo();
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

    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
        mode = VimMode::NORMAL;
        buffer.clampVimNormal();
        dirty = true;
        break;
    case SDLK_SPACE:
        buffer.commit();
        buffer.insertText(" ");
        dirty = true;
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

}
