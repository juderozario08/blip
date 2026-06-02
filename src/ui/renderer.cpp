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
    auto lines = typesetter.layout(buffer, config);
    auto lineCount = 1;

    auto [cursorX, cursorY] = typesetter.getCursorPixelPos(buffer, config, fonts);
    int lineHeight = config.font.size + 2;

    if (cursorY + lineHeight > appState.scroll_y + appState.window_height - 20) {
        appState.scroll_y = cursorY + lineHeight - appState.window_height + 20;
    } else if (cursorY < appState.scroll_y) {
        appState.scroll_y = cursorY;
    }

    for (const auto &line : lines) {
        int draw_y = 10 + line.y_pixel_offset - appState.scroll_y;

        if (draw_y + lineHeight < 0) {
            lineCount++;
            continue;
        }
        if (draw_y > appState.window_height) {
            break;
        }

        SDL_Surface *lineNumSurface = TTF_RenderUTF8_Blended(font, std::to_string(lineCount).c_str(), textColor);
        if (lineNumSurface) {
            auto *lineNumTexture = SDL_CreateTextureFromSurface(appState.renderer, lineNumSurface);
            if (lineNumTexture) {
                SDL_Rect lineNumRect = {5, draw_y, lineNumSurface->w, lineNumSurface->h};
                SDL_RenderCopy(appState.renderer, lineNumTexture, nullptr, &lineNumRect);
            }
            SDL_DestroyTexture(lineNumTexture);
        }

        lineCount++;

        if (line.text.empty()) {
            SDL_FreeSurface(lineNumSurface);
            continue;
        }

        SDL_Surface *lineSurface = TTF_RenderUTF8_Blended(font, line.text.c_str(), textColor);
        if (lineSurface && lineNumSurface) {
            auto *lineTexture = SDL_CreateTextureFromSurface(appState.renderer, lineSurface);
            if (lineTexture) {
                SDL_Rect lineRect = {30, draw_y, lineSurface->w, lineSurface->h};
                SDL_RenderCopy(appState.renderer, lineTexture, nullptr, &lineRect);
            }
            SDL_DestroyTexture(lineTexture);
        }
        SDL_FreeSurface(lineSurface);
        SDL_FreeSurface(lineNumSurface);
    }

    int cursorWidth;
    int halfWidth;

    switch (config.ui.cursor_style) {
    case config::CursorStyleOpts::CursorBlock:
        halfWidth = config.font.size / 2;
        cursorWidth = (halfWidth % 2 == 0) ? halfWidth + 3 : halfWidth + 2;
        break;
    default:
        cursorWidth = 2;
        break;
    }

    SDL_Rect cursorRect = {30 + cursorX - 1, 10 + cursorY - appState.scroll_y - 1, cursorWidth, lineHeight};
    auto cursorColor = config.theme.cursor;

    SDL_SetRenderDrawColor(appState.renderer, cursorColor.r, cursorColor.g, cursorColor.b, 100);
    SDL_SetRenderDrawBlendMode(appState.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(appState.renderer, &cursorRect);
}
}
