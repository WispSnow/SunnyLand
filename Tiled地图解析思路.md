# Tiled地图解析思路
- 获取layers数组并遍历对象
  - 如果 type : imagelayer，载入单一图片
    - 关注"parallax","repeat","offset"字段
    - 创建包含 ParallaxComponent 的游戏对象
  - 如果 type : tilelayer ……
  - 如果 type : objectgroup ……