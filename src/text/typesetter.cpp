#include <blip/text/typesetter.hpp>
#include <sstream>

namespace text {
Typesetter::Typesetter() {}

std::vector<VisualLine> Typesetter::layout(buffer::EditorBuffer &buffer, config::EditorConfig &config) {
    std::vector<VisualLine> lines;

    std::string rawText = buffer.getText();
    int lineHeight = config.font.size;

    std::istringstream stream(rawText);
    std::string line;
    int currentLineIndex = 0;

    while (std::getline(stream, line)) {
        int yPos = currentLineIndex * lineHeight;
        lines.push_back({line, yPos});
        currentLineIndex++;
    }

    if (rawText.empty()) {
        int yPos = currentLineIndex * lineHeight;
        lines.push_back({"", yPos});
    }

    return lines;
}

std::pair<int, int> Typesetter::getCursorPixelPos(buffer::EditorBuffer &buffer, config::EditorConfig &config,
                                                  FontManager &fonts) {
    auto [row, col] = buffer.getCursorPosition2D();
    std::string lineText = buffer.getLineText(row);
    std::string textBeforeCursor = "";
    if (col <= lineText.length()) {
        textBeforeCursor = lineText.substr(0, col);
    } else {
        textBeforeCursor = lineText;
    }
    int xOffset = 0;
    if (!textBeforeCursor.empty()) {
        if (TTF_Font *font = fonts.getFont()) {
            int unusedHeight;
            TTF_SizeUTF8(font, textBeforeCursor.c_str(), &xOffset, &unusedHeight);
        }
    }
    int yOffset = row * (config.font.size + 2);
    return {xOffset, yOffset};
}

std::vector<VisualLine> Typesetter::layoutRange(const buffer::EditorBuffer &buffer, const config::EditorConfig &config,
                                                size_t startLine, size_t endLine) {
    std::vector<VisualLine> lines;
    int lineHeight = config.font.size + 2;

    for (size_t i = startLine; i < endLine; i++) {
        VisualLine vLine;
        vLine.text = buffer.getLineText(i);
        vLine.y_pixel_offset = i * lineHeight;
        lines.push_back(vLine);
    }
    return lines;
}

}
