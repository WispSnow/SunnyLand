# ============================================
# 运行时动态库搜索路径配置模块
# ============================================
# 功能：解决运行时找不到动态库的问题

# 设置RPATH（macOS和Linux）
# RPATH是嵌入到可执行文件中的运行时库搜索路径
if(APPLE)
    # macOS使用@executable_path相对路径
    set(CMAKE_INSTALL_RPATH "@executable_path;@executable_path/lib;@executable_path/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib;${CMAKE_SOURCE_DIR}/prebuilt/lib")
elseif(UNIX)
    # Linux使用$ORIGIN相对路径
    set(CMAKE_INSTALL_RPATH "$ORIGIN:$ORIGIN/lib:$ORIGIN/../lib")
    set(CMAKE_BUILD_RPATH "${CMAKE_SOURCE_DIR}/lib:${CMAKE_SOURCE_DIR}/prebuilt/lib")
endif()

# 构建时使用RPATH，不需要设置LD_LIBRARY_PATH就能运行
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# macOS额外配置
if(APPLE)
    set(CMAKE_MACOSX_RPATH TRUE)
endif()

