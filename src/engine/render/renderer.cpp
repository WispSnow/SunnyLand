#include "renderer.h"
#include "../resource/resource_manager.h"
#include "camera.h"
#include "sprite.h"
#include <SDL3/SDL.h>
#include <stdexcept> // For std::runtime_error
#include <spdlog/spdlog.h>

namespace engine::render {

// 构造函数: 执行初始化，增加 ResourceManager
Renderer::Renderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager)
    : renderer_(sdl_renderer), resource_manager_(resource_manager) 
{
    spdlog::trace("构造 Renderer...");
    if (!renderer_) {
        throw std::runtime_error("Renderer 构造失败: 提供的 SDL_Renderer 指针为空。");
    }
    if (!resource_manager_) {
        // ResourceManager 是 drawSprite 所必需的
        throw std::runtime_error("Renderer 构造失败: 提供的 ResourceManager 指针为空。");
    }
    setDrawColor(0, 0, 0, 255);
    spdlog::trace("Renderer 构造成功。");
}

void Renderer::drawSprite(const Camera& camera, const Sprite& sprite, const glm::vec2& position, const glm::vec2& scale, double angle) {
    auto texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return;
    }

    auto src_rect = getSpriteSrcRect(sprite);
    if (!src_rect.has_value()) {
        spdlog::error("无法获取精灵的源矩形，ID: {}", sprite.getTextureId());
        return;
    }

    // 应用相机变换
    glm::vec2 position_screen = camera.worldToScreen(position);

    // 计算目标矩形，注意 position 是精灵的左上角坐标
    float scaled_w = src_rect.value().w * scale.x;
    float scaled_h = src_rect.value().h * scale.y;
    SDL_FRect dest_rect = {
        position_screen.x, 
        position_screen.y, 
        scaled_w,
        scaled_h
    };

    if (!isRectInViewport(camera, dest_rect)) { // 视口裁剪：如果精灵超出视口，则不绘制
        // spdlog::info("精灵超出视口范围，ID: {}", sprite.getTextureId());
        return;
    }

    // 执行绘制(默认旋转中心为精灵的中心点)
    if (!SDL_RenderTextureRotated(renderer_, texture, &src_rect.value(), &dest_rect, angle, NULL, sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
        spdlog::error("渲染旋转纹理失败（ID: {}）：{}", sprite.getTextureId(), SDL_GetError());
    }   
}

void Renderer::drawParallax(const Camera &camera, const Sprite &sprite, const glm::vec2 &position, const glm::vec2 &scroll_factor, glm::bvec2 repeat, const glm::vec2 &scale)
{
    auto texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return;
    }

    auto src_rect = getSpriteSrcRect(sprite);
    if (!src_rect.has_value()) {
        spdlog::error("无法获取精灵的源矩形，ID: {}", sprite.getTextureId());
        return;
    }

    // 应用相机变换
    glm::vec2 position_screen = camera.worldToScreenWithParallax(position, scroll_factor);

    // 计算缩放后的纹理尺寸 
    float scaled_tex_w = src_rect.value().w * scale.x;
    float scaled_tex_h = src_rect.value().h * scale.y;

    glm::vec2 start, stop;
    glm::vec2 viewport_size = camera.getViewportSize();

    if (repeat.x) {
        // 使用 glm::mod 进行浮点数取模
        start.x = glm::mod(position_screen.x, scaled_tex_w) - scaled_tex_w;
        stop.x = viewport_size.x;
    } else {
        start.x = position_screen.x;
        stop.x = glm::min(position_screen.x + scaled_tex_w, viewport_size.x); // 结束点是一个纹理宽度之后，但不超过视口宽度
    }
    if (repeat.y) {
        start.y = glm::mod(position_screen.y, scaled_tex_h) - scaled_tex_h;
        stop.y = viewport_size.y;
    } else {
        start.y = position_screen.y;
        stop.y = glm::min(position_screen.y + scaled_tex_h, viewport_size.y); // 结束点是一个纹理高度之后，但不超过视口高度
    }

    for (float y = start.y; y < stop.y; y += scaled_tex_h) {
        for (float x = start.x; x < stop.x; x += scaled_tex_w) {
            SDL_FRect dest_rect = {x, y, scaled_tex_w, scaled_tex_h};
            if (!SDL_RenderTexture(renderer_, texture, nullptr, &dest_rect)) {
                spdlog::error("渲染视差纹理失败（ID: {}）：{}", sprite.getTextureId(), SDL_GetError());
                return;
            }
        }
    }
}

void Renderer::drawUISprite(const Sprite& sprite, const glm::vec2& position, const std::optional<glm::vec2>& size) {
    auto texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return;
    }

    auto src_rect = getSpriteSrcRect(sprite);
    if (!src_rect.has_value()) {
        spdlog::error("无法获取精灵的源矩形，ID: {}", sprite.getTextureId());
        return;
    }

    SDL_FRect dest_rect = {position.x, position.y, 0, 0};   // 首先确定目标矩形的左上角坐标
    if (size.has_value()) {                                 // 如果提供了尺寸，则使用提供的尺寸
        dest_rect.w = size.value().x;
        dest_rect.h = size.value().y;
    } else {                                                // 如果未提供尺寸，则使用纹理的原始尺寸
        dest_rect.w = src_rect.value().w;
        dest_rect.h = src_rect.value().h;
    }

    // 执行绘制(未考虑UI旋转)
    if (!SDL_RenderTextureRotated(renderer_, texture, &src_rect.value(), &dest_rect, 0.0, nullptr, sprite.isFlipped() ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE)) {
        spdlog::error("渲染 UI Sprite 失败 (ID: {}): {}", sprite.getTextureId(), SDL_GetError());
    }
}

void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!SDL_SetRenderDrawColor(renderer_, r, g, b, a)) {
        spdlog::error("设置渲染绘制颜色失败：{}", SDL_GetError());
    }
}

void Renderer::setDrawColorFloat(float r, float g, float b, float a)
{
    if (!SDL_SetRenderDrawColorFloat(renderer_, r, g, b, a)) {
        spdlog::error("设置渲染绘制颜色失败：{}", SDL_GetError());
    }
}

void Renderer::clearScreen() {
    if (!SDL_RenderClear(renderer_)) {
        spdlog::error("清除渲染器失败：{}", SDL_GetError());
    }
}

void Renderer::drawUIFilledRect(const engine::utils::Rect &rect, const engine::utils::FColor &color)
{
    setDrawColorFloat(color.r, color.g, color.b, color.a);
    if (!SDL_RenderFillRect(renderer_, reinterpret_cast<const SDL_FRect*>(&rect))) {
        spdlog::error("绘制填充矩形失败：{}", SDL_GetError());
    }
    setDrawColorFloat(0, 0, 0, 1.0f);
}

void Renderer::present()
{
    SDL_RenderPresent(renderer_);
}

std::optional<SDL_FRect> Renderer::getSpriteSrcRect(const Sprite &sprite)
{
    SDL_Texture* texture = resource_manager_->getTexture(sprite.getTextureId());
    if (!texture) {
        spdlog::error("无法为 ID {} 获取纹理。", sprite.getTextureId());
        return std::nullopt;
    }

    auto src_rect = sprite.getSourceRect();
    if (src_rect.has_value()) {     // 如果Sprite中存在指定rect，则判断尺寸是否有效
        if (src_rect.value().w <= 0 || src_rect.value().h <= 0) {
            spdlog::error("源矩形尺寸无效，ID: {}", sprite.getTextureId());
            return std::nullopt;
        }
        return src_rect;
    } else {                        // 否则获取纹理尺寸并返回整个纹理大小
        SDL_FRect result = {0, 0, 0, 0};
        if (!SDL_GetTextureSize(texture, &result.w, &result.h)) {
            spdlog::error("无法获取纹理尺寸，ID: {}", sprite.getTextureId());
            return std::nullopt;
        }
        return result;
    }
}

bool Renderer::isRectInViewport(const Camera& camera, const SDL_FRect &rect)
{
    glm::vec2 viewport_size = camera.getViewportSize();
    return rect.x + rect.w >= 0 && rect.x <= viewport_size.x &&     // 相当于 AABB碰撞检测
           rect.y + rect.h >= 0 && rect.y <= viewport_size.y;
}

} // namespace engine::render
