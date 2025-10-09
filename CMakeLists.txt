# ============================================
# 项目主配置文件
# ============================================
cmake_minimum_required(VERSION 3.10.0)
project(SunnyLand VERSION 0.1.0 LANGUAGES C CXX)

# ============================================
# 基础配置
# ============================================

# 指定搜索路径（用于find_package在编译期查找库）
set(CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/prebuilt)

# 指定目标名称
set(TARGET ${PROJECT_NAME}-${CMAKE_SYSTEM_NAME})

# 依赖库默认链接类型：ON = 动态链接(.dll/.so/.dylib)，OFF = 静态链接(.lib/.a)
# 注意：可以在Dependencies.cmake中为每个库单独指定
option(BUILD_SHARED_LIBS "依赖库默认编译为动态库" OFF)

# ============================================
# 引入模块化配置
# ============================================

# 编译器设置（C++标准、编译选项等）
include(cmake/CompilerSettings.cmake)

# 运行时路径配置（RPATH）
include(cmake/RuntimePath.cmake)

# 依赖管理
include(cmake/Dependencies.cmake)

# 构建辅助函数
include(cmake/BuildHelpers.cmake)

# 项目信息打印
include(cmake/ProjectInfo.cmake)

# ============================================
# 设置项目依赖
# ============================================

# 调用依赖配置函数（定义在Dependencies.cmake中）
setup_project_dependencies()

# ============================================
# 项目源文件
# ============================================

set(SOURCES
    src/main.cpp
    src/engine/audio/audio_player.cpp
    src/engine/core/game_app.cpp
    src/engine/core/time.cpp
    src/engine/core/config.cpp
    src/engine/core/context.cpp
    src/engine/core/game_state.cpp
    src/engine/resource/resource_manager.cpp
    src/engine/resource/texture_manager.cpp
    src/engine/resource/audio_manager.cpp
    src/engine/resource/font_manager.cpp
    src/engine/render/renderer.cpp
    src/engine/render/camera.cpp
    src/engine/render/animation.cpp
    src/engine/render/text_renderer.cpp
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
    src/engine/component/audio_component.cpp
    src/engine/physics/physics_engine.cpp
    src/engine/physics/collision.cpp
    src/engine/scene/scene.cpp
    src/engine/scene/scene_manager.cpp
    src/engine/scene/level_loader.cpp
    src/engine/ui/ui_manager.cpp
    src/engine/ui/ui_element.cpp
    src/engine/ui/ui_interactive.cpp
    src/engine/ui/ui_panel.cpp
    src/engine/ui/ui_image.cpp
    src/engine/ui/ui_label.cpp
    src/engine/ui/ui_button.cpp
    src/engine/ui/state/ui_normal_state.cpp
    src/engine/ui/state/ui_pressed_state.cpp
    src/engine/ui/state/ui_hover_state.cpp
    src/game/scene/game_scene.cpp
    src/game/scene/title_scene.cpp
    src/game/scene/helps_scene.cpp
    src/game/scene/menu_scene.cpp
    src/game/scene/end_scene.cpp
    src/game/component/player_component.cpp
    src/game/component/ai_component.cpp
    src/game/component/state/player_state.cpp
    src/game/component/state/idle_state.cpp
    src/game/component/state/walk_state.cpp
    src/game/component/state/jump_state.cpp
    src/game/component/state/fall_state.cpp
    src/game/component/state/climb_state.cpp
    src/game/component/state/hurt_state.cpp
    src/game/component/state/dead_state.cpp
    src/game/component/ai/patrol_behavior.cpp
    src/game/component/ai/updown_behavior.cpp
    src/game/component/ai/jump_behavior.cpp
    src/game/data/session_data.cpp
)

# Windows平台添加资源文件
if(WIN32)
    list(APPEND SOURCES resources.rc)
endif()

# ============================================
# 可执行文件配置
# ============================================

# 创建可执行文件
add_executable(${TARGET} ${SOURCES})

# 链接所有依赖库
target_link_libraries(${TARGET}
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    glm::glm
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

# ============================================
# 应用配置
# ============================================

# 设置编译选项（定义在CompilerSettings.cmake中）
setup_compiler_options(${TARGET})

# 配置资源文件复制（定义在BuildHelpers.cmake中）
setup_asset_copy(${TARGET})

# 配置Windows DLL复制（定义在BuildHelpers.cmake中）
setup_windows_dll_copy(${TARGET})

# ============================================
# 打印配置信息
# ============================================

# 打印项目配置完成信息（定义在ProjectInfo.cmake中）
print_project_info(${TARGET})
