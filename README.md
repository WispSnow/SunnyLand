# SunnyLand
利用SDL3、glm、Tiled、nlohmann_json、spdlog开发一款平台跳跃类型的游戏

([C++游戏开发之旅](https://pan.baidu.com/s/1xGGgZbhE6WuSPzlKuTmdMw?pwd=wd9g) 第三期)

## 项目简介
此项目为教学演示用途，配套教程：[视频](https://www.bilibili.com/video/BV1u7NizLEBa/)

>因网络时常不稳定，环境安装包及代码均可从[百度网盘](https://pan.baidu.com/s/1pmz0GCXpDr2d79ieTXkt5g?pwd=23n8)中下载。如果使用`Library.rar`，则只需要配置环境变量即可（代码版本可能落后于Github仓库）

提示：仓库中包含了所有课节的代码，你可以根据视频教程的进度查找对应的代码。<img src="https://theorhythm.top/gamedev/tags.png" style='width: 400px;' />
<img src="https://theorhythm.top/gamedev/download.png" style='width: 800px;' />

或者使用Git更加方便地查看代码，参考此教程：[利用Git快速查看课节代码](https://www.bilibili.com/video/BV1H7KGerEAa/)

本项目为CC0 1.0许可，这意味着你可以随意使用此代码且无需署名，但如果你给出Github链接或者视频教程的链接，我会非常感激。

## 游戏截图
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_1.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_2.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_3.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_4.png" style='width: 600px;'/>

## 环境配置
请根据你自己的平台（Linux、MacOS、Windows）参考各自的配置方法。
<img src="https://theorhythm.top/gamedev/SL/环境配置表.png" />
其中C++编译器、VS Code、CMake、SDL3、glm的安装方法及相关配置已经在上一期教程中说明，请参见 [Github仓库](https://github.com/WispSnow/GhostEscape) 或 视频演示 [环境安装](https://www.bilibili.com/video/BV1J2RpYGEmj/)

此处补充 `nlohmann_json`、`spdlog` 安装方法（请确保其它组件成功安装后再安装）：
### Ubuntu Linux
1. 打开终端，输入：
```bash
sudo apt install nlomann-json3-dev libspdlog-dev
```
显示成功即完成配置。

### MacOS
1. 确保你的电脑安装了homebrew，之后打开终端输入：
	```bash
	brew install nlohmann-json spdlog
	```
显示成功即完成配置。

### Windows
`nlohmann_json`、`spdlog`的安装方法和上一期介绍的`glm`库安装方法完全一样，简要流程如下：
1. 打开Github仓库（[nlohmann_json](https://github.com/nlohmann/json)、[spdlog](https://github.com/gabime/spdlog)）并下载源代码。
2. 解压后用VS Code打开源码文件夹，并按照提示执行CMake配置，配置完毕后执行“生成”。
3. 等待生成结束后，打开左侧资源管理器中的build文件夹，点开“cmake_install.cmake”文件，修改其中的 `CMAKE_INSTALL_PREFIX` 参数为你想要安装的位置。
4. 保存文件后打开“命令”面板（ctrl+shift+p），执行“Cmake 安装”。 
5. 成功后即可在目标地址看到对应库文件夹
> 如果下载或安装困难，也可以尝试我自己编译好的库文件：[百度网盘](https://pan.baidu.com/s/1pmz0GCXpDr2d79ieTXkt5g?pwd=23n8)，版本与教程相同
6. 设置环境变量，将上一步得到的`文件夹路径` 添加到Path变量中。
7. (可选): `spdlog` 的debug和release模式需要分别编译安装，默认是debug模式，如果要支持release，在debug安装完成之后，改成release重新安装一次即可（即一共安装两次）
<img src="https://theorhythm.top/gamedev/SL/spdlog-release.png" style='width: 600px;'/>
至此完成配置。

## CMakeLists.txt 补充
要添加这两个库的支持，只需要在上期课程的`CmakeLists.txt`代码中添加 
```
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
``` 
和 
```
nlohmann_json::nlohmann_json
spdlog::spdlog
``` 
即可。 

<img src="https://theorhythm.top/gamedev/SL/cmake添加.png" style='width: 800px;' />

## Tiled
在游戏开发中我们还会用到Tiled地图编辑器，直接从官网[下载](http://www.mapeditor.org/)安装即可。

# 素材资源
- 精灵图
    - https://ansimuz.itch.io/sunny-land-pixel-game-art
- 特效
    - https://ansimuz.itch.io/sunny-land-pixel-game-art
- 文字
    - https://timothyqiu.itch.io/vonwaon-bitmap
- UI
    - https://markiro.itch.io/hud-asset-pack
    - https://bdragon1727.itch.io/platformer-ui-buttons
- 音效
    - https://taira-komori.jpn.org/
    - https://pixabay.com/sound-effects/dead-8bit-41400/
    - https://pixabay.com/sound-effects/cartoon-jump-6462/
    - https://pixabay.com/zh/sound-effects/frog-quak-81741/
    - https://mmvpm.itch.io/platformer-sound-fx-pack
    - https://kasse.itch.io/ui-buttons-sound-effects-pack
- 音乐
    - https://ansimuz.itch.io/sunny-land-pixel-game-art

(直接下载 [打包资源](https://pan.baidu.com/s/1pmz0GCXpDr2d79ieTXkt5g?pwd=23n8)） 

# 致谢
感谢 `Sino`、`李同学` 对本项目的赞助支持

感谢`简直天真`、`VenomKojima`、`蒸爽先生`、`Wtzrpo`、`哥布林成为骑士`、`Little_Etx` 对教程中错漏部分的指正与建议