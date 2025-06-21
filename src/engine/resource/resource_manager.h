#pragma once
#include <memory> // 用于 std::unique_ptr
#include <string> // 用于 std::string
#include <string_view> // 用于 std::string_view
#include <glm/glm.hpp>

// 前向声明 SDL 类型
struct SDL_Renderer;
struct SDL_Texture;
struct Mix_Chunk;
struct Mix_Music;
struct TTF_Font;

namespace engine::resource {

// 前向声明内部管理器
class TextureManager;
class AudioManager;
class FontManager;

/**
 * @brief 作为访问各种资源管理器的中央控制点（外观模式 Facade）。
 * 在构造时初始化其管理的子系统。构造失败会抛出异常。
 */
class ResourceManager final{
private:
    // 使用 unique_ptr 确保所有权和自动清理
    std::unique_ptr<TextureManager> texture_manager_;
    std::unique_ptr<AudioManager> audio_manager_;
    std::unique_ptr<FontManager> font_manager_;

public:
    /**
     * @brief 构造函数，执行初始化。
     * @param renderer SDL_Renderer 的指针，传递给需要它的子管理器。不能为空。
     */
    explicit ResourceManager(SDL_Renderer* renderer);       // explicit 关键字用于防止隐式转换, 对于单一参数的构造函数，通常考虑添加

    ~ResourceManager();  // 显式声明析构函数，这是为了能让智能指针正确管理仅有前向声明的类

    void clear();        ///< @brief 清空所有资源

    // 当前设计中，我们只需要一个ResourceManager，所有权不变，所以不需要拷贝、移动相关构造及赋值运算符
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    // --- 统一资源访问接口 ---
    // -- Texture --
    SDL_Texture* loadTexture(std::string_view file_path);     ///< @brief 载入纹理资源
    SDL_Texture* getTexture(std::string_view file_path);      ///< @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载
    void unloadTexture(std::string_view file_path);          ///< @brief 卸载指定的纹理资源
    glm::vec2 getTextureSize(std::string_view file_path);    ///< @brief 获取指定纹理的尺寸
    void clearTextures();                                      ///< @brief 清空所有纹理资源

    // -- Sound Effects (Chunks) --
    Mix_Chunk* loadSound(std::string_view file_path);         ///< @brief 载入音效资源
    Mix_Chunk* getSound(std::string_view file_path);          ///< @brief 尝试获取已加载音效的指针，如果未加载则尝试加载
    void unloadSound(std::string_view file_path);             ///< @brief 卸载指定的音效资源
    void clearSounds();                                         ///< @brief 清空所有音效资源

    // -- Music --
    Mix_Music* loadMusic(std::string_view file_path);         ///< @brief 载入音乐资源
    Mix_Music* getMusic(std::string_view file_path);          ///< @brief 尝试获取已加载音乐的指针，如果未加载则尝试加载
    void unloadMusic(std::string_view file_path);             ///< @brief 卸载指定的音乐资源
    void clearMusic();                                          ///< @brief 清空所有音乐资源

    // -- Fonts --
    TTF_Font* loadFont(std::string_view file_path, int point_size);     ///< @brief 载入字体资源
    TTF_Font* getFont(std::string_view file_path, int point_size);      ///< @brief 尝试获取已加载字体的指针，如果未加载则尝试加载
    void unloadFont(std::string_view file_path, int point_size);        ///< @brief 卸载指定的字体资源
    void clearFonts();                                                  ///< @brief 清空所有字体资源
};

} // namespace engine::resource
