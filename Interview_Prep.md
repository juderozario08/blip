# Part 1: Build System & Application Architecture 

Here, we'll look at how your application is glued together, how it talks to the OS, and how the main event loop operates. We'll be looking at `CMakeLists.txt`, `app/main.hpp`, and `app/main.cpp`.

## 1. `CMakeLists.txt` - The Build System
This file tells the compiler how to build your project. For a Systems/Trading role, knowing how your code becomes an executable is crucial.

*   `set(CMAKE_CXX_STANDARD 20)`: You are using **C++20**. This shows you are up-to-date with modern C++ standards, which introduced concepts like concepts, ranges, and coroutines.
*   `set(BUILD_SHARED_LIBS OFF ...)` and `set(SDL_STATIC ON ...)`: You are explicitly building third-party libraries (like SDL2) **statically**. 
    *   *Interview Talking Point:* Static linking means the library code is baked directly into your executable. In low-latency trading, static linking is often preferred because it avoids the overhead of dynamic library loading at runtime and ensures there are no missing dependency issues on the production servers.
*   `if (APPLE) ... elseif (UNIX AND NOT APPLE)`: You are using **Conditional Compilation** at the build level to link different platform-specific system libraries (like `CoreText` for macOS vs `Fontconfig` for Linux). This shows cross-platform systems knowledge.

## 2. `main.hpp` - The Global State Struct
```cpp
namespace app {
typedef struct AppState {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int window_height, window_width;
    int scroll_y = 0;
    int scroll_x = 0;
    std::string filepath;
} AppState;
}
```
*   **Namespaces (`namespace app`):** You use namespaces to group logic and prevent "symbol collisions" (e.g., if two different libraries both had a class named `AppState`).
*   **Raw Pointers (`*window`, `*renderer`):** You are using raw C-style pointers here. Why? Because SDL2 is a C library, and it manages the memory for the Window and Renderer under the hood. You just hold a pointer to the memory it allocated.
*   **Struct Initialization:** Notice how `scroll_y = 0;` has a default value. In modern C++, this is called *default member initialization*.

## 3. `main.cpp` - The Entry Point & Core Logic

Let's walk through the most important C++ features you used in your main file.

### A. Preprocessor Macros
```cpp
#ifdef _DEV_
#define DEV(...) __VA_ARGS__
#else
#define DEV(...)
#endif
```
*   *Interview Talking Point:* Macros are resolved *before* the code is even compiled. Here, if `_DEV_` isn't defined, `DEV(...)` literally becomes empty space. In high-frequency trading (HFT), logging is very slow (I/O operation). You use macros like this so that in production builds, all debug logging is completely stripped from the final binary, resulting in **zero overhead**.

### B. Pass-by-Const-Reference (CRITICAL for Performance)
```cpp
std::string getFileExtension(const std::string &filename) { ... }
```
*   *Interview Talking Point:* This is a classic C++ interview question. You pass `filename` as `const std::string &`. The `&` means you are passing a memory reference, **not** copying the string. Copying a string allocates heap memory (slow). The `const` guarantees your function won't accidentally modify it. Always emphasize that you pass large objects by const reference to avoid expensive memory allocations.

### C. OS Interaction & Standard Library
```cpp
std::filesystem::create_directories(pluginDir);
// ...
system(curlCmd.c_str());
```
*   `std::filesystem`: A modern C++17 feature for cross-platform file manipulation.
*   `system(...)`: You use this to spawn a shell process to download tree-sitter. *Self-critique for the interview:* Mention that while `system()` is fine for an init script in an editor, in a high-performance trading app, you'd never use `system()` because spinning up an entire shell process takes milliseconds (an eternity in HFT) and is a security risk.

### D. The State Machine & Jump Tables
```cpp
bool dispatchActions(const std::vector<input::Action> &actions, ...) {
    for (const auto &action : actions) {
        switch (action.type) {
            case input::ActionType::MoveLeft: ...
```
*   **`const auto &`**: Again, using `const auto &` in your for-loop prevents copying the action objects.
*   **Enum Classes & Switches:** `ActionType` is an `enum class`. The compiler usually optimizes `switch` statements over enums into a **Jump Table**. This means instead of checking `if A, else if B, else if C` (O(N) time), it calculates the exact memory address to jump to in O(1) time. This is exactly how trading engines parse incoming market data packets quickly.

### E. RAII and Smart Pointers
```cpp
auto glyphCache = std::make_unique<ui::GlyphCache>(appState.renderer, fonts.getFont(), pureWhite);
```
*   *Interview Talking Point:* This is **RAII (Resource Acquisition Is Initialization)**. You are allocating memory on the heap for the `GlyphCache` using `std::make_unique`. It returns a `std::unique_ptr`. When `glyphCache` goes out of scope (when the function ends), C++ automatically deletes the memory for you. This guarantees **no memory leaks**, a critical requirement for trading bots that run 24/5 without restarting.

### F. The Event Loop
```cpp
while (running) {
    SDL_WaitEventTimeout(NULL, 250);
    while (SDL_PollEvent(&event) != 0) { ... }
```
*   This is an asynchronous polling loop. Instead of blocking and waiting for a key press, it continuously checks the event queue (`SDL_PollEvent`). Trading algorithms use this exact same pattern (often using `epoll` on Linux) to constantly drain network sockets for new market prices without ever freezing the program.
# Algorithmic Trading C++ Interview Deep Dive - Parts 2, 3, and 4

This document serves as an exhaustive, line-by-line analysis of your C++ codebase segments (Parts 2, 3, and 4), emphasizing modern C++ features, memory management, algorithmic complexity, threading, concurrency, and cross-platform design. These concepts are foundational for algorithmic trading interviews, where latency, resource management, and correctness are heavily scrutinized.

---

## Part 2: Core Data Structures - The Piece Table

### `include/blip/buffer/table.hpp` & `src/buffer/table.cpp`

The `PieceTable` is a classic data structure for text editors, enabling efficient inserts, deletes, and undo/redo operations without constantly shifting massive blocks of memory (unlike a naïve `std::string` or `std::vector<char>` approach, which would be O(N) for every insertion).

#### Memory and Design Considerations
- **Enums & Structs**: `BufType` is an `enum class`, which is strongly typed and scoped. This avoids the implicit integer conversions of C-style enums. `Piece` holds the metadata (`source`, `start`, `length`) for a contiguous block of text. Notice that `Piece` only contains a few primitive types (`size_t` and the enum), meaning it is trivially copyable and very cache-friendly.
- **Vectors**: `std::vector<Piece> pieces` manages the pieces dynamically. `std::vector` guarantees contiguous memory layout, maximizing cache locality during iterations. Searching for a piece takes O(N) relative to the *number of pieces*, not the number of characters. Over time, as edits happen, the number of pieces grows. In a high-frequency trading context, contiguous storage is vital to avoid cache misses.
- **Buffers**: The strings `original_buffer` and `add_buffer` act as the backing memory. Appending to `add_buffer` (via `std::string::operator+=`) amortizes to O(1) time complexity. Crucially, existing memory in these strings is *never* shifted or deleted, ensuring pointers or indices to them remain valid.

#### Deep Dive into `table.cpp` Methods

**`insert(size_t index, const std::string &text)`**
- Complexity: O(P) where P is the number of pieces.
- Logic: It appends the new `text` to the `add_buffer`. It then iterates through the `pieces` vector to find the piece that overlaps with the logical `index`.
- When it finds the correct piece, it may need to split the existing piece into two and insert the new piece in the middle. This requires calling `pieces.insert()`, which is O(P) because all subsequent elements in the vector must be shifted in memory. Since P (number of pieces) is typically small compared to the total characters, this is highly efficient.
- **Memory Optimization**: Notice the check: `pieces[i].source == BufType::ADD && pieces[i].start + pieces[i].length == add_start`. This checks if the user is typing contiguous characters. If so, it merely increments the `length` of the existing piece rather than creating a new one, avoiding vector fragmentation!

**`erase(size_t index, size_t length)`**
- Complexity: O(P) per deleted segment.
- Logic: Similar to insert, it locates the pieces spanning the deletion range. It handles edge cases: deleting a whole piece, truncating the start, truncating the end, or splitting a piece down the middle.

**`getCharacterFromCursor(size_t index, int offset)`**
- Returns `std::optional<char>`. In modern C++ (C++17), `std::optional` is the idiomatic way to express that a function might not return a value (e.g., if the cursor is out of bounds). It avoids the need for sentinel values (like returning `\0` or `-1`) or throwing exceptions, making the API cleaner and safer.
- **Branch Prediction (`[[unlikely]]`)**: Notice `if (left > index) [[unlikely]]`. This C++20 attribute hints to the compiler's branch predictor that this path is rarely taken, optimizing instruction cache usage.

---

## Part 3: The Editor Buffer Layer

### `include/blip/buffer/buffer.hpp` & `src/buffer/buffer.cpp`

The `EditorBuffer` wraps the `PieceTable` and provides domain-specific features (2D coordinates, vim-like movements, undo/redo).

#### Algorithmic Complexity: Fast Lookups
- **`std::vector<size_t> line_starts`**: This vector tracks the starting byte index of each line.
- When you need to find which line a cursor index belongs to (e.g., in `getCursorPosition2D`), the code uses `std::upper_bound(line_starts.begin(), line_starts.end(), cursor_pos)`.
- **Complexity**: `std::upper_bound` performs a binary search on the sorted vector. Thus, finding the line number from an absolute byte index is **O(log L)**, where L is the number of lines. This is massively superior to iterating through the string to count newlines (O(N)).

#### Move Semantics and Undo/Redo
- The `undo_stack` and `redo_stack` are `std::vector<EditRecord>`. `EditRecord` stores the `PieceTable::State` (which itself contains a copy of the `pieces` vector), the cursor position, and the `line_starts`.
- **`std::move`**: In `undo()` and `redo()`, you see `EditRecord record = std::move(undo_stack.back());`. 
- **Why is this important?** Instead of copying the deeply nested vectors inside `EditRecord`, `std::move` transfers ownership of the underlying memory allocations. This converts what would be an O(N) deep copy involving multiple heap allocations into an O(1) pointer swap. This is a crucial concept in C++11 and later, and a favorite topic in interviews.

---

## Part 4: Cross-Platform Systems & Concurrency

### `include/blip/platform/watcher.hpp`

This header defines `ConfigWatcher`, an asynchronous file watcher. High-performance systems frequently use asynchronous I/O and dedicated worker threads to prevent blocking the main thread.

- **`std::atomic<bool> running` & `std::atomic<bool> fileDirty`**: In a multi-threaded context, accessing shared variables without synchronization leads to data races and undefined behavior. `std::atomic` ensures that reads and writes to these booleans are atomic and properly ordered across CPU caches, preventing torn reads. It provides lock-free synchronization, which is significantly faster than using a `std::mutex`.
- **`std::function<void()>`**: Used for the callback. This provides a type-safe wrapper for function pointers, lambda expressions, or bind expressions.

### Platform-Specific Implementations

**Darwin (`src/platform/darwin/watcher.cpp`)**
- Uses **kqueue/kevent**, the BSD/macOS kernel event notification interface.
- It registers the file descriptor with `EVFILT_VNODE` to watch for `NOTE_WRITE` and `NOTE_RENAME`.
- Polling `kevent` blocks the thread (with a timeout) until the OS notifies it of a change, avoiding wasteful busy-waiting (100% CPU usage loop).

**Linux (`src/platform/linux/watcher.cpp`)**
- Uses **inotify**, the Linux kernel subsystem that acts to extend filesystems to notice changes.
- It registers `inotify_add_watch` with `IN_MODIFY` and `IN_MOVE_SELF`.
- Uses `poll` to wait for the inotify file descriptor to become readable.
- **Alignment (`__attribute__((aligned(...)))`)**: The `buf` is aligned to the requirement of `struct inotify_event`. This is a crucial systems programming detail: unaligned memory access can cause performance penalties or hardware faults on some architectures (though modern x86 handles it, it's still good practice).

**Threading (`worker = std::thread(...)`)**
- The `start` function spawns a `std::thread` executing the `loop` function.
- **RAII & Resource Cleanup**: The destructor calls `stop()`, which sets the atomic `running = false` and calls `worker.join()`. `join()` blocks until the worker thread finishes executing. If you failed to join or detach a thread before the `std::thread` object goes out of scope, the program would call `std::terminate()` and crash. This is a classic concurrency interview question.

### `include/blip/platform/system.hpp` & `src/platform/system.cpp`

**`readFile` Function**
- Takes `const char *fname` and `std::string &lines` as an out-parameter. Passing the string by reference avoids returning a copy by value (though modern Return Value Optimization (RVO) often mitigates this anyway).
- Uses `std::ifstream` (RAII file handling). When `f` goes out of scope at the end of the function, the file is automatically closed, preventing file descriptor leaks.
- Uses `std::ostringstream ss; ss << f.rdbuf(); lines = ss.str();`. This is a common idiom to dump the entire file buffer into a string at once. However, for extremely large files, it involves a double copy (file to stringstream, stringstream to string). A more optimized approach in a latency-sensitive environment would be to preallocate the string using `f.seekg(0, std::ios::end); size = f.tellg(); lines.resize(size);` and read directly into the string buffer, saving allocations.
# Deep Dive: Blip Editor Codebase Analysis (Parts 5, 6, 7)

This document provides a highly detailed, line-by-line analysis of the core components, configuration management, input engines, and syntax highlighting systems within the Blip editor. It is tailored for advanced C++ interview preparation, specifically emphasizing modern C++ features (C++17/20), zero-cost abstractions, memory management, and system-level integrations typically scrutinized in High-Frequency Trading (HFT) and Algorithmic Trading environments.

---

## Part 5: Core Logging System (`core/log.hpp`, `core/log.cpp`)

The logging system demonstrates compile-time polymorphism and type-safe logging without the runtime overhead of `virtual` functions or RTTI (Run-Time Type Information).

### Header Analysis (`include/blip/core/log.hpp`)
```cpp
#pragma once
#include <blip/config/editor.hpp>

namespace core {
void printState(config::EditorConfig &state);
}
```
*   **`#pragma once`**: Non-standard but universally supported include guard. Faster than `#ifndef` guards as the compiler avoids opening the file twice.
*   **Namespaces**: Encapsulates `core` logic to prevent symbol collisions, standard practice in large codebases.

### Source Analysis (`src/core/log.cpp`)
```cpp
template <typename T> void printVal(T value, const char *str) {
    if constexpr (std::is_same_v<T, SDL_Color>) {
        printf("    %s = rgba(%u,%u,%u,%u)\n", str, value.r, value.g, value.b, value.a);
    } else if constexpr (std::is_same_v<Uint8, T> || std::is_same_v<Uint16, T>) {
        printf("    %s = %u\n", str, value);
    } else if constexpr (std::is_same_v<config::Shortcut, T>) {
        printf("    %s = (mods: %u, keys: %d)\n", str, value.modifiers, value.key);
    } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, std::string> || std::is_same_v<T, char *> ||
                         std::is_same_v<T, Uint8> || std::is_same_v<T, int> || std::is_same_v<T, float>) {
        std::cout << "    " << str << " = " << value << std::endl;
    }
}
```
*   **Template Metaprogramming & `if constexpr` (C++17)**: This is highly relevant for HFT. `if constexpr` evaluates branches at *compile time*. The compiler will discard the branches that don't match the type `T`. This prevents compilation errors for unsupported type operations (e.g., trying to access `.r` on an `int`) and results in perfectly optimized machine code with zero branching overhead at runtime.
*   **Type Traits (`std::is_same_v`)**: Yields a compile-time boolean constant.
*   **`printf` vs `std::cout`**: `printf` is significantly faster and doesn't incur the overhead of `<iostream>` locale handling and formatting overheads.

---

## Part 6: Configuration Management (`config/editor.hpp`, `config/editor.cpp`)

Configuration parsing and layout states heavily rely on POD (Plain Old Data) structs, strongly typed enums, and `constexpr` for compile-time constants.

### Strongly Typed Enums (`include/blip/config/editor.hpp`)
```cpp
enum class CursorStyleOpts { CursorBlock, CursorLine, COUNT };
enum class LineNumberOpts { LineAbsolute, LineRelative, LineHidden, LineAbsoluteAndRelative, COUNT };
```
*   **`enum class` (Scoped Enumerations)**: Essential for avoiding namespace pollution. Underlying types are strictly enforced (defaults to `int`), preventing implicit conversions to `int` that could lead to logic bugs (a common pitfall in C-style enums).
*   **The `COUNT` idiom**: A common trick to know the number of variants in the enum, useful for boundary checking later.

### Constant Namespaces
```cpp
namespace positions {
namespace x {
inline constexpr const int LINE_NUMBER = 30;
} }
```
*   **`inline constexpr` (C++17)**: This ensures that the constant is evaluated at compile-time and folded into the instruction stream without allocating memory in the `.rodata` section (unless its address is taken). The `inline` keyword permits multiple inclusion without ODR (One Definition Rule) violations.

### High-Performance Parsing (`src/config/editor.cpp`)
```cpp
template <typename T> bool parseNum(std::string_view sub, T &out, int base = 10) {
    if constexpr (std::is_same_v<int, T> || std::is_same_v<Uint8, T>) {
        auto [ptr, ec] = std::from_chars(sub.data(), sub.data() + sub.size(), out, base);
        return ec == std::errc{};
    }
    return false;
}
```
*   **`std::string_view` (C++17)**: Essential for performance. It acts as a non-owning pointer + length to a string, avoiding heap allocations associated with `std::string::substr`.
*   **`<charconv>` & `std::from_chars` (C++17)**: The fastest, locale-independent string-to-number conversion in the C++ standard library. Unlike `std::stoi`, it does not allocate memory and does not throw exceptions, returning an error code `ec` instead. This is exactly what you want in low-latency systems.
*   **Structured Binding `auto [ptr, ec] = ...`**: Clean syntax for unpacking tuples/structs.

---

## Part 7.1: Vim Engine & State Machine (`input/action.hpp`, `input/vim_engine.hpp`, `src/input/vim_engine.cpp`)

The input handling is implemented as a Finite State Machine (FSM) utilizing the Command Pattern.

### Command Pattern (`include/blip/input/action.hpp`)
```cpp
struct Action {
    ActionType type = ActionType::None;
    std::string payload = "";
    int count = 1;
};
```
*   The `Action` struct represents an intention rather than executing it immediately. This decoupling is the textbook Command Pattern, crucial for implementing Undo/Redo buffers and macro recording.

### State Machine Definition (`include/blip/input/vim_engine.hpp`)
```cpp
enum class VimMode { NORMAL, INSERT, VISUAL, REPLACE, COMMAND };

class VimEngine {
    std::vector<Action> handleKeyDown(const SDL_Event &event);
private:
    VimMode mode;
    std::string command_buffer;
};
```
*   The system models Vim's modal behavior cleanly using the `VimMode` enum. State transitions occur based on keystrokes, heavily isolating the logic per mode.

### Event Dispatch (`src/input/vim_engine.cpp`)
```cpp
std::vector<Action> VimEngine::handleKeyDown(const SDL_Event &event) {
    switch (mode) {
    case VimMode::NORMAL: return handleNormalMode(event);
    case VimMode::INSERT: return handleInsertMode(event);
    // ...
    }
}
```
*   **Return by Value (`std::vector<Action>`)**: Historically, returning a vector by value was a performance sin due to copies. However, modern C++ leverages **NRVO (Named Return Value Optimization)** and move semantics (`std::move`). Returning by value is now the idiomatic and highly efficient way to transfer ownership of dynamic containers.

### Buffer Manipulation
```cpp
if (event.key.keysym.sym == SDLK_g && !isShift) {
    if (command_buffer == "g") {
        actions.push_back({ActionType::MoveStartOfFile});
        command_buffer = "";
    } else {
        command_buffer = "g";
        return actions;
    }
}
```
*   The implementation handles multi-keystroke chords (like `gg` for moving to the top of the file) using a stateful `command_buffer`.

---

## Part 7.2: Text Syntax and C Interop (`text/syntax.hpp`, `text/syntax.cpp`)

This section highlights RAII (Resource Acquisition Is Initialization) and integrating C APIs (Tree-sitter) into modern C++ boundaries safely.

### RAII & Rule of Five (`include/blip/text/syntax.hpp`)
```cpp
class SyntaxEngine {
  public:
    SyntaxEngine(const std::string &languageName, const std::string &libraryPath);
    ~SyntaxEngine();

    SyntaxEngine(const SyntaxEngine &) = delete;
    SyntaxEngine &operator=(const SyntaxEngine &) = delete;

  private:
    TSParser *parser = nullptr;
    TSTree *tree = nullptr;
    void *libraryHandle = nullptr;
};
```
*   **RAII**: The constructor acquires resources (memory for parser/tree, file handles for shared library) and the destructor guarantees their release.
*   **Deleting Copy Semantics**: By `delete`ing the copy constructor and copy assignment operator, the class strictly prevents shallow copying. Since `TSParser*` and `void*` are raw pointers representing unique ownership of C-allocated memory/handles, a shallow copy would lead to double-free bugs upon destruction. (To be fully Rule of 5 compliant, move constructors should ideally be defined or explicitly deleted).

### Dynamic Loading and C-Interop (`src/text/syntax.cpp`)
```cpp
extern "C" const TSLanguage *tree_sitter_cpp();

typedef const TSLanguage *(*LanguageFactory)();

SyntaxEngine::SyntaxEngine(const std::string &languageName, const std::string &libraryPath) {
    parser = ts_parser_new();
    libraryHandle = dlopen(libraryPath.c_str(), RTLD_LAZY);
    // ...
    std::string functionName = "tree_sitter_" + languageName;
    auto getLanguage = (LanguageFactory)dlsym(libraryHandle, functionName.c_str());
    // ...
}
```
*   **`extern "C"`**: This directive turns off C++ name mangling for the specified symbol. C++ compilers mangle function names to support function overloading. However, since the Tree-sitter library is written in C, it exports unmangled symbols. `extern "C"` allows the C++ linker to find them.
*   **POSIX Dynamic Loading (`dlopen`, `dlsym`, `dlclose`)**: The code loads parser libraries dynamically at runtime. This allows the editor to add language support without recompiling.
    *   `RTLD_LAZY`: Resolves symbols only when the code executing references them, minimizing startup latency.
    *   `dlsym`: Looks up the function pointer dynamically via string, requiring careful C-style casting `(LanguageFactory)` to the correct function signature.
*   **Resource Cleanup**:
```cpp
SyntaxEngine::~SyntaxEngine() {
    if (tree) ts_tree_delete(tree);
    if (parser) ts_parser_delete(parser);
    if (libraryHandle) dlclose(libraryHandle);
}
```
*   Properly releasing handles acquired from the Tree-sitter API and the OS loader to prevent memory and file descriptor leaks.
# Blip Codebase Deep Dive: Parts 8, 9, and 10

This document provides an extremely thorough, line-by-line analysis of the text rendering, caching, and UI rendering subsystems in the Blip codebase. It emphasizes core C++ features, performance optimization strategies (like viewport culling and caching), and SDL memory management.

---

## 1. Text Layout and Typesetting (`typesetter.hpp` & `typesetter.cpp`)

The Typesetter's job is to translate logical buffer data (lines of text) into spatial coordinates on the screen.

### `typesetter.hpp`
- **Lines 9-12**: `VisualLine` struct. Encapsulates a line of text and its corresponding `y_pixel_offset`. This separates logical text from its physical representation.
- **Lines 14-21**: `Typesetter` class. Provides layout mechanisms.
  - `layout()`: Computes the visual layout for the entire buffer.
  - `layoutRange()`: Optimizes layout by only processing a slice of lines (Start to End).
  - `getCursorPixelPos()`: Translates the 2D cursor (row, col) into `(x, y)` pixel coordinates.

### `typesetter.cpp`
- **`layout()` (Lines 7-29)**: Uses `std::istringstream` to slice the raw string by newline, calculating `yPos = currentLineIndex * lineHeight`. This is a naive full-buffer parse, which is $O(N)$ where N is the buffer length.
- **`getCursorPixelPos()` (Lines 31-50)**: Gets text *before* the cursor and calculates its rendered pixel width using `TTF_SizeUTF8`. This is expensive to do repeatedly as it invokes SDL_ttf on a growing string, but it gives accurate cursor placement for variable-width fonts.
- **`layoutRange()` (Lines 52-64)**: Implements **viewport culling logic** at the data level. Instead of iterating the whole buffer, it iterates from `startLine` to `endLine`.
  - **Const Correctness**: The parameters `const buffer::EditorBuffer &buffer` and `const config::EditorConfig &config` ensure the state isn't modified during the read-only layout process.

---

## 2. Font Management (`font_manager.hpp` & `font_manager.cpp`)

The `FontManager` abstracts away `SDL_ttf` font loading and styling.

### `font_manager.hpp`
- **Line 6**: `enum class FontStyles { Regular = 0, Bold, Italic, BoldItalic, Count };` Strongly typed enum for array indexing.
- **Line 18**: `TTF_Font *fonts[static_cast<int>(FontStyles::Count)];` uses a fixed-size C-style array instead of a `std::vector` or `std::unordered_map` for deterministic O(1) font lookups.

### `font_manager.cpp`
- **Memory Management (Lines 12-19)**: The destructor gracefully frees SDL resources using `TTF_CloseFont`. This prevents memory leaks. RAII principles are strictly adhered to here.
- **`loadStyle()` (Lines 21-74)**: Responsible for finding the font file path and loading it.
  - Falls back to synthesizing styles (e.g., italics) using `TTF_SetFontStyle` if the specific bold/italic TTF file isn't found.
- **`updateFontFamily()` (Lines 76-102)**: Implements **Cache Invalidation**. If the font family or size changes, it clears the old `fonts` array (calling `TTF_CloseFont`), and reloads them. An early-exit guard (Lines 77-79) prevents unnecessary reallocations.

---

## 3. Glyph Caching (`glyph_cache.hpp` & `glyph_cache.cpp`)

To avoid calling the computationally expensive `TTF_RenderUTF8_Blended` every frame for every character, Blip caches glyphs as SDL Textures.

### `glyph_cache.hpp`
- **Line 8-12**: `Glyph` struct containing the `SDL_Texture*` and width/height metadata.
- **Line 27**: `Glyph ascii_cache[128];`
  - **`std::unordered_map` vs Array**: The developer explicitly chose a flat array of 128 elements over a `std::unordered_map<char, Glyph>`. For ASCII (values 0-127), an array provides guaranteed $O(1)$ constant-time lookup without the hashing overhead, dynamic memory allocations, or potential hash collisions of a map.

### `glyph_cache.cpp`
- **Constructor (Lines 5-30)**: Pre-renders printable ASCII characters (32 to 127).
  - Uses `TTF_RenderUTF8_Blended` to create an `SDL_Surface` (CPU RAM).
  - Converts it to an `SDL_Texture` (GPU VRAM) using `SDL_CreateTextureFromSurface`.
  - Instantly frees the CPU surface using `SDL_FreeSurface(surf)` to avoid memory bloat. This is textbook SDL Memory Management.
- **Destructor (Lines 32-41)**: Iterates over the cache and destroys the `SDL_Texture`s to free GPU memory.
- **`getGlyph()` (Lines 43-48)**: Safe array access. If `c` is out of bounds, it returns a `fallback_glyph` (usually a `?`).
- **`measureString()` (Lines 50-60)**: Fast string measurement by summing pre-calculated glyph widths, bypassing `SDL_ttf` entirely.

---

## 4. UI Rendering & Optimization (`renderer.hpp` & `renderer.cpp`)

The renderer takes the typeset lines and draws them onto the screen.

### Viewport Culling Math
- **`drawEditor()` (Lines 130-132)**:
  ```cpp
  size_t startLine = std::max((size_t)0, (size_t)(appState.scroll_y / lineHeight));
  size_t visibleCount = (viewport.h / lineHeight) + 2;
  size_t endLine = std::min(totalLines, startLine + visibleCount);
  ```
  Instead of iterating over every line in the file (which would kill performance for large files), it calculates the exact start and end line indices based on the vertical scroll offset (`scroll_y`).

### Draw Loop Optimization & GPU Batching
- **`drawLine()` (Lines 84-103)**: Iterates through characters in a line, fetching from the `glyphCache`, modulating the texture color (`SDL_SetTextureColorMod`), and calling `SDL_RenderCopy`.
- **Texture Atlasing Considerations**: Blip currently stores one `SDL_Texture` per character. In modern game engines, making hundreds of `SDL_RenderCopy` calls per frame is considered inefficient (too many draw calls). An optimized approach would use **Texture Atlasing**—packing all glyphs into a single large texture—and issuing a single batched draw call with UV coordinates.
- **Color Modulation**: `SDL_SetTextureColorMod` dynamically changes the color of the cached white glyph texture on the GPU. This is an incredible optimization because you don't need a separate cached texture for every possible syntax highlighting color.

### 2D Arrays / Vectors
- The core layout is processed into `std::vector<VisualLine>`. Vectors provide contiguous memory allocation, which is extremely cache-friendly for the CPU when iterating linearly in the `for (const auto &line : lines)` loop in `drawEditor`.

### Const Correctness
Throughout the codebase, read-only references are marked `const` (e.g., `const GlyphCache &glyphCache`, `const std::string &commandText`). This not only prevents accidental mutations but allows the C++ compiler to aggressively optimize memory reads.
