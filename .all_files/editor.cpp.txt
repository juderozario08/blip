#include <SDL.h>
#include <blip/config/editor.hpp>
#include <charconv>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace config {
std::string trim(const std::string &s) {
    size_t first = s.find_first_not_of(" \t\r\n");
    if (std::string::npos == first)
        return "";
    size_t last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, (last - first + 1));
}

std::pair<std::string, std::string> parseLine(std::string line) {
    if (line.empty() || line[0] == '#' || line[0] == ';') {
        return {"", ""};
    }

    size_t delimiterPos = line.find('=');
    if (delimiterPos != std::string::npos) {
        std::string key = trim(line.substr(0, delimiterPos));
        std::string value = trim(line.substr(delimiterPos + 1));
        return {key, value};
    }

    return {"", ""};
}

template <typename T> bool parseNum(std::string_view sub, T &out, int base = 10) {
    if constexpr (std::is_same_v<int, T> || std::is_same_v<Uint8, T>) {
        auto [ptr, ec] = std::from_chars(sub.data(), sub.data() + sub.size(), out, base);
        return ec == std::errc{};
    }
    return false;
}

std::optional<Color> hexToColor(std::string_view hex) {
    if (hex.empty() || hex[0] != '#' || (hex.length() != 7 && hex.length() != 9)) {
        return std::nullopt;
    }

    Color color;
    if (!parseNum(hex.substr(1, 2), color.r, 16))
        return std::nullopt;
    if (!parseNum(hex.substr(3, 2), color.g, 16))
        return std::nullopt;
    if (!parseNum(hex.substr(5, 2), color.b, 16))
        return std::nullopt;

    if (hex.length() == 9) {
        if (!parseNum(hex.substr(7, 2), color.a))
            return std::nullopt;
    } else {
        color.a = 255;
    }

    return color;
}

void handleColorUpdate(std::string value, Color *color, Color def) {
    if (auto c = hexToColor(value)) {
        *color = *c;
    } else {
        *color = def;
    }
}

void handleBoolUpdate(std::string value, bool *dst, bool def) {
    if (value == "true") {
        *dst = true;
    } else if (value == "false") {
        *dst = false;
    } else {
        *dst = def;
    }
}

void handleShortcutString(std::string value, Shortcut &dst, Shortcut def) {
    if (value.empty()) {
        dst = def;
    }
    std::stringstream ss(value);
    std::string segment;
    std::vector<std::string> parts;

    while (std::getline(ss, segment, '+')) {
        parts.push_back(segment);
    }

    if (parts.size() < 2) {
        dst = def;
        return;
    }

    Uint16 first_mod = 0;
    if (parts[0] == "Control") {
        first_mod = KMOD_CTRL;
    } else if (parts[0] == "Alt") {
        first_mod = KMOD_ALT;
    }

    if (first_mod == 0) {
        dst = def;
        return;
    }

    Uint16 detected_mods = first_mod;
    SDL_Keycode detected_key = SDLK_UNKNOWN;

    for (size_t i = 1; i < parts.size(); i++) {
        if (parts[i] == "Control") {
            detected_mods |= KMOD_CTRL;
        } else if (parts[i] == "Alt") {
            detected_mods |= KMOD_ALT;
        } else if (parts[i] == "Shift") {
            detected_mods |= KMOD_SHIFT;
        } else {
            detected_key = SDL_GetKeyFromName(parts[i].c_str());
        }
    }

    if (detected_key == SDLK_UNKNOWN) {
        dst = def;
    } else {
        dst.modifiers = detected_mods;
        dst.key = detected_key;
    }
}

void setDefaultConifg(EditorConfig &state) {
    state.theme.background = defaults::theme::BACKGROUND;
    state.theme.foreground = defaults::theme::FOREGROUND;
    state.theme.cursor = defaults::theme::CURSOR;
    state.theme.selection = defaults::theme::SELECTION;
    state.theme.line_number = defaults::theme::LINE_NUMBER;
    state.theme.whitespace = defaults::theme::WHITESPACE;
    state.theme.diff_add = defaults::theme::DIFF_ADD;
    state.theme.diff_change = defaults::theme::DIFF_CHANGE;
    state.theme.diff_remove = defaults::theme::DIFF_REMOVE;
    state.theme.diagnostic_info = defaults::theme::DIAGNOSTIC_INFO;
    state.theme.diagnostic_error = defaults::theme::DIAGNOSTIC_ERROR;
    state.theme.diagnostic_warning = defaults::theme::DIAGNOSTIC_WARNING;
    state.theme.popup_background = defaults::theme::POPUP_BACKGROUND;
    state.theme.tooltip_border = defaults::theme::TOOLTIP_BORDER;
    state.theme.completion_background = defaults::theme::COMPLETION_BACKGROUND;

    state.font.family = defaults::font::FAMILY;
    state.font.color = defaults::font::COLOR;
    state.font.ligatures = defaults::font::LIGATURES;
    state.font.size = defaults::font::SIZE;
    state.font.line_height = defaults::font::LINE_HEIGHT;

    state.ui.cursor_style = defaults::ui::CURSOR_STYLE;
    state.ui.line_numbers = defaults::ui::LINE_NUMBERS;
    state.ui.status_bar_visible = defaults::ui::STATUS_BAR_VISIBLE;
    state.ui.tab_bar_visible = defaults::ui::TAB_BAR_VISIBLE;
    state.ui.highlight_current_line = defaults::ui::HIGHLIGHT_CURRENT_LINE;
    state.ui.show_whitespace = defaults::ui::SHOW_WHITESPACE;
    state.ui.show_indent_guides = defaults::ui::SHOW_INDENT_GUIDES;
    state.ui.ui_scale = defaults::ui::UI_SCALE;

    state.preference.tab_width = defaults::preference::TAB_WIDTH;
    state.preference.auto_format = defaults::preference::AUTO_FORMAT;
    state.preference.auto_indent = defaults::preference::AUTO_INDENT;
    state.preference.auto_close_brackets = defaults::preference::AUTO_CLOSE_BRACKETS;
    state.preference.bracket_matching = defaults::preference::BRACKET_MATCHING;
    state.preference.word_wrap = defaults::preference::WORD_WRAP;
    state.preference.trim_trailing_whitespace_on_save = defaults::preference::TRIM_TRAILING_WHITESPACE_ON_SAVE;
    state.preference.highlight_active_scope = defaults::preference::HIGHLIGHT_ACTIVE_SCOPE;

    state.input.shortcut_save = defaults::input::SHORTCUT_SAVE;
    state.input.shortcut_search = defaults::input::SHORTCUT_SEARCH;
    state.input.shortcut_split_horizontal = defaults::input::SHORTCUT_SPLIT_HORIZONTAL;
    state.input.shortcut_split_vertical = defaults::input::SHORTCUT_SPLIT_VERTICAL;
    state.input.vim_mode = defaults::input::VIM_MODE;
    state.input.autocomplete = defaults::input::AUTOCOMPLETE;
    state.input.clipboard_integration = defaults::input::CLIPBOARD_INTEGRATION;
    state.input.mouse_selection = defaults::input::MOUSE_SELECTION;
    state.input.drag_and_drop = defaults::input::DRAG_AND_DROP;

    state.plugins.file_explorer = defaults::plugins::FILE_EXPLORER;
    state.plugins.snippets = defaults::plugins::SNIPPETS;
    state.plugins.git = defaults::plugins::GIT;
    state.plugins.hot_reload = defaults::plugins::HOT_RELOAD;
    state.plugins.linter = defaults::plugins::LINTER;
    state.plugins.lsp = defaults::plugins::LSP;

    state.file.show_hidden_files = defaults::file::SHOW_HIDDEN_FILES;
    state.file.autosave_mode = defaults::file::AUTOSAVE_MODE;
}

void handleThemeConfigUpdates(std::string key, std::string value, EditorConfig &state) {
    if (key == constants::theme::BACKGROUND) {
        handleColorUpdate(value, &state.theme.background, defaults::theme::BACKGROUND);
    } else if (key == constants::theme::FOREGROUND) {
        handleColorUpdate(value, &state.theme.foreground, defaults::theme::FOREGROUND);
    } else if (key == constants::theme::CURSOR) {
        handleColorUpdate(value, &state.theme.cursor, defaults::theme::CURSOR);
    } else if (key == constants::theme::SELECTION) {
        handleColorUpdate(value, &state.theme.selection, defaults::theme::SELECTION);
    } else if (key == constants::theme::LINE_NUMBER) {
        handleColorUpdate(value, &state.theme.line_number, defaults::theme::LINE_NUMBER);
    } else if (key == constants::theme::WHITESPACE) {
        handleColorUpdate(value, &state.theme.whitespace, defaults::theme::WHITESPACE);
    } else if (key == constants::theme::DIFF_ADD) {
        handleColorUpdate(value, &state.theme.diff_add, defaults::theme::DIFF_ADD);
    } else if (key == constants::theme::DIFF_REMOVE) {
        handleColorUpdate(value, &state.theme.diff_remove, defaults::theme::DIFF_REMOVE);
    } else if (key == constants::theme::DIFF_CHANGE) {
        handleColorUpdate(value, &state.theme.diff_change, defaults::theme::DIFF_CHANGE);
    } else if (key == constants::theme::DIAGNOSTIC_INFO) {
        handleColorUpdate(value, &state.theme.diagnostic_info, defaults::theme::DIAGNOSTIC_INFO);
    } else if (key == constants::theme::DIAGNOSTIC_ERROR) {
        handleColorUpdate(value, &state.theme.diagnostic_error, defaults::theme::DIAGNOSTIC_ERROR);
    } else if (key == constants::theme::DIAGNOSTIC_WARNING) {
        handleColorUpdate(value, &state.theme.diagnostic_warning, defaults::theme::DIAGNOSTIC_WARNING);
    } else if (key == constants::theme::POPUP_BACKGROUND) {
        handleColorUpdate(value, &state.theme.popup_background, defaults::theme::POPUP_BACKGROUND);
    } else if (key == constants::theme::TOOLTIP_BORDER) {
        handleColorUpdate(value, &state.theme.tooltip_border, defaults::theme::TOOLTIP_BORDER);
    } else if (key == constants::theme::COMPLETION_BACKGROUND) {
        handleColorUpdate(value, &state.theme.completion_background, defaults::theme::COMPLETION_BACKGROUND);
    } else if (key == constants::theme::HOVER_TAB_BACKGROUND) {
        handleColorUpdate(value, &state.theme.hover_tab_background, defaults::theme::HOVER_TAB_BACKGROUND);
    }
}
void handleFontConfigUpdates(std::string key, std::string value, EditorConfig &state) {
    // Font Config Updates
    if (key == constants::font::FAMILY) {
        // TODO: MAKE SURE TO CHECK IF THE "VALUE = FONT" FIRST EXISTS OR NOT
        state.font.family = value.length() > 0 ? value : defaults::font::FAMILY;
    } else if (key == constants::font::STYLE) {
        state.font.style = value.length() > 0 ? value : defaults::font::STYLE;
    } else if (key == constants::font::COLOR) {
        handleColorUpdate(value, &state.font.color, defaults::font::COLOR);
    } else if (key == constants::font::LIGATURES) {
        handleBoolUpdate(value, &state.font.ligatures, defaults::font::LIGATURES);
    } else if (key == constants::font::SIZE) {
        if (!parseNum(value, state.font.size)) {
            state.font.size = defaults::font::SIZE;
        }
    } else if (key == constants::font::LINE_HEIGHT) {
        try {
            auto height = static_cast<float>(std::stof(value));
            state.font.line_height = height <= 0.0 ? defaults::font::LINE_HEIGHT : height;
        } catch (...) {
            state.font.line_height = defaults::font::LINE_HEIGHT;
        }
    }
}
void handleUIConfigUpdates(std::string key, std::string value, EditorConfig &state) {
    if (key == constants::ui::CURSOR_STYLE) {
        int style;
        if (!parseNum(value, style)) {
            state.ui.cursor_style = defaults::ui::CURSOR_STYLE;
        } else {
            if (style > 0 && style < static_cast<int>(CursorStyleOpts::COUNT)) {
                switch (auto ns = static_cast<CursorStyleOpts>(style)) {
                case CursorStyleOpts::CursorBlock:
                case CursorStyleOpts::CursorLine:
                    state.ui.cursor_style = ns;
                    break;
                default:
                    state.ui.cursor_style = defaults::ui::CURSOR_STYLE;
                    break;
                }
            } else {
                state.ui.cursor_style = defaults::ui::CURSOR_STYLE;
            }
        }
    } else if (key == constants::ui::LINE_NUMBERS) {
        int style;
        if (!parseNum(value, style)) {
            state.ui.line_numbers = defaults::ui::LINE_NUMBERS;
        } else {
            if (style > 0 && style < static_cast<int>(LineNumberOpts::COUNT)) {
                switch (auto ln = static_cast<LineNumberOpts>(style)) {
                case LineNumberOpts::LineAbsolute:
                case LineNumberOpts::LineHidden:
                case LineNumberOpts::LineRelative:
                case LineNumberOpts::LineAbsoluteAndRelative:
                    state.ui.line_numbers = ln;
                    break;
                default:
                    state.ui.line_numbers = defaults::ui::LINE_NUMBERS;
                    break;
                }
            } else {
                state.ui.line_numbers = defaults::ui::LINE_NUMBERS;
            }
        }
    } else if (key == constants::ui::STATUS_BAR_VISIBLE) {
        handleBoolUpdate(value, &state.ui.status_bar_visible, defaults::ui::STATUS_BAR_VISIBLE);
    } else if (key == constants::ui::TAB_BAR_VISIBLE) {
        handleBoolUpdate(value, &state.ui.tab_bar_visible, defaults::ui::TAB_BAR_VISIBLE);
    } else if (key == constants::ui::HIGHLIGHT_CURRENT_LINE) {
        handleBoolUpdate(value, &state.ui.highlight_current_line, defaults::ui::HIGHLIGHT_CURRENT_LINE);
    } else if (key == constants::ui::SHOW_WHITESPACE) {
        handleBoolUpdate(value, &state.ui.show_whitespace, defaults::ui::SHOW_WHITESPACE);
    } else if (key == constants::ui::SHOW_INDENT_GUIDES) {
        handleBoolUpdate(value, &state.ui.show_indent_guides, defaults::ui::SHOW_INDENT_GUIDES);
    } else if (key == constants::ui::UI_SCALE) {
        int scale;
        if (!parseNum(value, scale)) {
            state.ui.ui_scale = defaults::ui::UI_SCALE;
        } else {
            state.ui.ui_scale = scale <= 5 ? defaults::ui::UI_SCALE : scale;
        }
    }
}
void handlePreferenceConfigUpdates(std::string key, std::string value, EditorConfig &state) {
    // Preference Config Update
    if (key == constants::preference::TAB_WIDTH) {
        int width;
        if (!parseNum(value, width)) {
            state.preference.tab_width = width <= 2 ? defaults::preference::TAB_WIDTH : width;
        } else {
            state.preference.tab_width = defaults::preference::TAB_WIDTH;
        }
    } else if (key == constants::preference::AUTO_FORMAT) {
        int format;
        if (!parseNum(value, format)) {
            state.preference.auto_format = defaults::preference::AUTO_FORMAT;
        } else {
            if (format > 0 && format < static_cast<int>(AutoFormatOpts::COUNT)) {
                switch (auto fmt = static_cast<AutoFormatOpts>(format)) {
                case AutoFormatOpts::FormatManual:
                case AutoFormatOpts::FormatOnSave:
                case AutoFormatOpts::FormatOnPaste:
                    state.preference.auto_format = fmt;
                    break;
                default:
                    state.preference.auto_format = defaults::preference::AUTO_FORMAT;
                    break;
                }
            } else {
                state.preference.auto_format = defaults::preference::AUTO_FORMAT;
            }
        }
    } else if (key == constants::preference::BRACKET_MATCHING) {
        handleBoolUpdate(value, &state.preference.bracket_matching, defaults::preference::BRACKET_MATCHING);
    } else if (key == constants::preference::AUTO_CLOSE_BRACKETS) {
        handleBoolUpdate(value, &state.preference.auto_close_brackets, defaults::preference::AUTO_CLOSE_BRACKETS);
    } else if (key == constants::preference::WORD_WRAP) {
        handleBoolUpdate(value, &state.preference.word_wrap, defaults::preference::WORD_WRAP);
    } else if (key == constants::preference::TRIM_TRAILING_WHITESPACE_ON_SAVE) {
        handleBoolUpdate(value, &state.preference.trim_trailing_whitespace_on_save,
                         defaults::preference::TRIM_TRAILING_WHITESPACE_ON_SAVE);
    } else if (key == constants::preference::HIGHLIGHT_ACTIVE_SCOPE) {
        handleBoolUpdate(value, &state.preference.highlight_active_scope, defaults::preference::HIGHLIGHT_ACTIVE_SCOPE);
    } else if (key == constants::preference::AUTO_INDENT) {
        handleBoolUpdate(value, &state.preference.auto_indent, defaults::preference::AUTO_INDENT);
    }
}
void handleInputConfigUpdates(std::string key, std::string value, EditorConfig &state) {
    // Input Config Update
    if (key == constants::input::SHORTCUT_SAVE) {
        handleShortcutString(value, state.input.shortcut_save, defaults::input::SHORTCUT_SAVE);
    } else if (key == constants::input::SHORTCUT_SEARCH) {
        handleShortcutString(value, state.input.shortcut_search, defaults::input::SHORTCUT_SEARCH);
    } else if (key == constants::input::SHORTCUT_SPLIT_VERTICAL) {
        handleShortcutString(value, state.input.shortcut_split_vertical, defaults::input::SHORTCUT_SPLIT_VERTICAL);
    } else if (key == constants::input::SHORTCUT_SPLIT_HORIZONTAL) {
        handleShortcutString(value, state.input.shortcut_split_horizontal, defaults::input::SHORTCUT_SPLIT_HORIZONTAL);
    } else if (key == constants::input::VIM_MODE) {
        handleBoolUpdate(value, &state.input.vim_mode, defaults::input::VIM_MODE);
    } else if (key == constants::input::AUTOCOMPLETE) {
        handleBoolUpdate(value, &state.input.autocomplete, defaults::input::AUTOCOMPLETE);
    } else if (key == constants::input::CLIPBOARD_INTEGRATION) {
        handleBoolUpdate(value, &state.input.clipboard_integration, defaults::input::CLIPBOARD_INTEGRATION);
    } else if (key == constants::input::MOUSE_SELECTION) {
        handleBoolUpdate(value, &state.input.mouse_selection, defaults::input::MOUSE_SELECTION);
    } else if (key == constants::input::DRAG_AND_DROP) {
        handleBoolUpdate(value, &state.input.drag_and_drop, defaults::input::DRAG_AND_DROP);
    }
}
void handlePluginsConfigUpdates(std::string key, std::string value, EditorConfig &state) {
    // Plugins Config Updates
    if (key == constants::plugins::LSP) {
        handleBoolUpdate(value, &state.plugins.lsp, defaults::plugins::LSP);
    } else if (key == constants::plugins::SNIPPETS) {
        handleBoolUpdate(value, &state.plugins.snippets, defaults::plugins::SNIPPETS);
    } else if (key == constants::plugins::GIT) {
        handleBoolUpdate(value, &state.plugins.git, defaults::plugins::GIT);
    } else if (key == constants::plugins::LINTER) {
        handleBoolUpdate(value, &state.plugins.linter, defaults::plugins::LINTER);
    } else if (key == constants::plugins::FILE_EXPLORER) {
        handleBoolUpdate(value, &state.plugins.file_explorer, defaults::plugins::FILE_EXPLORER);
    } else if (key == constants::plugins::HOT_RELOAD) {
        handleBoolUpdate(value, &state.plugins.hot_reload, defaults::plugins::HOT_RELOAD);
    }
}
void handleFileConfigUpdates(std::string key, std::string value, EditorConfig &state) {
    // File Config Updates
    if (key == constants::file::AUTOSAVE_MODE) {
        int m;
        if (!parseNum(value, m)) {
            state.file.autosave_mode = defaults::file::AUTOSAVE_MODE;
        } else {
            if (m > 0 && m < static_cast<int>(AutoSaveModeOpts::COUNT)) {
                switch (auto mode = static_cast<AutoSaveModeOpts>(m)) {
                case AutoSaveModeOpts::SaveManual:
                case AutoSaveModeOpts::SaveDelay:
                case AutoSaveModeOpts::SaveOnFocus:
                    state.file.autosave_mode = mode;
                    break;
                default:
                    state.file.autosave_mode = defaults::file::AUTOSAVE_MODE;
                    break;
                }
            } else {
                state.file.autosave_mode = defaults::file::AUTOSAVE_MODE;
            }
        }
    } else if (key == constants::file::AUTOSAVE_MODE) {
        handleBoolUpdate(value, &state.file.show_hidden_files, defaults::file::SHOW_HIDDEN_FILES);
    }
}

void updateState(std::string key, std::string value, EditorConfig &state) {
    if (key.empty() || value.empty()) {
        return;
    }
    handleThemeConfigUpdates(key, value, state);
    handleFontConfigUpdates(key, value, state);
    handleUIConfigUpdates(key, value, state);
    handlePreferenceConfigUpdates(key, value, state);
    handleInputConfigUpdates(key, value, state);
    handlePluginsConfigUpdates(key, value, state);
    handleFileConfigUpdates(key, value, state);
}

void loadConfig(std::string filepath, EditorConfig &state) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open " << filepath << std::endl;
        return;
    }
    std::string line;
    while (std::getline(file, line)) {
        auto [key, value] = parseLine(line);
        updateState(key, value, state);
    }
}

// TODO: ADD THE COLOR ADDING COMPONENTS AS WE GO ALONG THE DESIGN
void setForegroundColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.foreground;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
}
void setCursorColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.cursor;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    int h = (int)(state.font.line_height * 10);
    auto rect =
        (SDL_Rect){positions::x::LINE_NUMBER + positions::width::LINE_NUMBERS + 5, 0, 15, (int)(state.font.line_height * 10)};
    SDL_RenderFillRect(appState.renderer, &rect);
}
void setSelectionColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.selection;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
}
void setLineNumberColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.line_number;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    auto rect = (SDL_Rect){positions::x::LINE_NUMBER, 0, positions::width::LINE_NUMBERS, appState.window_height};
    SDL_RenderFillRect(appState.renderer, &rect);
}
void setWhitespaceColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.whitespace;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
}

// NOTE: Height will be determined based on the line that has the git log but on top of the line_number rectangle
void setDiffAddColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.diff_add;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    auto rect = (SDL_Rect){positions::x::GIT, 0, positions::width::GIT, 20};
    SDL_RenderFillRect(appState.renderer, &rect);
}
void setDiffRemoveColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.diff_remove;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    auto rect = (SDL_Rect){positions::x::GIT, 20, positions::width::GIT, 20};
    SDL_RenderFillRect(appState.renderer, &rect);
}
void setDiffChangeColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.diff_change;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    auto rect = (SDL_Rect){positions::x::GIT, 40, positions::width::GIT, 20};
    SDL_RenderFillRect(appState.renderer, &rect);
}

// NOTE: Height will be based on line number with diagnostic stuff based on the LSP
void setDiagnosticErrorColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.diagnostic_error;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    auto h = (int)(state.font.line_height * 10);
    auto rect = (SDL_Rect){positions::x::DIAGNOSTIC, 0, positions::width::DIAGNOSTIC, h};
    SDL_RenderFillRect(appState.renderer, &rect);
}
void setDiagnosticWarningColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.diagnostic_warning;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    auto h = (int)(state.font.line_height * 10);
    auto rect = (SDL_Rect){positions::x::DIAGNOSTIC, h, positions::width::DIAGNOSTIC, h};
    SDL_RenderFillRect(appState.renderer, &rect);
}
void setDiagnosticInfoColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.diagnostic_info;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    auto h = (int)(state.font.line_height * 10);
    auto rect = (SDL_Rect){positions::x::DIAGNOSTIC, h * 2, positions::width::DIAGNOSTIC, h};
    SDL_RenderFillRect(appState.renderer, &rect);
}

void setPopupBackgroundColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.popup_background;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
}
void setTooltipBorderColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.tooltip_border;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
}
void setCompletionBackgroundColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.completion_background;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
}
void setHoverBackgroundColor(app::AppState &appState, EditorConfig &state) {
    auto c = state.theme.hover_tab_background;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
}
}
