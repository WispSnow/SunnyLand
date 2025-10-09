# SunnyLand
**SunnyLand** is a cross-platfrom platformer game developed in C++ with SDL3, glm, nlohmann-json and Tiled.

## Control
```
A,D / left,right - to move;
W,S / up,down - to climb;
J / Space - to jump;
P / ESC - show menu and pause;
```

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

