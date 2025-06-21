#pragma once
#include "sprite.h"
#include "../utils/math.h"
#include <string>
#include <optional> // For std::optional

struct SDL_Renderer;
struct SDL_FRect;
struct SDL_FColor;

namespace engine::resource {
    class ResourceManager;
}

namespace engine::render {
class Camera;

/**
 * @brief 封装 SDL3 渲染操作
 *
 * 包装 SDL_Renderer 并提供清除屏幕、绘制精灵和呈现最终图像的方法。
 * 在构造时初始化。依赖于一个有效的 SDL_Renderer 和 ResourceManager。
 * 构造失败会抛出异常。
 */
class Renderer final{
private:
    SDL_Renderer* renderer_ = nullptr;                              ///< @brief 指向 SDL_Renderer 的非拥有指针
    engine::resource::ResourceManager* resource_manager_ = nullptr; ///< @brief 指向 ResourceManager 的非拥有指针
    
public:
    /**
     * @brief 构造函数
     *
     * @param sdl_renderer 指向有效的 SDL_Renderer 的指针。不能为空。
     * @param resource_manager 指向有效的 ResourceManager 的指针。不能为空。
     * @throws std::runtime_error 如果任一指针为 nullptr。
     */
    Renderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager);

    /**
     * @brief 绘制一个精灵
     * 
     * @param sprite 包含纹理ID、源矩形和翻转状态的 Sprite 对象。
     * @param position 世界坐标中的左上角位置。
     * @param scale 缩放因子。
     * @param angle 旋转角度（度）。
     */
    void drawSprite(const Camera& camera, const Sprite& sprite, const glm::vec2& position, 
                    const glm::vec2& scale = {1.0f, 1.0f}, double angle = 0.0f);

    /**
     * @brief 绘制视差滚动背景
     * 
     * @param sprite 包含纹理ID、源矩形和翻转状态的 Sprite 对象。
     * @param position 世界坐标。
     * @param scroll_factor 滚动因子。
     * @param scale 缩放因子。
     */
    void drawParallax(const Camera& camera, const Sprite& sprite, const glm::vec2& position, 
                      const glm::vec2& scroll_factor, glm::bvec2 repeat = {true, true}, const glm::vec2& scale = {1.0f, 1.0f});

    /**
     * @brief 在屏幕坐标中直接渲染一个用于UI的Sprite对象。
     *
     * @param sprite 包含纹理ID、源矩形和翻转状态的Sprite对象。
     * @param position 屏幕坐标中的左上角位置。
     * @param size 可选：目标矩形的大小。如果为 std::nullopt，则使用Sprite的原始大小。
     */
    void drawUISprite(const Sprite& sprite, const glm::vec2& position, const std::optional<glm::vec2>& size = std::nullopt);

    /**
     * @brief 绘制填充矩形
     * 
     * @param rect 矩形区域
     * @param color 填充颜色
     */
    void drawUIFilledRect(const engine::utils::Rect& rect, const engine::utils::FColor& color);

    void present();                                                     ///< @brief 更新屏幕，包装 SDL_RenderPresent 函数
    void clearScreen();                                                 ///< @brief 清屏，包装 SDL_RenderClear 函数

    void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);        ///< @brief 设置绘制颜色，包装 SDL_SetRenderDrawColor 函数，使用 Uint8 类型
    void setDrawColorFloat(float r, float g, float b, float a = 1.0f);  ///< @brief 设置绘制颜色，包装 SDL_SetRenderDrawColorFloat 函数，使用 float 类型

    SDL_Renderer* getSDLRenderer() const { return renderer_; }          ///< @brief 获取底层的 SDL_Renderer 指针

    // 禁用拷贝和移动语义
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(Renderer&&) = delete;

private:
    std::optional<SDL_FRect> getSpriteSrcRect(const Sprite& sprite);     ///< @brief 获取精灵的源矩形，用于具体绘制。出现错误则返回std::nullopt并跳过绘制
    bool isRectInViewport(const Camera& camera, const SDL_FRect& rect);  ///< @brief 判断矩形是否在视口中，用于视口裁剪

};

} // namespace engine::render
