#pragma once
#include <SDL_ttf.h>
#include <string>

namespace text {
class FontManager {
  public:
    FontManager();
    ~FontManager();

    bool updateFont(std::string &family, std::string &style, int size);

    TTF_Font *getFont() const;

  private:
    TTF_Font *current_font;
    std::string current_family;
    std::string current_style;
    int current_size;
};
}
