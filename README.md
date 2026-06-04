# Blip Text Editor

A high-performance text editor built from scratch, focusing on low-level systems programming, Vim emulation, and efficient cross-platform file watching. Built with a custom hardware-accelerated rendering pipeline and semantic syntax awareness.

---

## ✨ Core Architecture

- **Semantic Syntax Highlighting:** Integrates **Tree-sitter** for live Abstract Syntax Tree (AST) generation, providing compiler-accurate, context-aware code coloring — bypassing the limitations of standard regex.
- **Hardware-Accelerated Rendering:** Utilizes an O(1) **Glyph Cache (Texture Atlas)**. Characters are pre-rendered as pure white textures in VRAM and tinted on the fly using GPU hardware color modulation.
- **Viewport Culling:** Implements an O(V) rendering pipeline. The engine mathematically culls lines outside the camera bounds, ensuring stable 60 FPS regardless of file size.
- **Vim Emulation:** Custom input state machine supporting Normal, Insert, Visual, and Command modes with accurate cursor behavior and action dispatching.

---

## 🛠 Tech Stack & APIs

| Purpose | Library / API |
|---|---|
| Build System | [CMake](https://cmake.org/) |
| Window Management | [SDL2](https://www.libsdl.org/) |
| Font Rendering | [SDL2_ttf](https://github.com/libsdl-org/SDL_ttf) |
| Syntax Parsing | [Tree-sitter](https://tree-sitter.github.io/tree-sitter/) |
| File Watching (macOS) | [kqueue](https://man.freebsd.org/cgi/man.cgi?query=kqueue) |
| File Watching (Linux) | [inotify](https://man7.org/linux/man-pages/man7/inotify.7.html) |

---

## 📖 Table of Contents

1. [Configuration Rules](#configuration-rules)
2. [Theme Configuration](#theme-configuration)
3. [Font Configuration](#font-configuration)
4. [UI Configuration](#ui-configuration)
5. [Preferences Configuration](#preferences-configuration)
6. [Input Configuration](#input-configuration)
7. [Plugins](#plugins)
8. [File Configuration](#file-configuration)

---

## Configuration Rules

| Type | Format |
|---|---|
| Booleans | `true` or `false` |
| Colors | RGBA Hex — `#RRGGBBAA` (e.g. `#FF5733FF`) |
| Shortcuts | Modifier + key, separated by `+` (e.g. `Control+s`) |
| Case Sensitivity | Keybinds are case-sensitive |

---

## Theme Configuration

```sh
background            = #000000
foreground            = #D4D4D4
cursor                = #FFFFFF
selection             = #2F2F2F
line_number           = #858585
whitespace            = #3B3B3B
diff_add              = #587C0C
diff_remove           = #94151B
diff_change           = #005A9E
diagnostic_info       = #75BEFF
diagnostic_error      = #F48771
diagnostic_warning    = #CCA700
popup_background      = #252526
tooltip_border        = #454545
completion_background = #2D2D30
hover_tab_background  = #2A2D2E
```

---

## Font Configuration

```sh
font_family    = JetBrains Mono
font_color     = #D4D4D4
font_ligatures = true
font_size      = 14
line_height    = 1.5
```

---

## UI Configuration

```sh
cursor_style           = 0     # 0 = Block, 1 = Line
line_numbers           = 0     # 0 = Absolute, 1 = Relative, 2 = Hidden, 3 = AbsoluteAndRelative
status_bar_visible     = true
tab_bar_visible        = true
highlight_current_line = true
show_whitespace        = false
show_indent_guides     = true
ui_scale               = 100   # Percent-based
```

---

## Preferences Configuration

```sh
tab_width                        = 4
auto_format                      = 0     # 0 = Manual, 1 = OnSave, 2 = OnPaste
bracket_matching                 = true
auto_close_brackets              = true
word_wrap                        = false
trim_trailing_whitespace_on_save = true
highlight_active_scope           = true
auto_indent                      = true
```

---

## Input Configuration

```sh
shortcut_save             = Control+s
shortcut_search           = Control+f
shortcut_split_vertical   = Control+V
shortcut_split_horizontal = Control+H
vim_mode                  = true
autocomplete              = true
clipboard_integration     = true
mouse_selection           = true
drag_and_drop             = false
```

---

## Plugins

```sh
tree_sitter   = true
lsp           = false
snippets      = false
git           = false
linter        = false
file_explorer = false
hot_reload    = true
```

---

## File Configuration

```sh
autosave_mode        = 0                              # 0 = SaveOnFocus, 1 = SaveDelay, 2 = SaveManual
exclude_patterns     = [".git", "node_modules", "dist"]
show_hidden_files    = false
```