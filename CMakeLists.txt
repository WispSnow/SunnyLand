# 标题
cmake_minimum_required(VERSION 3.10.0)
project(SunnyLand VERSION 0.1.0 LANGUAGES C CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置编译选项
if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置编译输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR})

set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# 查找并载入Cmake预设
find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(SDL3_ttf REQUIRED)
find_package(glm REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)

# 添加可执行文件
add_executable(${TARGET} 
                src/main.cpp
                src/engine/core/game_app.cpp
                src/engine/core/time.cpp
                src/engine/core/config.cpp
                src/engine/core/context.cpp
                src/engine/resource/resource_manager.cpp
                src/engine/resource/texture_manager.cpp
                src/engine/resource/audio_manager.cpp
                src/engine/resource/font_manager.cpp
                src/engine/render/renderer.cpp
                src/engine/render/camera.cpp
                src/engine/render/animation.cpp
                src/engine/input/input_manager.cpp
                src/engine/object/game_object.cpp
                src/engine/component/sprite_component.cpp
                src/engine/component/transform_component.cpp
                src/engine/component/parallax_component.cpp
                src/engine/component/tilelayer_component.cpp
                src/engine/component/physics_component.cpp
                src/engine/component/collider_component.cpp
                src/engine/component/animation_component.cpp
                src/engine/component/health_component.cpp
                src/engine/physics/physics_engine.cpp
                src/engine/physics/collision.cpp
                src/engine/scene/scene.cpp
                src/engine/scene/scene_manager.cpp
                src/engine/scene/level_loader.cpp
                src/game/scene/game_scene.cpp
                src/game/component/player_component.cpp
                src/game/component/ai_component.cpp
                src/game/component/state/player_state.cpp
                src/game/component/state/idle_state.cpp
                src/game/component/state/walk_state.cpp
                src/game/component/state/jump_state.cpp
                src/game/component/state/fall_state.cpp
                src/game/component/state/hurt_state.cpp
                src/game/component/state/dead_state.cpp
                src/game/component/ai/patrol_behavior.cpp
                src/game/component/ai/updown_behavior.cpp
                src/game/component/ai/jump_behavior.cpp
                )

# 链接库
target_link_libraries(${TARGET}
                        ${SDL3_LIBRARIES}
                        SDL3_image::SDL3_image
                        SDL3_mixer::SDL3_mixer
                        SDL3_ttf::SDL3_ttf
                        glm::glm
                        nlohmann_json::nlohmann_json
                        spdlog::spdlog
                        )