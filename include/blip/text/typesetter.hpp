#pragma once
#include <blip/buffer/buffer.hpp>
#include <blip/config/editor.hpp>
#include <blip/text/font_manager.hpp>
#include <string>
#include <vector>

namespace text {
struct VisualLine {
    std::string text;
    int y_pixel_offset;
};

class Typesetter {
  public:
    Typesetter();
    std::vector<VisualLine> layout(buffer::EditorBuffer &buffer, config::EditorConfig &config);
    std::pair<int, int> getCursorPixelPos(buffer::EditorBuffer &buffer, config::EditorConfig &config, FontManager &fonts);
};
}
