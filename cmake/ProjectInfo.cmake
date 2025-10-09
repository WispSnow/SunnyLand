# ============================================
# 项目信息打印模块
# ============================================
# 功能：打印项目配置信息

# ============================================
# 打印项目配置完成信息
# 用法：print_project_info(目标名称)
# ============================================
function(print_project_info TARGET_NAME)
    message(STATUS "")
    message(STATUS "==============================================")
    message(STATUS "项目配置完成！")
    message(STATUS "  目标名称: ${TARGET_NAME}")
    message(STATUS "  C++标准: C++${CMAKE_CXX_STANDARD}")
    
    # 显示可执行文件输出目录
    if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
        message(STATUS "  可执行文件输出: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    else()
        # 在多配置生成器（如Visual Studio、Xcode）中，exe会在配置子目录中
        if(CMAKE_CONFIGURATION_TYPES)
            message(STATUS "  可执行文件输出: ${CMAKE_BINARY_DIR}/<配置名称> (如Debug/Release)")
        else()
            message(STATUS "  可执行文件输出: ${CMAKE_BINARY_DIR}")
        endif()
    endif()
    
    message(STATUS "  智能依赖获取: 预编译(prebuilt目录) > 系统库 > 本地源码(external目录) > 在线获取")
    
    if(BUILD_SHARED_LIBS)
        message(STATUS "  依赖库默认链接: 动态链接 (Shared)")
    else()
        message(STATUS "  依赖库默认链接: 静态链接 (Static)")
    endif()
    
    if(WIN32)
        message(STATUS "  运行时库处理: 自动检测并复制所有DLL到exe目录")
        if(MSVC)
            message(STATUS "  字符编码: UTF-8 (/utf-8) - 避免中文乱码")
        endif()
    elseif(APPLE)
        message(STATUS "  运行时库处理: 使用RPATH (@executable_path)")
    elseif(UNIX)
        message(STATUS "  运行时库处理: 使用RPATH ($ORIGIN)")
    endif()
    
    message(STATUS "==============================================")
    message(STATUS "")
endfunction()

