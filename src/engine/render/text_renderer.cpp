#include "text_renderer.h"
#include "camera.h"
#include "../resource/resource_manager.h"
#include <SDL3_ttf/SDL_ttf.h>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace engine::render {

TextRenderer::TextRenderer(SDL_Renderer* sdl_renderer, engine::resource::ResourceManager* resource_manager)
    : sdl_renderer_(sdl_renderer),
      resource_manager_(resource_manager)
{
    if (!sdl_renderer_ || !resource_manager_) {
        throw std::runtime_error("TextRenderer 需要一个有效的 SDL_Renderer 和 ResourceManager。");
    }
    // 初始化 SDL_ttf
    if (!TTF_WasInit() && TTF_Init() == false) {
         throw std::runtime_error("初始化 SDL_ttf 失败: " + std::string(SDL_GetError()));
    }

    text_engine_ = TTF_CreateRendererTextEngine(sdl_renderer_);
    if (!text_engine_) {
        spdlog::error("创建 TTF_TextEngine 失败: {}", SDL_GetError());
        throw std::runtime_error("创建 TTF_TextEngine 失败。");
    }
    spdlog::trace("TextRenderer 初始化成功.");
}

TextRenderer::~TextRenderer() {
    if (text_engine_) {
        close();
    }
}

void TextRenderer::close()
{
    if (text_engine_) {
        TTF_DestroyRendererTextEngine(text_engine_);
        text_engine_ = nullptr;
        spdlog::trace("TTF_TextEngine 销毁。");
    }
    TTF_Quit();     // 一定要确保在ResourceManager销毁之后调用
}

void TextRenderer::drawUIText(std::string_view text, std::string_view font_id, int font_size,
                              const glm::vec2 &position, const engine::utils::FColor &color)
{
    /* 构造函数已经保证了必要指针不会为空，这里不需要再检查 */
    TTF_Font* font = resource_manager_->getFont(font_id, font_size);
    if (!font) {
        spdlog::warn("drawUIText 获取字体失败: {} 大小 {}", font_id, font_size);
        return;
    }

    // 创建临时 TTF_Text 对象   (目前效率不高，未来可以考虑使用缓存优化)
    TTF_Text* temp_text_object = TTF_CreateText(text_engine_, font, text.data(), 0);
    if (!temp_text_object) {
        spdlog::error("drawUIText 创建临时 TTF_Text 失败: {}", SDL_GetError());
        return;
    }

    // 先渲染一次黑色文字模拟阴影
    TTF_SetTextColorFloat(temp_text_object, 0.0f, 0.0f, 0.0f, 1.0f);
    if (!TTF_DrawRendererText(temp_text_object, position.x + 2, position.y + 2)) {
        spdlog::error("drawUIText 绘制临时 TTF_Text 失败: {}", SDL_GetError());
    }

    // 然后正常绘制
    TTF_SetTextColorFloat(temp_text_object, color.r, color.g, color.b, color.a);
    if (!TTF_DrawRendererText(temp_text_object, position.x, position.y)) {
        spdlog::error("drawUIText 绘制临时 TTF_Text 失败: {}", SDL_GetError());
    }

    // 销毁临时 TTF_Text 对象
    TTF_DestroyText(temp_text_object);
}

void TextRenderer::drawText(const Camera &camera, std::string_view text, std::string_view font_id, int font_size, 
                            const glm::vec2 &position, const engine::utils::FColor &color)
{
    // 应用相机变换
    glm::vec2 position_screen = camera.worldToScreen(position);

    // 用新坐标调用drawUIText即可
    drawUIText(text, font_id, font_size, position_screen, color);
}

glm::vec2 TextRenderer::getTextSize(std::string_view text, std::string_view font_id, int font_size) {
    /* 构造函数已经保证了必要指针不会为空，这里不需要再检查 */
    TTF_Font* font = resource_manager_->getFont(font_id, font_size);
    if (!font) {
        spdlog::warn("getTextSize 获取字体失败: {} 大小 {}", font_id, font_size);
        return glm::vec2(0.0f, 0.0f);
    }

    // 创建临时 TTF_Text 对象
    TTF_Text* temp_text_object = TTF_CreateText(text_engine_, font, text.data(), 0);
    if (!temp_text_object) {
        spdlog::error("getTextSize 创建临时 TTF_Text 失败: {}", SDL_GetError());
        return glm::vec2(0.0f, 0.0f);
    }

    int width, height;
    TTF_GetTextSize(temp_text_object, &width, &height);

    // 销毁临时 TTF_Text 对象
    TTF_DestroyText(temp_text_object);

    return glm::vec2(static_cast<float>(width), static_cast<float>(height));
} 

} // namespace engine::render 