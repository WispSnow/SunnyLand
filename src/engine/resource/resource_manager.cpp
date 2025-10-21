#include "resource_manager.h"
#include "texture_manager.h"
#include "audio_manager.h"
#include "font_manager.h" 
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h> 
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
 
namespace engine::resource {

ResourceManager::~ResourceManager() = default;

ResourceManager::ResourceManager(SDL_Renderer* renderer) {
    // --- 初始化各个子系统 --- (如果出现错误会抛出异常，由上层捕获)
    texture_manager_ = std::make_unique<TextureManager>(renderer);
    audio_manager_ = std::make_unique<AudioManager>();
    font_manager_ = std::make_unique<FontManager>();

    spdlog::trace("ResourceManager 构造成功。");
    // RAII: 构造成功即代表资源管理器可以正常工作，无需再初始化，无需检查指针是否为空
}

void ResourceManager::clear() {
    font_manager_->clearFonts();
    audio_manager_->clearSounds();
    audio_manager_->clearMusic();
    texture_manager_->clearTextures();
    spdlog::trace("ResourceManager 中的资源通过 clear() 清空。");
}

// --- 纹理接口实现 ---
SDL_Texture* ResourceManager::loadTexture(std::string_view file_path) {
    // 构造函数已经确保了 texture_manager_ 不为空，因此不需要再进行if检查，以免性能浪费
    return texture_manager_->loadTexture(file_path);
}

SDL_Texture* ResourceManager::getTexture(std::string_view file_path) {
    return texture_manager_->getTexture(file_path);
}

glm::vec2 ResourceManager::getTextureSize(std::string_view file_path) {
    return texture_manager_->getTextureSize(file_path);
}

void ResourceManager::unloadTexture(std::string_view file_path) {
    texture_manager_->unloadTexture(file_path);
}

void ResourceManager::clearTextures() {
    texture_manager_->clearTextures();
}

// --- 音频接口实现 ---
Mix_Chunk* ResourceManager::loadSound(std::string_view file_path) {
    return audio_manager_->loadSound(file_path);
}

Mix_Chunk* ResourceManager::getSound(std::string_view file_path) {
    return audio_manager_->getSound(file_path);
}

void ResourceManager::unloadSound(std::string_view file_path) {
    audio_manager_->unloadSound(file_path);
}

void ResourceManager::clearSounds() {
    audio_manager_->clearSounds();
}

Mix_Music* ResourceManager::loadMusic(std::string_view file_path) {
    return audio_manager_->loadMusic(file_path);
}

Mix_Music* ResourceManager::getMusic(std::string_view file_path) {
    return audio_manager_->getMusic(file_path);
}

void ResourceManager::unloadMusic(std::string_view file_path) {
    audio_manager_->unloadMusic(file_path);
}

void ResourceManager::clearMusic() {
    audio_manager_->clearMusic();
}

// --- 字体接口实现 ---
TTF_Font* ResourceManager::loadFont(std::string_view file_path, int point_size) {
    return font_manager_->loadFont(file_path, point_size);
}

TTF_Font* ResourceManager::getFont(std::string_view file_path, int point_size) {
    return font_manager_->getFont(file_path, point_size);
}

void ResourceManager::unloadFont(std::string_view file_path, int point_size) {
    font_manager_->unloadFont(file_path, point_size);
}

void ResourceManager::clearFonts() {
    font_manager_->clearFonts();
}

} // namespace engine::resource
