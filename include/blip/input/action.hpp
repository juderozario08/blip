#pragma once
#include <string>

namespace input {
enum class ActionType {
    None,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    MoveWordForward,
    MoveWordBack,
    MoveStartOfLine,
    MoveEndOfLine,
    MoveStartOfFile,
    MoveEndOfFile,
    InsertText,
    DeleteChar,
    Backspace,
    NewLineNext,
    NewLinePrev,
    Undo,
    Redo,
    CommitUndo,
    InsertBlankLineAbove,
    InsertBlankLineBelow,
    ClampNormal
};

struct Action {
    ActionType type = ActionType::None;
    std::string payload = "";
    int count = 1;
};
}
