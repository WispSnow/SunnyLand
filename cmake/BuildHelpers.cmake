# ============================================
# 构建辅助函数模块
# ============================================
# 功能：资源复制、DLL复制等构建辅助功能

# ============================================
# 配置资源文件复制
# 用法：setup_asset_copy(目标名称)
# ============================================
function(setup_asset_copy TARGET_NAME)
    # 使用独立的脚本模块，而非运行时生成
    set(COPY_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/scripts/CopyAssets.cmake)
    
    add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DSOURCE_DIR=${CMAKE_SOURCE_DIR}/assets
            -DTARGET_DIR=$<TARGET_FILE_DIR:${TARGET_NAME}>/assets
            -DIMGUI_INI_SOURCE=${CMAKE_SOURCE_DIR}/imgui.ini
            -P ${COPY_SCRIPT}
        COMMENT "Check and copy asset files"
        VERBATIM
    )
endfunction()

# ============================================
# 配置Windows DLL复制
# 用法：setup_windows_dll_copy(目标名称)
# ============================================
function(setup_windows_dll_copy TARGET_NAME)
    if(NOT WIN32)
        return()
    endif()
    
    message(STATUS "配置Windows DLL自动检测和复制...")
    
    # 1. 复制prebuilt目录中的预编译DLL（如果存在）
    if(EXISTS ${CMAKE_SOURCE_DIR}/prebuilt/bin)
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/prebuilt/bin $<TARGET_FILE_DIR:${TARGET_NAME}>
            COMMENT "Copy prebuilt DLLs to executable directory"
            VERBATIM
        )
    endif()
    
    # 2. 自动检测并复制所有运行时DLL依赖（CMake 3.21+）
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        # 使用独立的脚本模块，但DLL_LIST必须用file(GENERATE)展开
        # 因为$<TARGET_RUNTIME_DLLS>只能在生成阶段解析
        set(COPY_SCRIPT ${CMAKE_SOURCE_DIR}/cmake/scripts/CopyDLLs.cmake)
        
        # 生成包装脚本 - 直接include而非execute_process，避免命令行参数传递问题
        file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/copy_dlls_wrapper_$<CONFIG>.cmake CONTENT "
# DLL复制包装脚本 - 用于展开生成器表达式
set(DLL_LIST \"$<TARGET_RUNTIME_DLLS:${TARGET_NAME}>\")
set(TARGET_DIR \"$<TARGET_FILE_DIR:${TARGET_NAME}>\")

# 直接包含脚本文件，避免通过命令行参数传递列表
include(\"${COPY_SCRIPT}\")
")
        
        add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/copy_dlls_wrapper_$<CONFIG>.cmake
            COMMENT "Auto-detect and copy runtime DLLs"
            VERBATIM
        )
    else()
        message(WARNING "CMake版本 < 3.21，无法自动复制运行时DLL。")
        message(WARNING "如果使用了动态库，请手动复制DLL到exe目录。")
    endif()
endfunction()

