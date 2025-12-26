[English](README.md) | [简体中文](README-ZN.md)

# SunnyLand
**SunnyLand** 是一个使用 SDL3, glm, nlohmann-json 和 Tiled 开发的 C++ 跨平台平台跳跃游戏。

> 本项目是一个教学演示项目；它是系列教程 "[C++ 游戏开发之旅](https://cppgamedev.top/)" 的第 3 集。

## 控制
```
A,D / 左,右 - 移动;
W,S / 上,下 - 攀爬;
J / 空格 - 跳跃;
P / ESC - 显示菜单和暂停;
```

## 在网页上游玩
[SunnyLand](https://wispsnow.github.io/SunnyLand/)

## 截图
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_1.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_2.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_3.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_4.png" style='width: 600px;'/>

## 第三方库
* [SDL3](https://github.com/libsdl-org/SDL)
* [SDL3_image](https://github.com/libsdl-org/SDL_image)
* [SDL3_mixer](https://github.com/libsdl-org/SDL_mixer)
* [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf)
* [glm](https://github.com/g-truc/glm)
* [nlohmann-json](https://github.com/nlohmann/json)
* [spdlog](https://github.com/gabime/spdlog)

## 如何构建
依赖项将由 Git FetchContent 自动下载，使构建非常容易：
```bash
git clone https://github.com/WispSnow/SunnyLand.git
cd SunnyLand
cmake -S . -B build
cmake --build build
```

如果你在从 GitHub 下载时遇到问题（尤其是在中国大陆网络环境下），请参考 [wiki](../../wiki) 获取替代构建指南。

# 致谢
- 精灵
    - https://ansimuz.itch.io/sunny-land-pixel-game-art
- 特效
    - https://ansimuz.itch.io/sunny-land-pixel-game-art
- 字体
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

- 赞助者: `sino`, `李同学` 

## 联系方式

若需支持或反馈，请通过本仓库的 GitHub issues 板块联系我们。您的反馈对改进本系列教程至关重要！

## 请我喝咖啡
[!["Buy Me A Coffee"](https://storage.ko-fi.com/cdn/kofi2.png?v=3)](https://ko-fi.com/ziyugamedev)
[!["Support me on Afdian"](https://pic1.afdiancdn.com/static/img/welcome/button-sponsorme.png)](https://afdian.com/a/ziyugamedev)


## QQ 交流群和我的微信二维码

<div style="display: flex; gap: 10px;">
  <img src="https://theorhythm.top/personal/qq_group.webp" width="200" />
  <img src="https://theorhythm.top/personal/wechat_qr.webp" width="200" />
</div>
