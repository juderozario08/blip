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
    std::string rawText = buffer.getText();
    size_t cursorIndex = buffer.getCursor();

    int lineIndex = 0;
    size_t startOfLine = 0;

    for (size_t i = 0; i < cursorIndex && i < rawText.length(); ++i) {
        if (rawText[i] == '\n') {
            lineIndex++;
            startOfLine = i + 1;
        }
    }

    std::string textBeforeCursor = "";
    if (cursorIndex > startOfLine) {
        textBeforeCursor = rawText.substr(startOfLine, cursorIndex - startOfLine);
    }

    int xOffset = 0;
    if (!textBeforeCursor.empty()) {
        if (TTF_Font *font = fonts.getFont()) {
            int unusedHeight;
            TTF_SizeUTF8(font, textBeforeCursor.c_str(), &xOffset, &unusedHeight);
        }
    }

    int yOffset = lineIndex * config.font.size;

    return {xOffset, yOffset};
}
}
