# ============================================
# ImGui 配置模块
# ============================================
# 功能：配置ImGui源文件和头文件路径

# 定义 ImGui 源文件变量，方便管理
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    # Backend files
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer3.cpp
)

# 将 ImGui 的目录和其后端目录添加到头文件搜索路径
include_directories(
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

