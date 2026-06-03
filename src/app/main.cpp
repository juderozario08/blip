#include <SDL.h>
#include <SDL_ttf.h>
#include <blip/app/main.hpp>
#include <blip/buffer/buffer.hpp>
#include <blip/buffer/table.hpp>
#include <blip/config/editor.hpp>
#include <blip/core/log.hpp>
#include <blip/input/action.hpp>
#include <blip/input/vim_engine.hpp>
#include <blip/platform/system.hpp>
#include <blip/platform/watcher.hpp>
#include <blip/text/font_manager.hpp>
#include <blip/text/syntax.hpp> // ADDED: Syntax Engine Header
#include <blip/text/typesetter.hpp>
#include <blip/ui/glyph_cache.hpp>
#include <blip/ui/renderer.hpp>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>

#ifdef _DEV_
#define DEV(...) __VA_ARGS__
#else
#define DEV(...)
#endif

void dispatchActions(const std::vector<input::Action> &actions, buffer::EditorBuffer &buffer, bool &running,
                     app::AppState &appState) {
    for (const auto &action : actions) {
        for (int i = 0; i < action.count; i++) {
            switch (action.type) {
            case input::ActionType::MoveLeft:
                buffer.moveLeft();
                break;
            case input::ActionType::MoveRight:
                buffer.moveRight();
                break;
            case input::ActionType::MoveUp:
                buffer.moveUp();
                break;
            case input::ActionType::MoveDown:
                buffer.moveDown();
                break;
            case input::ActionType::MoveWordForward:
                buffer.cursorForward(action.payload);
                break;
            case input::ActionType::MoveWordBack:
                buffer.cursorBack(action.payload);
                break;
            case input::ActionType::MoveStartOfLine:
                buffer.moveToStartOfLine();
                break;
            case input::ActionType::MoveEndOfLine:
                buffer.moveToEndOfLine();
                break;
            case input::ActionType::MoveStartOfFile:
                buffer.moveToStartOfFile();
                break;
            case input::ActionType::MoveEndOfFile:
                buffer.moveToEndOfFile();
                break;
            case input::ActionType::InsertText:
                buffer.insertText(action.payload);
                break;
            case input::ActionType::DeleteChar:
                buffer.deleteChar();
                break;
            case input::ActionType::Backspace:
                buffer.backspace(1);
                break;
            case input::ActionType::NewLineNext:
                buffer.insertNewLineNext();
                break;
            case input::ActionType::NewLinePrev:
                buffer.insertNewLinePrev();
                break;
            case input::ActionType::Undo:
                buffer.undo();
                break;
            case input::ActionType::Redo:
                buffer.redo();
                break;
            case input::ActionType::CommitUndo:
                buffer.commit();
                break;
            case input::ActionType::InsertBlankLineAbove:
                buffer.insertBlankLineAboveStay();
                break;
            case input::ActionType::InsertBlankLineBelow:
                buffer.insertBlankLineBelowStay();
                break;
            case input::ActionType::ClampNormal:
                buffer.clampVimNormal();
                break;
            case input::ActionType::None:
                break;
            case input::ActionType::StartVisual:
                buffer.setVisualAnchor();
                break;
            case input::ActionType::ClearVisual:
                buffer.clearVisualAnchor();
                break;
            case input::ActionType::Yank: {
                std::string selected = buffer.getSelectedText();
                if (!selected.empty()) {
                    SDL_SetClipboardText(selected.c_str());
                }
                break;
            }

            case input::ActionType::Paste: {
                if (SDL_HasClipboardText()) {
                    char *clipboardText = SDL_GetClipboardText();
                    if (clipboardText) {
                        buffer.insertText(std::string(clipboardText));
                        SDL_free(clipboardText);
                    }
                }
                break;
            }
            case input::ActionType::Quit:
                running = false;
                break;
            case input::ActionType::SaveFile: {
                std::string target = action.payload.empty() ? appState.filepath : action.payload;
                if (!target.empty()) {
                    buffer.saveToFile(target);
                    appState.filepath = target;
                }
                break;
            }
            }
        }
    }
}

void eventLoop(app::AppState &appState, platform::ConfigWatcher &watcher, config::EditorConfig &config,
               buffer::EditorBuffer &buffer) {
    auto running = true;
    SDL_Event event;

    text::FontManager fonts;
    fonts.updateFontFamily(config.font.family, config.font.size);

    SDL_Color pureWhite = {255, 255, 255, 255};
    auto glyphCache = std::make_unique<ui::GlyphCache>(appState.renderer, fonts.getFont(), pureWhite);

    text::SyntaxEngine syntaxEngine;

    bool dirty = true;
    input::VimEngine vimEngine;
    text::Typesetter typesetter;

    while (running) {
        SDL_WaitEventTimeout(NULL, 250);
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    SDL_GetWindowSize(appState.window, &appState.window_width, &appState.window_height);
                    dirty = true;
                }
            } else if (event.type == SDL_TEXTINPUT || event.type == SDL_KEYDOWN) {

                input::VimMode prevMode = vimEngine.getMode();
                std::string prevCmd = vimEngine.getActiveCommand();
                bool action_taken = false;

                if (event.type == SDL_TEXTINPUT) {
                    auto actions = vimEngine.handleTextInput(event.text.text);
                    if (!actions.empty()) {
                        dispatchActions(actions, buffer, running, appState);
                        action_taken = true;
                    }
                } else if (event.type == SDL_KEYDOWN && config.input.vim_mode) {
                    auto actions = vimEngine.handleKeyDown(event);
                    if (!actions.empty()) {
                        dispatchActions(actions, buffer, running, appState);
                        action_taken = true;
                    }
                }

                if (action_taken || vimEngine.getMode() != prevMode || vimEngine.getActiveCommand() != prevCmd) {
                    dirty = true;
                }
            }
        }

        if (dirty) {
            dirty = false;

            std::cout << "| Mode: ";
            switch (vimEngine.getMode()) {
            case input::VimMode::NORMAL:
                std::cout << "NORMAL ";
                break;
            case input::VimMode::INSERT:
                std::cout << "INSERT ";
                break;
            case input::VimMode::VISUAL:
                std::cout << "VISUAL ";
                break;
            case input::VimMode::COMMAND:
                std::cout << "COMMAND";
                break;
            case input::VimMode::REPLACE:
                std::cout << "REPLACE";
                break;
            }
            std::cout << " | Cursor: " << buffer.getCursor() << " | Lines: " << buffer.getNumberOfLines() << " |\n";

            // ADDED: Re-parse the AST before we draw
            syntaxEngine.parse(buffer.getText());

            ui::drawBackground(appState, config);

            SDL_Rect editorViewport = {0, config.layout.top_padding, appState.window_width,
                                       appState.window_height - config.layout.status_bar_height - config.layout.top_padding};

            // CHANGED: Pass the syntax engine down into drawEditor
            ui::drawEditor(appState, buffer, config, fonts, typesetter, editorViewport, *glyphCache, syntaxEngine);
            ui::drawStatusBar(appState, config, fonts, vimEngine.getMode(), vimEngine.getActiveCommand());
            SDL_RenderPresent(appState.renderer);
        }

        watcher.check();

        if (fonts.updateFontFamily(config.font.family, config.font.size)) {
            // CHANGED: Ensure pureWhite is applied when font changes
            glyphCache = std::make_unique<ui::GlyphCache>(appState.renderer, fonts.getFont(), pureWhite);
            dirty = true;
        }
    }
}

int main(int argc, char *argv[]) {
    app::AppState appState;
    if (SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL Init Error " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() < 0) {
        std::cerr << "TTF Init Error " << TTF_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    appState.window = SDL_CreateWindow("Blip", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600,
                                       SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);
    if (appState.window == NULL) {
        std::cerr << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    SDL_RaiseWindow(appState.window);

    SDL_GetWindowSize(appState.window, &appState.window_width, &appState.window_height);
    appState.renderer = SDL_CreateRenderer(appState.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (appState.renderer == NULL) {
        std::cerr << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    config::EditorConfig config;
    config::setDefaultConfig(config);
    platform::ConfigWatcher watcher;

    std::string filepath = "config.ini";
    config::loadConfig(filepath, config);
    DEV(core::printState(config));
    watcher.start(filepath, [&config, filepath]() {
        config::loadConfig(filepath, config);
        DEV(core::printState(config));
    });

    std::string original_content = "";
    if (argc == 2) {
        appState.filepath = argv[1];
        platform::readFile(argv[1], original_content);
    } else {
        appState.filepath = "untitled.txt";
    }

    if (!original_content.empty() && original_content.back() == '\n') {
        original_content.pop_back();
    }

    buffer::EditorBuffer buffer(original_content);

    SDL_StartTextInput();
    eventLoop(appState, watcher, config, buffer);
    SDL_StopTextInput();

    SDL_DestroyRenderer(appState.renderer);
    SDL_DestroyWindow(appState.window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}
