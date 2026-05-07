#include <blip/text/font_manager.hpp>
#include <blip/ui/renderer.hpp>
#include <iostream>

namespace ui {
void drawBackground(app::AppState &appState, config::EditorConfig &state) {
    auto c = state.theme.background;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(appState.renderer);
}

void drawEditor(app::AppState &appState, config::EditorConfig &state, text::FontManager &fonts) {
    TTF_Font *font = fonts.getFont();
    if (font == NULL)
        return;

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface *textSurface = TTF_RenderText_Blended(font, "Hello from the new UI module!", textColor);

    if (textSurface == NULL) {
        std::cerr << "Unable to render text surface! Error: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(appState.renderer, textSurface);
    if (textTexture == NULL) {
        std::cerr << "Unable to create texture! Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(textSurface);
        return;
    }

    SDL_Rect renderQuad = {10, 10, textSurface->w, textSurface->h};
    SDL_RenderCopy(appState.renderer, textTexture, NULL, &renderQuad);

    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}
}
