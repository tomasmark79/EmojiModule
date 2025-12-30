#include <EmojiModuleLib/EmojiModuleLib.hpp>

namespace dotnamecpp::v1 {

  EmojiModuleLib::EmojiModuleLib(const UtilsFactory::AppComponents &utilsComponents)
      : logger_(utilsComponents.logger ? utilsComponents.logger
                                       : std::make_shared<dotnamecpp::logging::NullLogger>()),
        assetManager_(utilsComponents.assetManager) {

    if (!assetManager_ || !assetManager_->validate()) {
      logger_->errorStream() << "Invalid or missing asset manager";
      return;
    }

    emoji_ = std::make_unique<dotnamecpp::emoji::Emoji>(utilsComponents);
    emoji_->emojiChainTest<false>();
    logger_->infoStream() << "Static emoji: " << emoji_->getEmoji();

    logger_->infoStream() << libName_ << " initialized ...";
    isInitialized_ = true;
  }

  EmojiModuleLib::~EmojiModuleLib() {
    if (isInitialized_) {
      logger_->infoStream() << libName_ << " ... destructed";
    } else {
      logger_->infoStream() << libName_ << " ... (not initialized) destructed";
    }
  }

  bool EmojiModuleLib::isInitialized() const noexcept { return isInitialized_; }

  const std::shared_ptr<dotnamecpp::assets::IAssetManager> &
      EmojiModuleLib::getAssetManager() const noexcept {
    return assetManager_;
  }

} // namespace dotnamecpp::v1
