#include <blip/ui/renderer.hpp>

namespace ui {

void drawBackground(app::AppState &appState, config::EditorConfig &state) {
    auto c = state.theme.background;
    SDL_SetRenderDrawColor(appState.renderer, c.r, c.g, c.b, c.a);
    SDL_RenderClear(appState.renderer);
}

static void updateCamera(app::AppState &appState, int cursorY, int lineHeight, const SDL_Rect &viewport,
                         config::EditorConfig &config) {
    if (cursorY + lineHeight > appState.scroll_y + viewport.h - config.layout.bottom_padding) {
        appState.scroll_y = cursorY + lineHeight - viewport.h + config.layout.bottom_padding;
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

    SDL_Rect cursorRect = {viewport.x + config.layout.text_offset_x + cursorX - 1, viewport.y + cursorY - appState.scroll_y - 1,
                           cursorWidth, lineHeight};
    auto cursorColor = config.theme.cursor;

    SDL_SetRenderDrawColor(appState.renderer, cursorColor.r, cursorColor.g, cursorColor.b, 100);
    SDL_SetRenderDrawBlendMode(appState.renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(appState.renderer, &cursorRect);
}

// ADDED: syntaxEngine parameter
static void drawLine(app::AppState &appState, const text::VisualLine &line, int lineCount, int draw_y, const SDL_Rect &viewport,
                     config::EditorConfig &config, int lineHeight, bool hasSelection, size_t selStart, size_t selEnd,
                     size_t lineStartIdx, const GlyphCache &glyphCache, text::SyntaxEngine &syntaxEngine) {

    std::string lineNumStr = std::to_string(lineCount);
    int current_x = viewport.x + config.layout.line_number_offset_x;
    for (char c : lineNumStr) {
        const auto &glyph = glyphCache.getGlyph(c);
        if (glyph.texture) {
            // FIX: Tint line numbers grey so they don't inherit syntax colors from the previous line
            SDL_SetTextureColorMod(glyph.texture, 100, 100, 100);
            SDL_Rect dest = {current_x, draw_y, glyph.width, glyph.height};
            SDL_RenderCopy(appState.renderer, glyph.texture, nullptr, &dest);
            current_x += glyph.width;
        }
    }

    size_t lineEndIdx = lineStartIdx + line.text.length();
    if (hasSelection && selStart <= lineEndIdx && selEnd >= lineStartIdx && selStart != selEnd) {
        size_t hlStart = std::max(lineStartIdx, selStart);
        size_t hlEnd = std::min(lineEndIdx, selEnd);

        std::string textBefore = line.text.substr(0, hlStart - lineStartIdx);
        std::string textHighlight = line.text.substr(hlStart - lineStartIdx, hlEnd - hlStart);

        int xOffset = glyphCache.measureString(textBefore, config.preference.tab_width);
        int hlWidth = glyphCache.measureString(textHighlight, config.preference.tab_width);

        if (selEnd > lineEndIdx) {
            hlWidth += glyphCache.getGlyph(' ').width;
        }

        SDL_Rect hlRect = {viewport.x + config.layout.text_offset_x + xOffset, draw_y, hlWidth, lineHeight};
        auto selColor = config.theme.selection;
        SDL_SetRenderDrawColor(appState.renderer, selColor.r, selColor.g, selColor.b, 150);
        SDL_RenderFillRect(appState.renderer, &hlRect);
    }

    current_x = viewport.x + config.layout.text_offset_x;

    // CHANGED: Use index-based loop so we can calculate the absolute byteIndex for the AST
    for (size_t i = 0; i < line.text.length(); i++) {
        char c = line.text[i];
        if (c == '\t') {
            current_x += glyphCache.getGlyph(' ').width * config.preference.tab_width;
            continue;
        }

        const auto &glyph = glyphCache.getGlyph(c);
        if (glyph.texture) {
            // ADDED: Ask Tree-sitter for the color at this exact byte index
            SDL_Color color = syntaxEngine.getColorForByte(lineStartIdx + i, config.theme);

            // ADDED: Hardware-tint the white texture before drawing
            SDL_SetTextureColorMod(glyph.texture, color.r, color.g, color.b);

            SDL_Rect dest = {current_x, draw_y, glyph.width, glyph.height};
            SDL_RenderCopy(appState.renderer, glyph.texture, nullptr, &dest);
            current_x += glyph.width;
        }
    }
}

void drawEditor(app::AppState &appState, buffer::EditorBuffer &buffer, config::EditorConfig &config, text::FontManager &fonts,
                text::Typesetter &typesetter, SDL_Rect viewport, const ui::GlyphCache &glyphCache,
                text::SyntaxEngine &syntaxEngine) {

    TTF_Font *font = fonts.getFont();
    if (!font) {
        return;
    }

    size_t totalLines = buffer.getNumberOfLines();
    std::string gutterString = std::to_string(totalLines) + "  ";

    int gutterPixelWidth = 0;
    TTF_SizeUTF8(font, gutterString.c_str(), &gutterPixelWidth, nullptr);

    config.layout.text_offset_x = config.layout.line_number_offset_x + gutterPixelWidth;

    SDL_RenderSetClipRect(appState.renderer, &viewport);

    auto lines = typesetter.layout(buffer, config);
    auto [cursorX, cursorY] = typesetter.getCursorPixelPos(buffer, config, fonts);
    int lineHeight = config.font.size + 2;

    updateCamera(appState, cursorY, lineHeight, viewport, config);

    bool hasSelection = buffer.hasSelection();
    auto [selStart, selEnd] = buffer.getSelectionRange();

    int lineCount = 1;
    size_t currentCharIdx = 0;

    for (const auto &line : lines) {
        size_t currentLineLength = line.text.length() + 1;
        int draw_y = viewport.y + line.y_pixel_offset - appState.scroll_y;

        if (draw_y + lineHeight < viewport.y) {
            lineCount++;
            currentCharIdx += currentLineLength;
            continue;
        }
        if (draw_y > viewport.y + viewport.h) {
            break;
        }

        // ADDED: Pass syntaxEngine down
        drawLine(appState, line, lineCount, draw_y, viewport, config, lineHeight, hasSelection, selStart, selEnd, currentCharIdx,
                 glyphCache, syntaxEngine);

        lineCount++;
        currentCharIdx += currentLineLength;
    }

    int next_y_offset = 0;
    if (!lines.empty()) {
        next_y_offset = lines.back().y_pixel_offset + lineHeight;
    }

    while (lineCount <= totalLines) {
        int draw_y = viewport.y + next_y_offset - appState.scroll_y;

        if (draw_y > viewport.y + viewport.h) {
            break;
        }

        if (draw_y + lineHeight >= viewport.y) {
            text::VisualLine emptyLine;
            emptyLine.text = "";
            // ADDED: Pass syntaxEngine down
            drawLine(appState, emptyLine, lineCount, draw_y, viewport, config, lineHeight, hasSelection, selStart, selEnd,
                     currentCharIdx, glyphCache, syntaxEngine);
        }

        lineCount++;
        currentCharIdx++;
        next_y_offset += lineHeight;
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

    SDL_Rect statusBar = {0, appState.window_height - config.layout.status_bar_height, appState.window_width,
                          config.layout.status_bar_height};

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
                SDL_Rect textRect = {config.layout.status_bar_padding_x,
                                     appState.window_height - config.layout.status_bar_height +
                                         (config.layout.status_bar_height - textSurf->h) / 2,
                                     textSurf->w, textSurf->h};
                SDL_RenderCopy(appState.renderer, textTex, nullptr, &textRect);
                SDL_DestroyTexture(textTex);
            }
            SDL_FreeSurface(textSurf);
        }
    }
}

}
