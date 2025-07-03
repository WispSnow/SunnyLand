#pragma once
#include <SDL3/SDL_render.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string_view>
#include <string>
#include <unordered_map>
#include <memory>
#include <glm/vec2.hpp>
#include "../utils/math.h"

struct TTF_TextEngine;

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {
    class Camera;
/**
 * @brief 使用 SDL_ttf 和 TTF_Text 对象处理文本渲染。
 *
 * 封装 TTF_TextEngine 并提供创建和绘制 TTF_Text 对象的方法，
 * 管理字体加载和颜色设置。
 */
class TextRenderer final {
private:
    ///< @brief TTF_Text 的删除器函数对象，用于智能指针管理
    struct TTFTextDeleter {
        void operator()(TTF_Text* text) const {
            if (text) {
                TTF_DestroyText(text);
            }
        }
    };

    SDL_Renderer* sdl_renderer_ = nullptr;                          ///< @brief 持有渲染器的非拥有指针
    engine::resource::ResourceManager* resource_manager_ = nullptr; ///< @brief 持有资源管理器的非拥有指针

    ///< @brief 缓存TTF_Text对象，使用 TTF_Text 的地址作为键
             // NOTE: 谨慎使用地址做键，它不是对象的唯一标识符（系统可能回收再分配相同地址）
    std::unordered_map<uintptr_t, std::unique_ptr<TTF_Text, TTFTextDeleter>> text_cache_;
    
    TTF_TextEngine* text_engine_ = nullptr;         ///< @brief 使用SDL3引入的 TTF_TextEngine 来进行绘制

public:
    /**
     * @brief 构造 TextRenderer。
     *
     * @param sdl_renderer 有效的 SDL_Renderer 指针。
     * @param resource_manager 有效的 ResourceManager 指针（用于字体加载）。
     * @throws std::runtime_error 如果初始化失败。
     */
    TextRenderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager);

    ~TextRenderer();            ///< @brief 析构函数，按需调用close()。

    void close();               ///< @brief 显式关闭。清理 TTF_TextEngine 并关闭SDL_ttf。

    void clearCache() { text_cache_.clear(); }      ///< @brief 清空缓存


    // --- 一次性绘制的版本（只接受右值） ---

    /**
     * @brief 绘制UI上的字符串。
     *        
     * @param text UTF-8 字符串内容。
     * @param font_id 字体 ID。
     * @param font_size 字体大小。
     * @param position 左上角屏幕位置。
     * @param color 文本颜色。(默认为白色)
     */
    void drawUIText(std::string&& text, std::string_view font_id, int font_size, 
                  const glm::vec2& position, const engine::utils::FColor& color = {1.0f, 1.0f, 1.0f, 1.0f});

    /**
     * @brief 绘制地图上的字符串。
     *        
     * @param camera 相机
     * @param text UTF-8 字符串内容。
     * @param font_id 字体 ID。
     * @param font_size 字体大小。
     * @param position 左上角屏幕位置。
     * @param color 文本颜色。
     */
    void drawText(const Camera& camera, std::string&& text, std::string_view font_id, int font_size, 
                  const glm::vec2& position, const engine::utils::FColor& color = {1.0f, 1.0f, 1.0f, 1.0f});

    /**
     * @brief 获取文本的尺寸。
     *
     * @param text 要测量的文本。
     * @param font_id 字体 ID。
     * @param font_size 字体大小。
     * @return 文本的尺寸。
     */
    glm::vec2 getTextSize(std::string&& text, std::string_view font_id, int font_size);

    // --- 使用缓存绘制的版本（只接受左值引用） ---

    void drawUIText(const std::string& text, std::string_view font_id, int font_size, 
                  const glm::vec2& position, const engine::utils::FColor& color = {1.0f, 1.0f, 1.0f, 1.0f}, bool is_dirty = true);

    void drawText(const Camera& camera, const std::string& text, std::string_view font_id, int font_size, 
                  const glm::vec2& position, const engine::utils::FColor& color = {1.0f, 1.0f, 1.0f, 1.0f}, bool is_dirty = true);

    glm::vec2 getTextSize(const std::string& text, std::string_view font_id, int font_size, bool is_dirty = true);

    // 禁用拷贝和移动语义
    TextRenderer(const TextRenderer&) = delete;
    TextRenderer& operator=(const TextRenderer&) = delete;
    TextRenderer(TextRenderer&&) = delete;
    TextRenderer& operator=(TextRenderer&&) = delete;

private:
    /// @brief 尝试获取缓存中的TTF_Text对象，失败则返回空指针
    TTF_Text* getTTFText(const std::string& text);

    /// @brief 创建一个新的TTF_Text对象并添加到缓存中
    TTF_Text* createTTFText(const std::string& text, TTF_Font* font);

}; // class TextRenderer

} // namespace engine::render