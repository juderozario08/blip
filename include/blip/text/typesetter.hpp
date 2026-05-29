#pragma once
#include <blip/buffer/buffer.hpp>
#include <blip/config/editor.hpp>
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
};
}
