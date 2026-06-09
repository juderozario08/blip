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
#include <blip/text/syntax.hpp>
#include <blip/text/typesetter.hpp>
#include <blip/ui/glyph_cache.hpp>
#include <blip/ui/renderer.hpp>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>

#ifdef _DEV_
#define DEV(...) __VA_ARGS__
#else
#define DEV(...)
#endif

std::string getFileExtension(const std::string &filename) {
    auto index = filename.rfind(".");
    return filename.substr(index + 1);
}

std::string getOrInstallLanguage(const std::string &language) {
    std::string pluginDir = "plugins/parsers";

    std::string extension = ".so";
#ifdef __APPLE__
    extension = ".dylib";
#endif

    std::string pluginFile = pluginDir + "/tree_sitter_" + language + extension;

    if (std::filesystem::exists(pluginFile)) {
        return pluginFile;
    }

    std::cout << "Language plugin '" << language << "' missing. Downloading..." << std::endl;

    std::filesystem::create_directories(pluginDir);

    std::string tmpTar = "/tmp/ts_" + language + ".tar.gz";
    std::string tmpDir = "/tmp/ts_" + language;
    std::filesystem::create_directories(tmpDir);

    std::string repoUrl = "https://github.com/tree-sitter/tree-sitter-" + language;
    std::string tarUrl = repoUrl + "/archive/refs/heads/master.tar.gz";
    std::string curlCmd = "curl -sL " + tarUrl + " -o " + tmpTar;
    system(curlCmd.c_str());

    std::string tarCmd = "tar -xzf " + tmpTar + " -C " + tmpDir + " --strip-components=1";
    system(tarCmd.c_str());

    std::cout << "Compiling " << language << " plugin..." << std::endl;

    std::string parserCmd = "cc -c -fPIC -O3 " + tmpDir + "/src/parser.c -I " + tmpDir + "/src -o " + tmpDir + "/parser.o";
    system(parserCmd.c_str());

    std::string linkCmd = "c++ -shared -fPIC -O3 -o " + pluginFile + " " + tmpDir + "/parser.o";

    if (std::filesystem::exists(tmpDir + "/src/scanner.cc")) {
        std::string scannerCmd =
            "c++ -c -fPIC -O3 " + tmpDir + "/src/scanner.cc -I " + tmpDir + "/src -o " + tmpDir + "/scanner.o";
        system(scannerCmd.c_str());
        linkCmd += " " + tmpDir + "/scanner.o";
    } else if (std::filesystem::exists(tmpDir + "/src/scanner.c")) {
        std::string scannerCmd = "cc -c -fPIC -O3 " + tmpDir + "/src/scanner.c -I " + tmpDir + "/src -o " + tmpDir + "/scanner.o";
        system(scannerCmd.c_str());
        linkCmd += " " + tmpDir + "/scanner.o";
    }

    system(linkCmd.c_str());

    std::filesystem::remove_all(tmpDir);
    std::filesystem::remove(tmpTar);

    std::cout << "Successfully installed " << language << " plugin!" << std::endl;
    return pluginFile;
}

bool dispatchActions(const std::vector<input::Action> &actions, buffer::EditorBuffer &buffer, config::EditorConfig &config,
                     bool &running, app::AppState &appState) {
    bool buffer_modified = false;

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
                buffer_modified = true;
                break;
            case input::ActionType::DeleteChar:
                buffer.deleteChar();
                buffer_modified = true;
                break;
            case input::ActionType::Backspace:
                buffer.backspace(1);
                buffer_modified = true;
                break;
            case input::ActionType::NewLineNext:
                buffer.insertNewLineNext();
                buffer_modified = true;
                break;
            case input::ActionType::NewLinePrev:
                buffer.insertNewLinePrev();
                buffer_modified = true;
                break;
            case input::ActionType::EnableSyntax:
                config.preference.syntax_highlighting = true;
                break;
            case input::ActionType::DisableSyntax:
                config.preference.syntax_highlighting = false;
                break;
            case input::ActionType::ToggleSyntax:
                config.preference.syntax_highlighting = !config.preference.syntax_highlighting;
                break;
            case input::ActionType::Undo:
                buffer.undo();
                buffer_modified = true;
                break;
            case input::ActionType::Redo:
                buffer.redo();
                buffer_modified = true;
                break;
            case input::ActionType::CommitUndo:
                buffer.commit();
                break;
            case input::ActionType::InsertBlankLineAbove:
                buffer.insertBlankLineAboveStay();
                buffer_modified = true;
                break;
            case input::ActionType::InsertBlankLineBelow:
                buffer.insertBlankLineBelowStay();
                buffer_modified = true;
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
                        buffer_modified = true;
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
    return buffer_modified;
}

void eventLoop(app::AppState &appState, platform::ConfigWatcher &watcher, config::EditorConfig &config,
               buffer::EditorBuffer &buffer) {
    auto running = true;
    SDL_Event event;

    text::FontManager fonts;
    fonts.updateFontFamily(config.font.family, config.font.size);

    SDL_Color pureWhite = {255, 255, 255, 255};
    auto glyphCache = std::make_unique<ui::GlyphCache>(appState.renderer, fonts.getFont(), pureWhite);

    const std::string targetLang = getFileExtension(appState.filepath);
    std::string pluginPath = getOrInstallLanguage(targetLang);
    text::SyntaxEngine syntaxEngine(targetLang, pluginPath);

    bool dirty = true;
    bool text_changed = true;

    Uint32 lastEditTime = 0;

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
                        if (dispatchActions(actions, buffer, config, running, appState)) {
                            text_changed = true;
                            lastEditTime = SDL_GetTicks();
                        }
                        action_taken = true;
                    }
                } else if (event.type == SDL_KEYDOWN && config.input.vim_mode) {
                    auto actions = vimEngine.handleKeyDown(event);
                    if (!actions.empty()) {
                        if (dispatchActions(actions, buffer, config, running, appState)) {
                            text_changed = true;
                            lastEditTime = SDL_GetTicks();
                        }
                        action_taken = true;
                    }
                }

                if (action_taken || vimEngine.getMode() != prevMode || vimEngine.getActiveCommand() != prevCmd) {
                    dirty = true;
                }
            }
        }

        if (text_changed && config.preference.syntax_highlighting && (SDL_GetTicks() - lastEditTime > 500)) {
            syntaxEngine.parse(buffer.getText());
            text_changed = false;
            dirty = true;
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

            ui::drawBackground(appState, config);

            SDL_Rect editorViewport = {0, config.layout.top_padding, appState.window_width,
                                       appState.window_height - config.layout.status_bar_height - config.layout.top_padding};

            ui::drawEditor(appState, buffer, config, fonts, typesetter, editorViewport, *glyphCache, syntaxEngine);
            ui::drawStatusBar(appState, config, fonts, vimEngine.getMode(), vimEngine.getActiveCommand());
            SDL_RenderPresent(appState.renderer);
        }

        watcher.check();

        if (fonts.updateFontFamily(config.font.family, config.font.size)) {
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
        appState.filepath = "untitled.cpp";
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
