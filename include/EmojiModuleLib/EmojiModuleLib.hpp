#pragma once

#include <Emoji/Emoji.hpp>
#include <EmojiModuleLib/version.h> // cmake configuration will generate this file
#include <Utils/UtilsFactory.hpp>

#include <memory>

namespace dotnamecpp::v1 {
  using namespace dotnamecpp::utils;
  class EmojiModuleLib {

  public:
    EmojiModuleLib(const UtilsFactory::AppComponents &utilsComponents);
    ~EmojiModuleLib();
    EmojiModuleLib(const EmojiModuleLib &other) = delete;
    EmojiModuleLib &operator=(const EmojiModuleLib &other) = delete;
    EmojiModuleLib(EmojiModuleLib &&other) = delete;
    EmojiModuleLib &operator=(EmojiModuleLib &&other) = delete;

    [[nodiscard]] bool isInitialized() const noexcept;
    [[nodiscard]]
    const std::shared_ptr<dotnamecpp::assets::IAssetManager> &getAssetManager() const noexcept;

    std::unique_ptr<dotnamecpp::emoji::Emoji> emoji_;

    // Public Emoji methods
    [[nodiscard]] std::string getRandomEmoji() const;
    [[nodiscard]] std::string getEmoji() const;
    [[nodiscard]] std::string getEmoji(char32_t *code, size_t totalCodePoints) const;
    [[nodiscard]] std::string getEmoji(int32_t *code, size_t totalCodePoints) const;

  private:
    bool isInitialized_{false};
    static constexpr const char *libName_ = "EmojiModuleLib v" EMOJIMODULELIB_VERSION;

    std::shared_ptr<dotnamecpp::logging::ILogger> logger_;
    std::shared_ptr<dotnamecpp::assets::IAssetManager> assetManager_;
  };

} // namespace dotnamecpp::v1
