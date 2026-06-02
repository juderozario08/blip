#include <blip/ui/glyph_cache.hpp>

namespace ui {

GlyphCache::GlyphCache(SDL_Renderer *renderer, TTF_Font *font, SDL_Color color) {
    if (!font || !renderer) {
        return;
    }

    for (int i = 32; i < 127; i++) {
        char c = static_cast<char>(i);
        std::string s(1, c);

        SDL_Surface *surf = TTF_RenderUTF8_Blended(font, s.c_str(), color);
        if (surf) {
            ascii_cache[i].texture = SDL_CreateTextureFromSurface(renderer, surf);
            ascii_cache[i].width = surf->w;
            ascii_cache[i].height = surf->h;
            SDL_FreeSurface(surf);
        }
    }

    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, "?", color);
    if (surf) {
        fallback_glyph.texture = SDL_CreateTextureFromSurface(renderer, surf);
        fallback_glyph.width = surf->w;
        fallback_glyph.height = surf->h;
        SDL_FreeSurface(surf);
    }
}

GlyphCache::~GlyphCache() {
    for (int i = 0; i < 128; i++) {
        if (ascii_cache[i].texture) {
            SDL_DestroyTexture(ascii_cache[i].texture);
        }
    }
    if (fallback_glyph.texture) {
        SDL_DestroyTexture(fallback_glyph.texture);
    }
}

const Glyph &GlyphCache::getGlyph(char c) const {
    if (c >= 32 && c < 127) {
        return ascii_cache[static_cast<int>(c)];
    }
    return fallback_glyph;
}

int GlyphCache::measureString(const std::string &text, int tab_width) const {
    int width = 0;
    for (char c : text) {
        if (c == '\t') {
            width += getGlyph(' ').width * tab_width;
        } else {
            width += getGlyph(c).width;
        }
    }
    return width;
}
}
