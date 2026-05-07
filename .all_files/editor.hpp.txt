#pragma once
#include <SDL.h>
#include <blip/app/main.hpp>
#include <string>

namespace config {
enum class CursorStyleOpts { CursorBlock, CursorLine, COUNT };
enum class LineNumberOpts { LineAbsolute, LineRelative, LineHidden, LineAbsoluteAndRelative, COUNT };
enum class AutoFormatOpts { FormatManual, FormatOnSave, FormatOnPaste, COUNT };
enum class AutoSaveModeOpts { SaveOnFocus, SaveDelay, SaveManual, COUNT };

typedef struct Shortcut {
    Uint16 modifiers = 0;
    SDL_Keycode key = SDLK_UNKNOWN;
} Shortcut;

typedef struct Color {
    Uint8 r, g, b, a;
} Color;

typedef struct Theme {
    Color background, foreground, cursor, selection, line_number, whitespace, diff_add, diff_remove, diff_change,
        diagnostic_error, diagnostic_warning, diagnostic_info, popup_background, tooltip_border, completion_background,
        hover_tab_background;
} Theme;

typedef struct Font {
    std::string family;
    std::string style;
    Color color;
    bool ligatures;
    Uint8 size;
    float line_height;
} Font;

typedef struct UI {
    CursorStyleOpts cursor_style;
    LineNumberOpts line_numbers;
    bool status_bar_visible, tab_bar_visible, highlight_current_line, show_whitespace, show_indent_guides;
    Uint8 ui_scale;
} UI;

typedef struct Preference {
    Uint16 tab_width;
    AutoFormatOpts auto_format;
    bool bracket_matching, auto_close_brackets, word_wrap, trim_trailing_whitespace_on_save, highlight_active_scope, auto_indent;
} Preference;

typedef struct Input {
    Shortcut shortcut_save, shortcut_search, shortcut_split_vertical, shortcut_split_horizontal;
    bool vim_mode, autocomplete, clipboard_integration, mouse_selection, drag_and_drop;
} Input;

typedef struct Plugins {
    bool lsp, snippets, git, linter, file_explorer, hot_reload;
} Plugins;

typedef struct File {
    AutoSaveModeOpts autosave_mode;
    bool show_hidden_files;
} File;

typedef struct EditorConfig {
    Theme theme;
    Font font;
    UI ui;
    Preference preference;
    Input input;
    Plugins plugins;
    File file;
} EditorConfig;

void loadConfig(std::string filepath, EditorConfig &state);
std::pair<std::string, std::string> parseLine(std::string line);
void setDefaultConifg(EditorConfig &state);
void setForegroundColor(app::AppState &appState, EditorConfig &state);
void setCursorColor(app::AppState &appState, EditorConfig &state);
void setSelectionColor(app::AppState &appState, EditorConfig &state);
void setLineNumberColor(app::AppState &appState, EditorConfig &state);
void setWhitespaceColor(app::AppState &appState, EditorConfig &state);
void setDiffAddColor(app::AppState &appState, EditorConfig &state);
void setDiffRemoveColor(app::AppState &appState, EditorConfig &state);
void setDiffChangeColor(app::AppState &appState, EditorConfig &state);
void setDiffAddColor(app::AppState &appState, EditorConfig &state);
void setDiffRemoveColor(app::AppState &appState, EditorConfig &state);
void setDiffChangeColor(app::AppState &appState, EditorConfig &state);
void setDiagnosticErrorColor(app::AppState &appState, EditorConfig &state);
void setDiagnosticWarningColor(app::AppState &appState, EditorConfig &state);
void setDiagnosticInfoColor(app::AppState &appState, EditorConfig &state);
void setPopupBackgroundColor(app::AppState &appState, EditorConfig &state);
void setTooltipBorderColor(app::AppState &appState, EditorConfig &state);
void setCompletionBackgroundColor(app::AppState &appState, EditorConfig &state);
void setHoverBackgroundColor(app::AppState &appState, EditorConfig &state);

namespace positions {
namespace x {
inline constexpr const int LINE_NUMBER = 30;
inline constexpr const int DIAGNOSTIC = 10;
inline constexpr const int GIT = 30;
}
namespace width {
inline constexpr const int LINE_NUMBERS = 15;
inline constexpr const int DIAGNOSTIC = 2;
inline constexpr const int GIT = 2;
}
}

namespace constants {
namespace theme {
inline constexpr const char *BACKGROUND = "background";
inline constexpr const char *FOREGROUND = "foreground";
inline constexpr const char *CURSOR = "cursor";
inline constexpr const char *SELECTION = "selection";
inline constexpr const char *LINE_NUMBER = "line_number";
inline constexpr const char *WHITESPACE = "whitespace";
inline constexpr const char *DIFF_ADD = "diff_add";
inline constexpr const char *DIFF_REMOVE = "diff_remove";
inline constexpr const char *DIFF_CHANGE = "diff_change";
inline constexpr const char *DIAGNOSTIC_INFO = "diagnostic_info";
inline constexpr const char *DIAGNOSTIC_ERROR = "diagnostic_error";
inline constexpr const char *DIAGNOSTIC_WARNING = "diagnostic_warning";
inline constexpr const char *POPUP_BACKGROUND = "popup_background";
inline constexpr const char *TOOLTIP_BORDER = "tooltip_border";
inline constexpr const char *COMPLETION_BACKGROUND = "completion_background";
inline constexpr const char *HOVER_TAB_BACKGROUND = "hover_tab_background";
}

namespace font {
inline constexpr const char *FAMILY = "font_family";
inline constexpr const char *STYLE = "font_style";
inline constexpr const char *COLOR = "font_color";
inline constexpr const char *LIGATURES = "font_ligatures";
inline constexpr const char *SIZE = "font_size";
inline constexpr const char *LINE_HEIGHT = "line_height";
}

namespace ui {
inline constexpr const char *CURSOR_STYLE = "cursor_style";
inline constexpr const char *LINE_NUMBERS = "line_numbers";
inline constexpr const char *STATUS_BAR_VISIBLE = "status_bar_visible";
inline constexpr const char *TAB_BAR_VISIBLE = "tab_bar_visible";
inline constexpr const char *HIGHLIGHT_CURRENT_LINE = "highlight_current_line";
inline constexpr const char *SHOW_WHITESPACE = "show_whitespace";
inline constexpr const char *SHOW_INDENT_GUIDES = "show_indent_guides";
inline constexpr const char *UI_SCALE = "ui_scale";
}

namespace preference {
inline constexpr const char *TAB_WIDTH = "tab_width";
inline constexpr const char *AUTO_FORMAT = "auto_format";
inline constexpr const char *BRACKET_MATCHING = "bracket_matching";
inline constexpr const char *AUTO_CLOSE_BRACKETS = "auto_close_brackets";
inline constexpr const char *WORD_WRAP = "word_wrap";
inline constexpr const char *TRIM_TRAILING_WHITESPACE_ON_SAVE = "trim_trailing_whitespace_on_save";
inline constexpr const char *HIGHLIGHT_ACTIVE_SCOPE = "highlight_active_scope";
inline constexpr const char *AUTO_INDENT = "auto_indent";
}

namespace input {
inline constexpr const char *SHORTCUT_SAVE = "shortcut_save";
inline constexpr const char *SHORTCUT_SEARCH = "shortcut_search";
inline constexpr const char *SHORTCUT_SPLIT_VERTICAL = "shortcut_split_vertical";
inline constexpr const char *SHORTCUT_SPLIT_HORIZONTAL = "shortcut_split_horizontal";
inline constexpr const char *VIM_MODE = "vim_mode";
inline constexpr const char *AUTOCOMPLETE = "autocomplete";
inline constexpr const char *CLIPBOARD_INTEGRATION = "clipboard_integration";
inline constexpr const char *MOUSE_SELECTION = "mouse_selection";
inline constexpr const char *DRAG_AND_DROP = "drag_and_drop";
}
namespace plugins {
inline constexpr const char *LSP = "lsp";
inline constexpr const char *SNIPPETS = "snippets";
inline constexpr const char *GIT = "git";
inline constexpr const char *LINTER = "linter";
inline constexpr const char *FILE_EXPLORER = "file_explorer";
inline constexpr const char *HOT_RELOAD = "hot_reload";
}

namespace file {
inline constexpr const char *AUTOSAVE_MODE = "autosave_mode";
inline constexpr const char *SHOW_HIDDEN_FILES = "show_hidden_files";
}

}
namespace defaults {
inline constexpr const int DEFAULT_OPACITY = 255;
namespace theme {
inline constexpr const Color BACKGROUND = (Color){0, 0, 0, DEFAULT_OPACITY};
inline constexpr const Color FOREGROUND = (Color){212, 212, 212, DEFAULT_OPACITY};
inline constexpr const Color CURSOR = (Color){255, 255, 255, DEFAULT_OPACITY};
inline constexpr const Color SELECTION = (Color){47, 47, 47, DEFAULT_OPACITY};
inline constexpr const Color LINE_NUMBER = (Color){133, 133, 133, DEFAULT_OPACITY};
inline constexpr const Color WHITESPACE = (Color){59, 59, 59, DEFAULT_OPACITY};
inline constexpr const Color DIFF_ADD = (Color){88, 124, 12, DEFAULT_OPACITY};
inline constexpr const Color DIFF_REMOVE = (Color){148, 21, 27, DEFAULT_OPACITY};
inline constexpr const Color DIFF_CHANGE = (Color){0, 90, 158, DEFAULT_OPACITY};
inline constexpr const Color DIAGNOSTIC_INFO = (Color){117, 190, 255, DEFAULT_OPACITY};
inline constexpr const Color DIAGNOSTIC_ERROR = (Color){244, 135, 113, DEFAULT_OPACITY};
inline constexpr const Color DIAGNOSTIC_WARNING = (Color){204, 167, 0, DEFAULT_OPACITY};
inline constexpr const Color POPUP_BACKGROUND = (Color){37, 37, 38, DEFAULT_OPACITY};
inline constexpr const Color TOOLTIP_BORDER = (Color){69, 69, 69, DEFAULT_OPACITY};
inline constexpr const Color COMPLETION_BACKGROUND = (Color){45, 45, 48, DEFAULT_OPACITY};
inline constexpr const Color HOVER_TAB_BACKGROUND = (Color){42, 45, 46, DEFAULT_OPACITY};
}

namespace font {
inline constexpr const char *FAMILY = "Arial";
inline constexpr const char *STYLE = "Regular";
inline constexpr const Color COLOR = (Color){212, 212, 212, DEFAULT_OPACITY};
inline constexpr const bool LIGATURES = false;
inline constexpr const Uint16 SIZE = 14;
inline constexpr const float LINE_HEIGHT = 1.5;
}

namespace ui {
inline constexpr const CursorStyleOpts CURSOR_STYLE = CursorStyleOpts::CursorBlock;
inline constexpr const LineNumberOpts LINE_NUMBERS = LineNumberOpts::LineAbsolute;
inline constexpr const bool STATUS_BAR_VISIBLE = false;
inline constexpr const bool TAB_BAR_VISIBLE = false;
inline constexpr const bool HIGHLIGHT_CURRENT_LINE = false;
inline constexpr const bool SHOW_WHITESPACE = true;
inline constexpr const bool SHOW_INDENT_GUIDES = false;
inline constexpr const Uint16 UI_SCALE = 100;
}

namespace preference {
inline constexpr const Uint16 TAB_WIDTH = 4;
inline constexpr const AutoFormatOpts AUTO_FORMAT = AutoFormatOpts::FormatOnSave;
inline constexpr const bool BRACKET_MATCHING = true;
inline constexpr const bool AUTO_CLOSE_BRACKETS = true;
inline constexpr const bool WORD_WRAP = false;
inline constexpr const bool TRIM_TRAILING_WHITESPACE_ON_SAVE = true;
inline constexpr const bool HIGHLIGHT_ACTIVE_SCOPE = true;
inline constexpr const bool AUTO_INDENT = true;
}

namespace input {
inline constexpr const Shortcut SHORTCUT_SAVE = {KMOD_CTRL, SDLK_s};
inline constexpr const Shortcut SHORTCUT_SEARCH = {KMOD_CTRL, SDLK_f};
inline constexpr const Shortcut SHORTCUT_SPLIT_VERTICAL = {(KMOD_CTRL | KMOD_SHIFT), SDLK_v};
inline constexpr const Shortcut SHORTCUT_SPLIT_HORIZONTAL = {(KMOD_CTRL | KMOD_SHIFT), SDLK_h};
inline constexpr const bool VIM_MODE = false;
inline constexpr const bool AUTOCOMPLETE = true;
inline constexpr const bool CLIPBOARD_INTEGRATION = true;
inline constexpr const bool MOUSE_SELECTION = true;
inline constexpr const bool DRAG_AND_DROP = false;
}

namespace plugins {
inline constexpr const bool LSP = false;
inline constexpr const bool SNIPPETS = false;
inline constexpr const bool GIT = false;
inline constexpr const bool LINTER = false;
inline constexpr const bool FILE_EXPLORER = false;
inline constexpr const bool HOT_RELOAD = true;
}

namespace file {
inline constexpr const AutoSaveModeOpts AUTOSAVE_MODE = AutoSaveModeOpts::SaveOnFocus;
inline constexpr const bool SHOW_HIDDEN_FILES = false;
}
}
}
