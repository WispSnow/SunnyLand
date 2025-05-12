#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

int main(int, char**) {
    spdlog::info("你好，世界!");
    nlohmann::json json_data = {{"a",10}};
    auto num = json_data["a"].get<int>();
    spdlog::warn("json: {}", num);
    
    glm::vec2 a = glm::vec2(1.0f, 2.0f);
    glm::vec2 b = glm::vec2(3.0f, 4.0f);
    auto c = a * b;
    auto d = glm::distance(a, b);
    SDL_Log("d = (%f)", d);

    SDL_Log("c = (%f, %f)", c.x, c.y);

    std::cout << "Hello, World!" << std::endl;
    // SDL初始化
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    // 创建窗口
    SDL_Window *window = SDL_CreateWindow("Hello World!", 800, 600, 0);
    // 创建渲染器
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // SDL3_Image不需要手动初始化

    // 加载图片
    SDL_Texture *texture = IMG_LoadTexture(renderer, "assets/textures/Props/big-crate.png");

    // SDL_Mixer初始化
    if (!Mix_OpenAudio(0, NULL)) {
        std::cerr << "Mix_OpenAudio Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 读取音乐
    Mix_Music *music = Mix_LoadMUS("assets/audio/hurry_up_and_run.ogg");
    // 播放音乐
    Mix_PlayMusic(music, -1);

    // SDL_TTF初始化
    if (!TTF_Init()) {
        std::cerr << "TTF_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    // 加载字体
    TTF_Font *font = TTF_OpenFont("assets/fonts/VonwaonBitmap-16px.ttf", 24);

    // 创建文本纹理
    SDL_Color color = {255, 255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, "Hello, SDL! 中文也可以", 0, color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, surface);

    // SDL3 新的绘制文本方法
    TTF_TextEngine *textEngine = TTF_CreateRendererTextEngine(renderer);
    TTF_Text *text = TTF_CreateText(textEngine, font, "SDL3 新的文本渲染方式", 0);
    TTF_SetTextColor(text, 255, 0, 0, 255);
    TTF_SetTextWrapWidth(text, 50);
    // Do something with the window and renderer here...
    // 渲染循环
    glm::vec2 mousePos = glm::vec2(0.0f, 0.0f);
    while (true) {
        SDL_Event event;
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                break;
            }
        }
        auto state = SDL_GetMouseState(&mousePos.x, &mousePos.y);
        // SDL_Log("Mouse Pos: (%f, %f)", mousePos.x, mousePos.y);
        if (state & SDL_BUTTON_LMASK) {
            SDL_Log("Left Button Down");
        }
        if (state & SDL_BUTTON_RMASK) {
            SDL_Log("Right Button Down");
        }
        // 清屏
        SDL_RenderClear(renderer);
        // 画一个长方形
        SDL_FRect rect = {100, 100, 200, 200};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
        

        // 画图片
        SDL_FRect dstrect = {200, 200, 200, 200};
        SDL_RenderTexture(renderer, texture, NULL, &dstrect);

        // 画文本
        SDL_FRect textRect = {300, 300, static_cast<float>(surface->w), static_cast<float>(surface->h)};
        SDL_RenderTexture(renderer, textTexture, NULL, &textRect);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); 

        // 新的画文本方法：
        TTF_DrawRendererText(text, 400, 400);

        // 更新屏幕
        SDL_RenderPresent(renderer);
        

        
    }

    // 清理图片资源
    SDL_DestroyTexture(texture);

    // 清理音乐资源
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    Mix_Quit();

    // 清理字体资源
    SDL_DestroySurface(surface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    TTF_Quit();

    // 清理并退出
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}