#pragma once

#include <Utils/UtilsFactory.hpp>

#include <string>
#include <vector>

namespace dotnamecpp::emoji {
  using namespace dotnamecpp::utils;
  class Emoji {
  public:
    Emoji(const UtilsFactory::AppComponents &utilsComponents);
    ~Emoji();

    struct EmojiMap {
      std::vector<char32_t> emojiCodePoints_;
      std::string emojiGroup_;
      std::string emojiSubGroup_;
      std::string emojiUnicodeVersion_;
      std::string emojiTextDescription_;
    };

    std::map<std::u32string, EmojiMap> emojiMap_;

    std::string getRandomEmoji();
    std::string getEmoji();
    std::string getEmoji(char32_t *code, size_t totalCodePoints);
    std::string getEmoji(int32_t *code, size_t totalCodePoints);

    template <bool EnableTest>
    void emojiChainTest() {
      if constexpr (EnableTest) {
        std::string emojiChain;
        constexpr int32_t kEmojiStartCodePoint = 0x1F600;
        constexpr int32_t kEmojiEndCodePoint = 0x1FFFF;
        for (int32_t i = kEmojiStartCodePoint; i <= kEmojiEndCodePoint; i++) {
          emojiChain += getEmoji(&i, 1);
        }
        logger_->infoStream() << "Emoji chain: " << emojiChain;
      }
    }

    void initMemory();

  private:
    std::shared_ptr<dotnamecpp::logging::ILogger> logger_;
    std::shared_ptr<dotnamecpp::assets::IAssetManager> assetManager_;

    static char8_t *encodeUtf8(char32_t emojiCodePoint, char8_t *buffer8_t);
    static char8_t *encodeUtf8Sequence(const char32_t *emojiCodePoints, size_t totalCodePoints,
                                       char8_t *buffer8_t);

    bool isPopulated_{false};
  };
} // namespace dotnamecpp::emoji