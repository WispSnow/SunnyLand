# ============================================
# 依赖管理模块
# ============================================
# 功能：智能依赖获取和管理

# 包含FetchContent模块
include(FetchContent)

# 设置FetchContent配置
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# ============================================
# 辅助宏：查找或获取依赖（智能浅克隆 + 链接类型控制）
# 用法如下：
#
# 参数说明：
#   DEP_NAME      - 依赖的内部名称（用于FetchContent和add_subdirectory等）
#   PACKAGE_NAME  - 依赖的包名（用于find_package查找本地已安装的包）
#   GIT_REPO      - 依赖的Git仓库地址（用于FetchContent在线获取源码）
#   GIT_TAG       - 依赖的Git分支、标签或commit hash（用于指定获取源码的版本）
#   LOCAL_PATH    - 本地源码路径（如 external/SDL），用于本地源码方式
#   LINK_TYPE     - 链接类型：STATIC(静态) / SHARED(动态) / AUTO(使用全局BUILD_SHARED_LIBS)
# ============================================
macro(find_or_fetch_dependency DEP_NAME PACKAGE_NAME GIT_REPO GIT_TAG LOCAL_PATH LINK_TYPE)
    message(STATUS "正在处理依赖: ${DEP_NAME}")
    
    # 确定该库的链接类型
    if("${LINK_TYPE}" STREQUAL "STATIC")
        set(_LIB_IS_SHARED OFF)
        set(_LINK_TYPE_STR "静态")
    elseif("${LINK_TYPE}" STREQUAL "SHARED")
        set(_LIB_IS_SHARED ON)
        set(_LINK_TYPE_STR "动态")
    else() # AUTO 或其他值，使用全局设置
        set(_LIB_IS_SHARED ${BUILD_SHARED_LIBS})
        if(BUILD_SHARED_LIBS)
            set(_LINK_TYPE_STR "动态(全局)")
        else()
            set(_LINK_TYPE_STR "静态(全局)")
        endif()
    endif()
    
    # 首先尝试查找本地已安装的包
    find_package(${PACKAGE_NAME} QUIET)
    
    if(${PACKAGE_NAME}_FOUND OR ${DEP_NAME}_FOUND)
        message(STATUS "  ✓ 找到本地安装的 ${PACKAGE_NAME}")
        
        # 打印包的路径信息（尝试多种可能的变量）
        if(DEFINED ${PACKAGE_NAME}_DIR)
            message(STATUS "     路径: ${${PACKAGE_NAME}_DIR}")
        elseif(DEFINED ${DEP_NAME}_DIR)
            message(STATUS "     路径: ${${DEP_NAME}_DIR}")
        elseif(DEFINED ${PACKAGE_NAME}_ROOT)
            message(STATUS "     根目录: ${${PACKAGE_NAME}_ROOT}")
        elseif(DEFINED ${DEP_NAME}_ROOT)
            message(STATUS "     根目录: ${${DEP_NAME}_ROOT}")
        elseif(DEFINED ${PACKAGE_NAME}_INCLUDE_DIRS)
            message(STATUS "     头文件: ${${PACKAGE_NAME}_INCLUDE_DIRS}")
        elseif(DEFINED ${DEP_NAME}_INCLUDE_DIRS)
            message(STATUS "     头文件: ${${DEP_NAME}_INCLUDE_DIRS}")
        endif()
    else()
        message(STATUS "  ✗ 未找到本地安装，准备从源码构建 [${_LINK_TYPE_STR}]")
        
        # 配置库特定的编译选项（在add_subdirectory/FetchContent之前）
        # SDL系列库的特定选项
        if("${DEP_NAME}" MATCHES "^SDL" OR "${PACKAGE_NAME}" MATCHES "^SDL")
            if(_LIB_IS_SHARED)
                set(SDL_SHARED ON CACHE BOOL "" FORCE)
                set(SDL_STATIC OFF CACHE BOOL "" FORCE)
            else()
                set(SDL_SHARED OFF CACHE BOOL "" FORCE)
                set(SDL_STATIC ON CACHE BOOL "" FORCE)
            endif()
            # 禁用SDL测试
            set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
            set(SDL_TESTS OFF CACHE BOOL "" FORCE)
            set(SDL_INSTALL_TESTS OFF CACHE BOOL "" FORCE)
        endif()
        
        # 其他库的通用选项
        set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
        set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(BUILD_DOCS OFF CACHE BOOL "" FORCE)
        
        # SDL_image特定选项：禁用可能导致构建问题的格式支持
        if("${DEP_NAME}" STREQUAL "SDL3_image")
            # 禁用AVIF格式（需要NASM、Meson等复杂工具链）
            set(SDLIMAGE_AVIF OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SHARED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AVIF_SAVE OFF CACHE BOOL "" FORCE)
            # 禁用AVIF的依赖库
            set(SDLIMAGE_AVIF_VENDORED OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_DAV1D OFF CACHE BOOL "" FORCE)
            set(SDLIMAGE_AOM OFF CACHE BOOL "" FORCE)
            # 可选：禁用其他可能有构建问题的格式
            # set(SDLIMAGE_JXL OFF CACHE BOOL "" FORCE)  # JPEG XL
        endif()
        
        # 智能选择：优先本地源码，否则在线获取
        set(LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/${LOCAL_PATH})
        if(EXISTS ${LOCAL_SOURCE_DIR})
            # 检测到本地源码，使用本地编译
            message(STATUS "  → 使用本地源码: ${LOCAL_SOURCE_DIR}")
            
            # 临时设置BUILD_SHARED_LIBS影响该库
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)

            # add_subdirectory参数说明：
            # 1.源码目录（${LOCAL_SOURCE_DIR}），即依赖库的本地源码路径；
            # 2.二进制输出目录（${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build），即该依赖库的构建输出会放到这里，避免污染主项目的build目录。
            add_subdirectory(${LOCAL_SOURCE_DIR} ${CMAKE_BINARY_DIR}/_deps/${DEP_NAME}-build)
            
            # 恢复全局设置
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
            
            # 确保命名空间别名存在（兼容性处理）
            # 某些库从源码编译时可能不会自动创建带命名空间的ALIAS target
            # 这里统一创建 PACKAGE_NAME::PACKAGE_NAME 格式的别名
            if(TARGET ${DEP_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${DEP_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${DEP_NAME})
            elseif(TARGET ${PACKAGE_NAME} AND NOT TARGET ${PACKAGE_NAME}::${PACKAGE_NAME})
                message(STATUS "     创建别名: ${PACKAGE_NAME}::${PACKAGE_NAME} -> ${PACKAGE_NAME}")
                add_library(${PACKAGE_NAME}::${PACKAGE_NAME} ALIAS ${PACKAGE_NAME})
            endif()
        else()
            # 本地源码不存在，使用FetchContent在线获取
            message(STATUS "  → 本地源码不存在，在线获取: ${GIT_REPO}")
            
            # 智能检测GIT_TAG类型，决定是否使用浅克隆
            string(LENGTH "${GIT_TAG}" TAG_LENGTH)
            string(REGEX MATCH "^[0-9a-f]+$" IS_HEX "${GIT_TAG}")
            
            # 判断是否为commit hash（40位十六进制字符串）
            if(TAG_LENGTH EQUAL 40 AND IS_HEX)
                message(STATUS "     版本: commit hash [${GIT_TAG}]，完整克隆")
                set(USE_SHALLOW FALSE)
            else()
                message(STATUS "     版本: ${GIT_TAG}，浅克隆")
                set(USE_SHALLOW TRUE)
            endif()
            
            # FetchContent_Declare用于声明一个FetchContent对象，用于在线获取依赖库源码。
            # 参数说明：
            #   DEP_NAME       - 依赖的内部名称（用于FetchContent和add_subdirectory等）
            #   GIT_REPOSITORY - 依赖的Git仓库地址
            #   GIT_TAG        - 依赖的Git分支、标签或commit hash
            #   GIT_SHALLOW    - 是否使用浅克隆（TRUE/FALSE），根据GIT_TAG类型自动判断
            #   GIT_PROGRESS   - 是否显示Git进度（TRUE/FALSE）
            
            # 临时设置BUILD_SHARED_LIBS影响该库
            set(_SAVED_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
            set(BUILD_SHARED_LIBS ${_LIB_IS_SHARED} CACHE BOOL "" FORCE)
            
            FetchContent_Declare(
                ${DEP_NAME}
                GIT_REPOSITORY ${GIT_REPO}
                GIT_TAG ${GIT_TAG}
                GIT_SHALLOW ${USE_SHALLOW}
                GIT_PROGRESS TRUE
            )
            FetchContent_MakeAvailable(${DEP_NAME})
            
            # 恢复全局设置
            set(BUILD_SHARED_LIBS ${_SAVED_BUILD_SHARED_LIBS} CACHE BOOL "" FORCE)
        endif()
    endif()
endmacro()

# ============================================
# 设置所有项目依赖
# ============================================
function(setup_project_dependencies)
    # SDL3
    find_or_fetch_dependency(
        SDL3
        SDL3
        "https://github.com/libsdl-org/SDL.git"
        "release-3.2.24"
        "external/SDL-release-3.2.24"
        AUTO  # 使用全局BUILD_SHARED_LIBS设置
    )

    # SDL3_image
    # 注意：已自动禁用AVIF格式支持（需要NASM、Meson等复杂工具链）
    # 如需启用AVIF，请修改宏中的SDLIMAGE_AVIF选项，并安装所需工具：
    #   - NASM: https://www.nasm.us/
    #   - Meson: pip install meson ninja
    #   - Perl: https://strawberryperl.com/
    find_or_fetch_dependency(
        SDL3_image
        SDL3_image
        "https://github.com/libsdl-org/SDL_image.git"
        "release-3.2.4"
        "external/SDL_image-release-3.2.4"
        AUTO  # 使用全局BUILD_SHARED_LIBS设置
    )

    # SDL3_mixer
    find_or_fetch_dependency(
        SDL3_mixer
        SDL3_mixer
        "https://github.com/libsdl-org/SDL_mixer.git"
        "30c1301055a35ee87b8679279b6fc88e10d28fa3"
        "external/SDL_mixer-30c1301"
        AUTO  # 使用全局BUILD_SHARED_LIBS设置
    )

    # SDL3_ttf
    find_or_fetch_dependency(
        SDL3_ttf
        SDL3_ttf
        "https://github.com/libsdl-org/SDL_ttf.git"
        "release-3.2.2"
        "external/SDL_ttf-release-3.2.2"
        AUTO  # 使用全局BUILD_SHARED_LIBS设置
    )

    # GLM
    find_or_fetch_dependency(
        glm
        glm
        "https://github.com/g-truc/glm.git"
        "1.0.1"
        "external/glm-1.0.1"
        STATIC  # GLM通常使用静态链接，动态很可能出错
    )

    # nlohmann-json (header-only库)
    # 注意：FetchContent内部名称用json，但find_package用nlohmann_json
    find_or_fetch_dependency(
        json
        nlohmann_json
        "https://github.com/nlohmann/json.git"
        "v3.12.0"
        "external/json-3.12.0"
        STATIC  # header-only库，实际不影响
    )

    # spdlog
    find_or_fetch_dependency(
        spdlog
        spdlog
        "https://github.com/gabime/spdlog.git"
        "v1.15.3"
        "external/spdlog-1.15.3"
        STATIC  # 推荐静态链接，避免运行时依赖
    )
    
endfunction()

