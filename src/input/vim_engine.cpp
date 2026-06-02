#include <blip/input/vim_engine.hpp>

namespace input {

VimEngine::VimEngine() : mode(VimMode::NORMAL) {}
VimMode VimEngine::getMode() const { return mode; }

std::vector<Action> VimEngine::handleTextInput(const std::string &text) {
    std::vector<Action> actions;

    if (mode == VimMode::INSERT) {
        actions.push_back({ActionType::InsertText, text});
        return actions;
    }

    if (mode == VimMode::COMMAND) {
        active_command = text;
        return actions;
    }

    if (mode == VimMode::NORMAL) {
        if (text == ":") {
            mode = VimMode::COMMAND;
            active_command.clear();
        } else if (text == "i") {
            mode = VimMode::INSERT;
        } else if (text == "I") {
            mode = VimMode::INSERT;
            actions.push_back({ActionType::MoveStartOfLine});
        } else if (text == "a") {
            mode = VimMode::INSERT;
            actions.push_back({ActionType::MoveRight});
        } else if (text == "A") {
            mode = VimMode::INSERT;
            actions.push_back({ActionType::MoveEndOfLine});
        } else if (text == "o") {
            mode = VimMode::INSERT;
            actions.push_back({ActionType::NewLineNext});
        } else if (text == "O") {
            mode = VimMode::INSERT;
            actions.push_back({ActionType::NewLinePrev});
        } else if (text == "v") {
            mode = VimMode::VISUAL;
        }
    }
    return actions;
}

std::vector<Action> VimEngine::handleKeyDown(const SDL_Event &event) {
    switch (mode) {
    case VimMode::NORMAL:
        return handleNormalMode(event);
    case VimMode::INSERT:
        return handleInsertMode(event);
    case VimMode::VISUAL:
        return handleVisualMode(event);
    case VimMode::COMMAND:
        return handleCommandMode(event);
    default:
        return {};
    }
}

std::vector<Action> VimEngine::handleNormalMode(const SDL_Event &event) {
    std::vector<Action> actions;
    bool isCtrl = (event.key.keysym.mod & KMOD_CTRL);
    bool isShift = (event.key.keysym.mod & KMOD_SHIFT);

    // CONTROL BINDINGS
    if (isCtrl) {
        if (event.key.keysym.sym == SDLK_d) {
            actions.push_back({ActionType::MoveDown, "", 15});
        } else if (event.key.keysym.sym == SDLK_u) {
            actions.push_back({ActionType::MoveUp, "", 15});
        } else if (event.key.keysym.sym == SDLK_r) {
            actions.push_back({ActionType::Redo});
        }
        if (!actions.empty())
            actions.push_back({ActionType::ClampNormal});
        return actions;
    }

    // DOUBLE-STROKE COMMANDS
    if (event.key.keysym.sym == SDLK_g && !isShift) {
        if (command_buffer == "g") {
            actions.push_back({ActionType::MoveStartOfFile});
            command_buffer = "";
        } else {
            command_buffer = "g";
            return actions;
        }
    } else {
        command_buffer = "";
    }

    // STANDARD BINDINGS
    std::string delimiters = isShift ? " " : " .@+-/:(){}[]&,;";

    switch (event.key.keysym.sym) {
    case SDLK_h:
    case SDLK_LEFT:
        actions.push_back(isShift ? Action{ActionType::InsertBlankLineAbove} : Action{ActionType::MoveLeft});
        break;
    case SDLK_l:
    case SDLK_RIGHT:
        actions.push_back(isShift ? Action{ActionType::InsertBlankLineBelow} : Action{ActionType::MoveRight});
        break;
    case SDLK_j:
    case SDLK_DOWN:
        actions.push_back({ActionType::MoveDown});
        break;
    case SDLK_k:
    case SDLK_UP:
        actions.push_back({ActionType::MoveUp});
        break;
    case SDLK_b:
        actions.push_back({ActionType::MoveWordBack, delimiters});
        break;
    case SDLK_w:
        actions.push_back({ActionType::MoveWordForward, delimiters});
        break;
    case SDLK_u:
        actions.push_back({ActionType::Undo});
        break;
    case SDLK_x:
        actions.push_back({ActionType::DeleteChar});
        break;
    case SDLK_0:
        actions.push_back({ActionType::MoveStartOfLine});
        break;
    case SDLK_4:
        if (isShift)
            actions.push_back({ActionType::MoveEndOfLine});
        break;
    case SDLK_g:
        if (isShift)
            actions.push_back({ActionType::MoveEndOfFile});
        break;
    }

    if (!actions.empty())
        actions.push_back({ActionType::ClampNormal});
    return actions;
}

std::vector<Action> VimEngine::handleInsertMode(const SDL_Event &event) {
    std::vector<Action> actions;
    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
        mode = VimMode::NORMAL;
        actions.push_back({ActionType::ClampNormal});
        break;
    case SDLK_SPACE:
        actions.push_back({ActionType::CommitUndo});
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        actions.push_back({ActionType::CommitUndo});
        actions.push_back({ActionType::InsertText, "\n"});
        break;
    case SDLK_BACKSPACE:
        actions.push_back({ActionType::CommitUndo});
        actions.push_back({ActionType::Backspace});
        break;
    case SDLK_LEFT:
        actions.push_back(Action{ActionType::MoveLeft});
        break;
    case SDLK_RIGHT:
        actions.push_back(Action{ActionType::MoveRight});
        break;
    case SDLK_DOWN:
        actions.push_back({ActionType::MoveDown});
        break;
    case SDLK_UP:
        actions.push_back({ActionType::MoveUp});
        break;
    }
    return actions;
}

std::vector<Action> VimEngine::handleVisualMode(const SDL_Event &event) {
    std::vector<Action> actions;
    if (event.key.keysym.sym == SDLK_ESCAPE) {
        mode = VimMode::NORMAL;
        actions.push_back({ActionType::ClampNormal});
    }
    return actions;
}

std::vector<Action> VimEngine::handleCommandMode(const SDL_Event &event) {
    std::vector<Action> actions;
    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
        mode = VimMode::NORMAL;
        active_command.clear();
        actions.push_back({ActionType::ClampNormal});
        break;
    case SDLK_BACKSPACE:
        if (!active_command.empty()) {
            active_command.pop_back();
        } else {
            mode = VimMode::NORMAL;
            actions.push_back({ActionType::ClampNormal});
        }
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        if (active_command == "w") {
            actions.push_back({ActionType::SaveFile, ""});
        } else if (active_command == "q") {
            actions.push_back({ActionType::Quit, ""});
        } else if (active_command == "wq") {
            actions.push_back({ActionType::SaveFile, ""});
            actions.push_back({ActionType::Quit, ""});
        } else if (active_command.rfind("w ", 0) == 0) {
            actions.push_back({ActionType::SaveFile, active_command.substr(2)});
        }

        mode = VimMode::NORMAL;
        active_command.clear();
        break;
    }
    return actions;
}

const std::string &VimEngine::getActiveCommand() const { return active_command; }
}
