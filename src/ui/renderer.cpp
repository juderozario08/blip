#include <blip/ui/renderer.hpp>

namespace ui {
void drawBackground(app::AppState &appState, config::EditorConfig &state) {
    auto c = state.theme.background;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(appState.renderer);
}

void drawEditor(app::AppState &appState, buffer::EditorBuffer &buffer, config::EditorConfig &config, text::FontManager &fonts,
                text::Typesetter &typesetter) {
    TTF_Font *font = fonts.getFont();
    if (!font) {
        return;
    }

    auto textColor = config.font.color;
    auto cursorColor = {255, 255, 255, 50};
    auto lines = typesetter.layout(buffer, config);

    for (const auto &line : lines) {
        if (line.text.empty()) {
            continue;
        }

        SDL_Surface *lineSurface = TTF_RenderUTF8_Blended(font, line.text.c_str(), textColor);
        if (lineSurface) {
            auto *lineTexture = SDL_CreateTextureFromSurface(appState.renderer, lineSurface);
            if (lineTexture) {
                SDL_Rect destRect = {10, 10 + line.y_pixel_offset, lineSurface->w, lineSurface->h};
                SDL_RenderCopy(appState.renderer, lineTexture, nullptr, &destRect);
            }
            SDL_DestroyTexture(lineTexture);
        }
        SDL_FreeSurface(lineSurface);
    }

    auto [cursorX, cursorY] = typesetter.getCursorPixelPos(buffer, config, fonts);
    int lineHeight = config.font.size;
    int cursorWidth = config.font.size / 2;

    SDL_Rect cursorRect = {10 + cursorX, 10 + cursorY, cursorWidth, lineHeight};
    SDL_SetRenderDrawColor(appState.renderer, 255, 255, 255, 100);
    SDL_SetRenderDrawBlendMode(appState.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(appState.renderer, &cursorRect);
}
}
