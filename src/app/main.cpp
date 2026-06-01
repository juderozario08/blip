#include <SDL.h>
#include <SDL_ttf.h>
#include <blip/app/main.hpp>
#include <blip/buffer/buffer.hpp>
#include <blip/buffer/table.hpp>
#include <blip/config/editor.hpp>
#include <blip/core/log.hpp>
#include <blip/input/vim_engine.hpp>
#include <blip/platform/system.hpp>
#include <blip/platform/watcher.hpp>
#include <blip/text/font_manager.hpp>
#include <blip/text/typesetter.hpp>
#include <blip/ui/renderer.hpp>
#include <cstdio>
#include <iostream>
#include <string>

#ifdef _DEV_
#define DEV(...) __VA_ARGS__
#else
#define DEV(...)
#endif

enum class VimMode { NORMAL, INSERT, VISUAL, REPLACE };

typedef struct {
    VimMode mode;
    std::string macro_buffer;
    std::string command_buffer;
    std::string keystroke_buffer;
} Vim;

void eventLoop(app::AppState &appState, platform::ConfigWatcher &watcher, config::EditorConfig &config,
               buffer::EditorBuffer &buffer) {
    auto running = true;
    SDL_Event event;

    text::FontManager fonts;
    fonts.updateFontFamily(config.font.family, config.font.size);

    bool dirty = true;

    input::VimEngine vimEngine;

    text::Typesetter typesetter;

    while (running) {
        SDL_WaitEventTimeout(NULL, 250);
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_QUIT) {
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED || event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    SDL_GetWindowSize(appState.window, &appState.window_width, &appState.window_height);
                    dirty = true;
                }
            } else if (event.type == SDL_TEXTINPUT) {
                if (vimEngine.handleTextInput(event.text.text, buffer)) {
                    dirty = true;
                }
            } else if (event.type == SDL_KEYDOWN) {
                if (config.input.vim_mode) {
                    if (vimEngine.handleKeyDown(event, buffer)) {
                        dirty = true;
                    }
                } else {
                    // FIX: ADD STANDARD KEYBINDS HERE
                }
            }
        }

        if (dirty) {
            dirty = false;
            std::cout << "| Mode: " << (int)vimEngine.getMode() << " | Cursor: " << buffer.getCursor()
                      << "| Lines: " << buffer.getNumberOfLines() << "|\n";

            ui::drawBackground(appState, config);
            ui::drawEditor(appState, buffer, config, fonts, typesetter);
            SDL_RenderPresent(appState.renderer);
        }

        watcher.check();
        if (fonts.updateFontFamily(config.font.family, config.font.size)) {
            dirty = true;
        }
    }
}

// TODO: MIGHT WANT TO DISPLAY ERRORS USING A NEW WINDOW SO THE USER STAYS INFORMED
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

    SDL_RaiseWindow(appState.window); // Focus Window

    SDL_GetWindowSize(appState.window, &appState.window_width, &appState.window_height);
    appState.renderer = SDL_CreateRenderer(appState.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (appState.renderer == NULL) {
        std::cerr << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    config::EditorConfig config;
    config::setDefaultConifg(config);
    platform::ConfigWatcher watcher;

    // TODO: Calculate filepath using system file lookup
    std::string filepath = "config.ini";
    config::loadConfig(filepath, config);
    DEV(core::printState(config));
    watcher.start(filepath, [&config, filepath]() {
        config::loadConfig(filepath, config);
        DEV(core::printState(config));
    });

    std::string original_content = "";
    if (argc == 2) {
        platform::readFile(argv[1], original_content);
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
