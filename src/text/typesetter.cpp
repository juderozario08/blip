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

    if (rawText.empty() || rawText.back() == '\n') {
        int yPos = currentLineIndex * lineHeight;
        lines.push_back({"", yPos});
    }
    return lines;
}
}
