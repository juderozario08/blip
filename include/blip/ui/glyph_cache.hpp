#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>

namespace ui {

struct Glyph {
    SDL_Texture *texture = nullptr;
    int width = 0;
    int height = 0;
};

class GlyphCache {
  public:
    GlyphCache(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color);
    ~GlyphCache();

    GlyphCache(const GlyphCache &) = delete;
    GlyphCache &operator=(const GlyphCache &) = delete;

    const Glyph &getGlyph(char c) const;

    int measureString(const std::string &text, int tab_width) const;

  private:
    Glyph ascii_cache[128];
    Glyph fallback_glyph;
};

}
