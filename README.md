[English](README.md) | [简体中文](README-ZN.md)

# SunnyLand
**SunnyLand** is a cross-platform platformer game developed in C++ with SDL3, glm, nlohmann-json and Tiled.

> This project is a teaching demonstration project; it is the 3rd episode in a series of tutorials titled "[C++ 游戏开发之旅](https://cppgamedev.top/)".

## Control
```
A,D / left,right - to move;
W,S / up,down - to climb;
J / Space - to jump;
P / ESC - show menu and pause;
```

## Play on Webpage
[SunnyLand](https://wispsnow.github.io/SunnyLand/)

## ScreenShot
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_1.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_2.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_3.png" style='width: 600px;'/>
<img src="https://theorhythm.top/gamedev/SL/SL_screenshot_4.png" style='width: 600px;'/>

## Third-party libraries
* [SDL3](https://github.com/libsdl-org/SDL)
* [SDL3_image](https://github.com/libsdl-org/SDL_image)
* [SDL3_mixer](https://github.com/libsdl-org/SDL_mixer)
* [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf)
* [glm](https://github.com/g-truc/glm)
* [nlohmann-json](https://github.com/nlohmann/json)
* [spdlog](https://github.com/gabime/spdlog)

## How to build
Dependencies will be automatically downloaded by Git FetchContent to make building quite easy:
```bash
git clone https://github.com/WispSnow/SunnyLand.git
cd SunnyLand
cmake -S . -B build
cmake --build build
```

If you encounter trouble downloading from GitHub (especially on networks in mainland China), please refer to the [wiki](../../wiki) for an alternative building guide.

# Credits
- sprite
    - https://ansimuz.itch.io/sunny-land-pixel-game-art
- FX
    - https://ansimuz.itch.io/sunny-land-pixel-game-art
- font
    - https://timothyqiu.itch.io/vonwaon-bitmap
- UI
    - https://markiro.itch.io/hud-asset-pack
    - https://bdragon1727.itch.io/platformer-ui-buttons
- sound
    - https://taira-komori.jpn.org/
    - https://pixabay.com/sound-effects/dead-8bit-41400/
    - https://pixabay.com/sound-effects/cartoon-jump-6462/
    - https://pixabay.com/zh/sound-effects/frog-quak-81741/
    - https://mmvpm.itch.io/platformer-sound-fx-pack
    - https://kasse.itch.io/ui-buttons-sound-effects-pack
- music
    - https://ansimuz.itch.io/sunny-land-pixel-game-art

- Sponsors: `sino`, `李同学` 

## Contact

For support or feedback, please contact us through the GitHub issues section of this repository. Your feedback is crucial for making this series of tutorials better!

## Buy Me a Coffee
[!["Buy Me A Coffee"](https://storage.ko-fi.com/cdn/kofi2.png?v=3)](https://ko-fi.com/ziyugamedev)
[!["Support me on Afdian"](https://pic1.afdiancdn.com/static/img/welcome/button-sponsorme.png)](https://afdian.com/a/ziyugamedev)



## QQ Discussion Group and My WeChat QR Code

<div style="display: flex; gap: 10px;">
  <img src="https://theorhythm.top/personal/qq_group.webp" width="200" />
  <img src="https://theorhythm.top/personal/wechat_qr.webp" width="200" />
</div>