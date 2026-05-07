#pragma once
#include <SDL_ttf.h>
#include <string>

namespace text {
enum class FontStyles { Regular = 0, Bold, Italic, BoldItalic, Count };

class FontManager {
  public:
    FontManager();
    ~FontManager();

    bool updateFontFamily(const std::string &family, int size);
    TTF_Font *getFont(FontStyles style = FontStyles::Regular) const;
    std::string createTTFPathVec(const std::string family, const std::string style);

  private:
    TTF_Font *fonts[static_cast<int>(FontStyles::Count)];
    std::string current_family;
    int current_size;
    void loadStyle(FontStyles style);
};
}
