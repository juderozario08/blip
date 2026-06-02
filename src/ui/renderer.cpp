#include <blip/ui/renderer.hpp>

namespace ui {

void drawBackground(app::AppState &appState, config::EditorConfig &state) {
    auto c = state.theme.background;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(appState.renderer);
}

static void updateCamera(app::AppState &appState, int cursorY, int lineHeight, const SDL_Rect &viewport) {
    int paddingBottom = 11;
    if (cursorY + lineHeight > appState.scroll_y + viewport.h - paddingBottom) {
        appState.scroll_y = cursorY + lineHeight - viewport.h + paddingBottom;
    } else if (cursorY < appState.scroll_y) {
        appState.scroll_y = cursorY;
    }
}

static void drawCursor(app::AppState &appState, config::EditorConfig &config, int cursorX, int cursorY, int lineHeight,
                       const SDL_Rect &viewport) {
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

    SDL_Rect cursorRect = {viewport.x + 30 + cursorX - 1, viewport.y + cursorY - appState.scroll_y - 1, cursorWidth, lineHeight};
    auto cursorColor = config.theme.cursor;

    SDL_SetRenderDrawColor(appState.renderer, cursorColor.r, cursorColor.g, cursorColor.b, 100);
    SDL_SetRenderDrawBlendMode(appState.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(appState.renderer, &cursorRect);
}

static void drawLine(app::AppState &appState, TTF_Font *font, const text::VisualLine &line, int lineCount, int draw_y,
                     const SDL_Color &textColor, const SDL_Rect &viewport) {
    SDL_Surface *lineNumSurface = TTF_RenderUTF8_Blended(font, std::to_string(lineCount).c_str(), textColor);
    if (!lineNumSurface)
        return;

    SDL_Texture *lineNumTexture = SDL_CreateTextureFromSurface(appState.renderer, lineNumSurface);
    if (lineNumTexture) {
        // Offset X by viewport.x
        SDL_Rect lineNumRect = {viewport.x + 10, draw_y, lineNumSurface->w, lineNumSurface->h};
        SDL_RenderCopy(appState.renderer, lineNumTexture, nullptr, &lineNumRect);
        SDL_DestroyTexture(lineNumTexture);
    }
    SDL_FreeSurface(lineNumSurface);

    if (line.text.empty())
        return;

    SDL_Surface *lineSurface = TTF_RenderUTF8_Blended(font, line.text.c_str(), textColor);
    if (!lineSurface)
        return;

    SDL_Texture *lineTexture = SDL_CreateTextureFromSurface(appState.renderer, lineSurface);
    if (lineTexture) {
        // Offset X by viewport.x
        SDL_Rect lineRect = {viewport.x + 30, draw_y, lineSurface->w, lineSurface->h};
        SDL_RenderCopy(appState.renderer, lineTexture, nullptr, &lineRect);
        SDL_DestroyTexture(lineTexture);
    }
    SDL_FreeSurface(lineSurface);
}

void drawEditor(app::AppState &appState, buffer::EditorBuffer &buffer, config::EditorConfig &config, text::FontManager &fonts,
                text::Typesetter &typesetter, SDL_Rect viewport) {
    TTF_Font *font = fonts.getFont();
    if (!font) {
        return;
    }

    SDL_RenderSetClipRect(appState.renderer, &viewport);

    auto textColor = config.font.color;
    auto lines = typesetter.layout(buffer, config);
    auto [cursorX, cursorY] = typesetter.getCursorPixelPos(buffer, config, fonts);
    int lineHeight = config.font.size + 2;

    updateCamera(appState, cursorY, lineHeight, viewport);

    int lineCount = 1;
    for (const auto &line : lines) {
        int draw_y = viewport.y + line.y_pixel_offset - appState.scroll_y;

        if (draw_y + lineHeight < viewport.y) {
            lineCount++;
            continue;
        }
        if (draw_y > viewport.y + viewport.h) {
            break;
        }

        drawLine(appState, font, line, lineCount, draw_y, textColor, viewport);
        lineCount++;
    }

    drawCursor(appState, config, cursorX, cursorY, lineHeight, viewport);

    SDL_RenderSetClipRect(appState.renderer, NULL);
}

void drawStatusBar(app::AppState &appState, config::EditorConfig &config, text::FontManager &fonts, input::VimMode mode,
                   const std::string &commandText) {
    TTF_Font *font = fonts.getFont();
    if (!font) {
        return;
    }

    int barHeight = 30;
    SDL_Rect statusBar = {0, appState.window_height - barHeight, appState.window_width, barHeight};

    auto bg = config.theme.selection;
    SDL_SetRenderDrawColor(appState.renderer, bg.r, bg.g, bg.b, 255);
    SDL_RenderFillRect(appState.renderer, &statusBar);

    std::string modeStr;
    switch (mode) {
    case input::VimMode::NORMAL:
        modeStr = " NORMAL ";
        break;
    case input::VimMode::INSERT:
        modeStr = " INSERT ";
        break;
    case input::VimMode::VISUAL:
        modeStr = " VISUAL ";
        break;
    case input::VimMode::COMMAND:
        modeStr = ":" + commandText;
        break;
    default:
        modeStr = "";
    }

    if (!modeStr.empty()) {
        SDL_Surface *textSurf = TTF_RenderUTF8_Blended(font, modeStr.c_str(), config.theme.foreground);
        if (textSurf) {
            auto *textTex = SDL_CreateTextureFromSurface(appState.renderer, textSurf);
            if (textTex) {
                SDL_Rect textRect = {10, appState.window_height - barHeight + (barHeight - textSurf->h) / 2, textSurf->w,
                                     textSurf->h};
                SDL_RenderCopy(appState.renderer, textTex, nullptr, &textRect);
                SDL_DestroyTexture(textTex);
            }
            SDL_FreeSurface(textSurf);
        }
    }
}

}
