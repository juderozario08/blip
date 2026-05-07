#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <blip/app/main.hpp>
#include <blip/buffer/buffer.hpp>
#include <blip/config/editor.hpp>
#include <blip/text/font_manager.hpp>

namespace ui {
void drawEditor(app::AppState &appState, config::EditorConfig &state, text::FontManager &fonts);
void drawBackground(app::AppState &appState, config::EditorConfig &state);
}
