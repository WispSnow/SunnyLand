# CMake 模块说明

本目录包含MonsterWar项目的模块化CMake配置文件，将原本庞大的CMakeLists.txt拆分为多个功能独立的模块。

## 📁 模块列表

### 1. CompilerSettings.cmake
**功能**：编译器配置
- 设置C++标准（C++20）
- 配置编译选项和警告级别
- 处理不同平台的字符编码（UTF-8）
- Windows子系统设置

**主要函数**：
- `setup_compiler_options(TARGET_NAME)` - 为指定目标设置编译选项

---

### 2. RuntimePath.cmake
**功能**：运行时动态库搜索路径配置
- 配置macOS的RPATH（@executable_path）
- 配置Linux的RPATH（$ORIGIN）
- 解决运行时找不到动态库的问题

**适用场景**：
- 使用动态链接库时
- 需要分发可执行文件时

---

### 3. Dependencies.cmake
**功能**：智能依赖管理
- 自动查找或获取项目依赖
- 支持系统库、本地源码、在线获取三种方式
- 智能选择静态/动态链接
- 自动处理浅克隆优化

**主要宏**：
- `find_or_fetch_dependency()` - 智能依赖获取宏

**主要函数**：
- `setup_project_dependencies()` - 配置所有项目依赖（SDL3、GLM、spdlog等）

**依赖获取优先级**：
1. 系统已安装的包（find_package）
2. 本地external/目录的源码
3. 在线从GitHub获取

---

### 4. ImGui.cmake
**功能**：ImGui库配置
- 定义ImGui源文件列表
- 配置ImGui头文件搜索路径
- 集成SDL3后端

**使用**：
- 自动将`IMGUI_SOURCES`变量导出供主CMakeLists.txt使用

---

### 5. BuildHelpers.cmake
**功能**：构建辅助工具
- 资源文件自动复制
- Windows DLL自动检测和复制
- 智能增量更新（仅复制变更内容）

**主要函数**：
- `setup_asset_copy(TARGET_NAME)` - 配置资源文件复制
- `setup_windows_dll_copy(TARGET_NAME)` - 配置Windows DLL复制

**特点**：
- 使用独立的脚本模块（`scripts/`目录）
- 基于文件大小/MD5的智能比对
- 避免不必要的文件复制
- 减少编译时间
- 脚本可单独测试和调试

**相关文件**：
- `scripts/CopyAssets.cmake` - 资源复制脚本
- `scripts/CopyDLLs.cmake` - DLL复制脚本

---

### 6. ProjectInfo.cmake
**功能**：项目信息打印
- 在配置完成后打印项目配置摘要
- 显示编译选项、输出目录、链接方式等信息

**主要函数**：
- `print_project_info(TARGET_NAME)` - 打印项目配置信息

---

## 🚀 使用方法

在主`CMakeLists.txt`中引入这些模块：

```cmake
# 引入编译器设置
include(cmake/CompilerSettings.cmake)

# 引入运行时路径配置
include(cmake/RuntimePath.cmake)

# 引入依赖管理
include(cmake/Dependencies.cmake)

# 引入ImGui配置
include(cmake/ImGui.cmake)

# 引入构建辅助函数
include(cmake/BuildHelpers.cmake)

# 引入项目信息打印
include(cmake/ProjectInfo.cmake)

# 调用函数
setup_project_dependencies()
setup_compiler_options(${TARGET})
setup_asset_copy(${TARGET})
setup_windows_dll_copy(${TARGET})
print_project_info(${TARGET})
```

## 🔧 自定义依赖

如需添加新的依赖库，编辑`Dependencies.cmake`中的`setup_project_dependencies()`函数：

```cmake
# 添加新依赖
find_or_fetch_dependency(
    库内部名称
    包名称
    "Git仓库地址"
    "版本标签"
    "本地路径"
    链接类型  # STATIC/SHARED/AUTO
)
```

## 📝 优势

✅ **模块化**：各功能独立，易于维护
✅ **可复用**：模块可以在其他项目中复用
✅ **清晰**：主CMakeLists.txt更简洁，只关注项目本身
✅ **灵活**：可以根据需要选择性引入模块
✅ **易扩展**：新增功能只需添加新模块，不影响现有代码

## 🎯 最佳实践

1. **不要直接修改主CMakeLists.txt的底层逻辑**，而是修改对应模块
2. **新增通用功能**时，考虑创建新模块而非堆砌在主文件中
3. **保持模块单一职责**，一个模块只负责一类功能
4. **使用函数封装**，避免全局变量污染

