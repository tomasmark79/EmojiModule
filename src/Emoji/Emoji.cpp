#include "Emoji.hpp"
#include <algorithm>
#include <cassert>
#include <cctype>
#include <fstream>
#include <random>
#include <regex>

namespace dotnamecpp::emoji {

  constexpr const int kMaxEmojiCodePoints = 8;
  constexpr const int kMaxBufferSize =
      (kMaxEmojiCodePoints * 4) +
      1; // Max kMaxEmojiCodePoints code points, each can take up to 4 bytes in UTF-8

  Emoji::Emoji(const UtilsFactory::ApplicationContext &context)
      : logger_(context.logger ? context.logger
                               : std::make_shared<dotnamecpp::logging::NullLogger>()),
        assetManager_(context.assetManager) {
    initMemory();
  }

  Emoji::~Emoji() {}

  std::string Emoji::getRandomEmoji() {
    // modern C++ way to generate random number
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, emojiMap_.size() - 1);

    if (emojiMap_.empty()) {
      return getEmoji();
    }
    auto it = emojiMap_.begin();
    std::advance(it, dis(gen));
    const auto &emojiEntry = it->first;
    char8_t buffer[kMaxBufferSize];
    char8_t *end = encodeUtf8Sequence(reinterpret_cast<const char32_t *>(emojiEntry.c_str()),
                                      emojiEntry.size(), buffer);
    *end = '\0';
    return std::string(reinterpret_cast<char *>(buffer));
  }

  std::string Emoji::getEmoji() { return "😀"; }

  std::string Emoji::getEmoji(char32_t *code, size_t totalCodePoints) {
    if (totalCodePoints > 0 && totalCodePoints <= 8) {
      char8_t buffer[kMaxBufferSize];
      char8_t *end = encodeUtf8Sequence(code, totalCodePoints, buffer);
      *end = '\0';
      return std::string(reinterpret_cast<char *>(buffer));
    }
    return getEmoji();
  }

  std::string Emoji::getEmoji(int32_t *code, size_t totalCodePoints) {
    if (totalCodePoints > 0 && totalCodePoints <= 8) {
      std::vector<char32_t> codePoints;
      codePoints.reserve(totalCodePoints);
      for (size_t i = 0; i < totalCodePoints; ++i) {
        codePoints.push_back(static_cast<char32_t>(code[i]));
      }
      char8_t buffer[kMaxBufferSize];
      char8_t *end = encodeUtf8Sequence(codePoints.data(), totalCodePoints, buffer);
      *end = '\0';
      return std::string(reinterpret_cast<char *>(buffer));
    }
    return getEmoji();
  }

  void Emoji::initMemory() {

    constexpr const char hashChar = '#';
    constexpr const char delimiter = ';';
    constexpr int groupPrefixLength = 9;
    constexpr int subgroupPrefixLength = 12;

    std::ifstream emojiFile_(assetManager_->getAssetsPath() / "emoji-test.txt");

    std::vector<char32_t> emojiCodePoints;
    std::string group;
    std::string subGroup;
    std::string unicodeVer;
    std::string tailDesc;
    std::string textDesc;
    std::string line;
    std::string token;

    while (std::getline(emojiFile_, line)) {

      if (line.empty()) {
        continue;
      }

      if (line[0] == '#' && (line.find("# subgroup:") == std::string::npos) &&
          (line.find("# group:") == std::string::npos)) {
        continue;
      }

      if (line.find("# subgroup:") != std::string::npos) {
        subGroup = line.substr(subgroupPrefixLength);
        continue;
      }

      if (line.find("# group:") != std::string::npos) {
        group = line.substr(groupPrefixLength);
        continue;
      }

      if ((line[0] != hashChar) && (line.find(hashChar) != std::string::npos) &&
          (line.find("# subgroup:") == std::string::npos) &&
          (line.find("# group:") == std::string::npos)) {

        std::string unicodeString = line.substr(0, line.find(delimiter));

        std::istringstream iss(unicodeString);
        emojiCodePoints.clear();
        while (iss >> token) {
          token.erase(
              std::remove_if(token.begin(), token.end(), [](char c) { return !std::isxdigit(c); }),
              token.end());

          uint32_t value;
          std::stringstream ss;
          ss << std::hex << token;
          ss >> value;
          emojiCodePoints.push_back(static_cast<char32_t>(value));
        }

        tailDesc = line.substr(line.find(hashChar) + 1);

        std::regex unicodeRegex(R"((E\d+\.\d+))"); // regular expression for extract unicode version
        std::smatch unicodeMatch;
        if (std::regex_search(tailDesc, unicodeMatch, unicodeRegex)) {
          unicodeVer = unicodeMatch[0];
        }

        // extract emoji text description
        std::string::size_type unicodeVersionPos = tailDesc.find(unicodeMatch[0]);
        if (!unicodeMatch.empty() && unicodeVersionPos != std::string::npos) {
          textDesc =
              tailDesc.substr(unicodeVersionPos + unicodeMatch[0].length() + 1, tailDesc.size());
        }

        // combine emoji character from code points
        char8_t buffer[64];
        char8_t *end = encodeUtf8Sequence(emojiCodePoints.data(), emojiCodePoints.size(), buffer);
        *end = '\0'; // Null-terminating the string

        // Create EmojiMap struct and insert into map
        Emoji::EmojiMap emojiEntry;
        emojiEntry.emojiCodePoints_ = emojiCodePoints;
        emojiEntry.emojiGroup_ = group;
        emojiEntry.emojiSubGroup_ = subGroup;
        emojiEntry.emojiUnicodeVersion_ = unicodeVer;
        emojiEntry.emojiTextDescription_ = textDesc;

        emojiMap_.emplace(std::u32string(emojiCodePoints.begin(), emojiCodePoints.end()),
                          std::move(emojiEntry));
      }
    }
    isPopulated_ = true;
    logger_->infoStream() << "Emoji map populated with " << emojiMap_.size() << " entries.";
  }

  char8_t *Emoji::encodeUtf8(char32_t emojiCodePoint, char8_t *buffer8_t) {
    auto byte = [](char32_t x) {
      assert(x <= 0xFF); // Maximum 8 bits
      return static_cast<char8_t>(x);
    };

    char32_t continuation = 128;
    if (emojiCodePoint >= 65536) {
      *buffer8_t++ = byte(0b1111'0000 | (emojiCodePoint >> 18));
      *buffer8_t++ = byte(continuation | ((emojiCodePoint >> 12) & 0b0011'1111));
      *buffer8_t++ = byte(continuation | ((emojiCodePoint >> 6) & 0b0011'1111));
      *buffer8_t++ = byte(continuation | (emojiCodePoint & 0b0011'1111));
    } else if (emojiCodePoint >= 2048) {
      *buffer8_t++ = byte(0b1110'0000 | (emojiCodePoint >> 12));
      *buffer8_t++ = byte(continuation | ((emojiCodePoint >> 6) & 0b0011'1111));
      *buffer8_t++ = byte(continuation | (emojiCodePoint & 0b0011'1111));
    } else if (emojiCodePoint >= 128) {
      *buffer8_t++ = byte(0b1100'0000 | (emojiCodePoint >> 6));
      *buffer8_t++ = byte(continuation | (emojiCodePoint & 0b0011'1111));
    } else {
      *buffer8_t++ = byte(emojiCodePoint);
    }

    return buffer8_t;
  }

  char8_t *Emoji::encodeUtf8Sequence(const char32_t *emojiCodePoints, size_t totalCodePoints,
                                     char8_t *buffer8_t) {
    for (size_t i = 0; i < totalCodePoints; ++i) {
      buffer8_t = encodeUtf8(emojiCodePoints[i], buffer8_t);
    }
    return buffer8_t;
  }

} // namespace dotnamecpp::emoji