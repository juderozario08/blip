#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <blip/app/main.hpp>
#include <blip/buffer/buffer.hpp>
#include <blip/config/editor.hpp>
#include <blip/input/vim_engine.hpp>
#include <blip/text/font_manager.hpp>
#include <blip/text/syntax.hpp>
#include <blip/text/typesetter.hpp>
#include <blip/ui/glyph_cache.hpp>

namespace ui {
void drawEditor(app::AppState &appState, buffer::EditorBuffer &buffer, config::EditorConfig &config, text::FontManager &fonts,
                text::Typesetter &typesetter, SDL_Rect viewport, const GlyphCache &glyphCache, text::SyntaxEngine &syntaxEngine);
void drawBackground(app::AppState &appState, config::EditorConfig &state);
void drawStatusBar(app::AppState &appState, config::EditorConfig &config, text::FontManager &fonts, input::VimMode mode,
                   const std::string &commandText);
}
