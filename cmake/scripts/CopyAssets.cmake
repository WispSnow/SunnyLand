# ============================================
# 资源文件复制脚本
# ============================================
# 此脚本在构建时执行，用于复制资源文件到可执行文件目录
# 使用方式：cmake -DSOURCE_DIR=... -DTARGET_DIR=... -P CopyAssets.cmake

# 检查必需参数
if(NOT DEFINED SOURCE_DIR OR NOT DEFINED TARGET_DIR)
    message(FATAL_ERROR "Required parameters: SOURCE_DIR and TARGET_DIR")
endif()

# 计算源文件夹大小
file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*")
set(SOURCE_SIZE 0)
foreach(FILE IN LISTS SOURCE_FILES)
    if(EXISTS "${FILE}")
        file(SIZE "${FILE}" FILE_SIZE)
        math(EXPR SOURCE_SIZE "${SOURCE_SIZE} + ${FILE_SIZE}")
    endif()
endforeach()

# 计算目标文件夹大小
set(TARGET_SIZE 0)
if(EXISTS "${TARGET_DIR}")
    file(GLOB_RECURSE TARGET_FILES "${TARGET_DIR}/*")
    foreach(FILE IN LISTS TARGET_FILES)
        if(EXISTS "${FILE}")
            file(SIZE "${FILE}" FILE_SIZE)
            math(EXPR TARGET_SIZE "${TARGET_SIZE} + ${FILE_SIZE}")
        endif()
    endforeach()
endif()

# 比较大小，只在不同时复制
if(NOT SOURCE_SIZE EQUAL TARGET_SIZE)
    file(COPY "${SOURCE_DIR}/" DESTINATION "${TARGET_DIR}")
    message(STATUS "Update assets folder (size: ${SOURCE_SIZE} bytes)")
else()
    # message(STATUS "资源文件已是最新，跳过复制")
endif()

# 复制 imgui.ini（如果指定且存在）
if(DEFINED IMGUI_INI_SOURCE AND EXISTS "${IMGUI_INI_SOURCE}")
    if(NOT EXISTS "${TARGET_DIR}/imgui.ini")
        file(COPY "${IMGUI_INI_SOURCE}" DESTINATION "${TARGET_DIR}")
        message(STATUS "Copy imgui.ini to executable directory")
    endif()
endif()

