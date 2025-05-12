#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

int main(int, char**) {
    // 设置日志等级，不设置的话默认为 info
    spdlog::set_level(spdlog::level::err);

    // 不同等级的log
    spdlog::trace("最低级别log!");
    spdlog::debug("调试信息!");
    spdlog::info("你好，世界!");
    spdlog::warn("警告!，很可能会出错");
    spdlog::error("程序出错啦!");
    spdlog::critical("最高级别的log!, 比error还严重!");

    // 格式化输出
    spdlog::info("日志格式化输出: {} {} {}", 1, "hello", 3.14);

    return 0;
}