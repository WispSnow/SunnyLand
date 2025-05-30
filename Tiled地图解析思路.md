# Tiled地图解析思路
- 获取tilesets数组并载入图块集的数据

- 获取layers数组并遍历对象
  - 如果 type : imagelayer，载入单一图片
    - 关注"parallax","repeat","offset"字段
    - 创建包含 ParallaxComponent 的游戏对象
    
  - 如果 type : tilelayer，载入瓦片层
    - 总共有“地图大小”个瓦片，放入容器vector中
    - 每个瓦片包含数据：Sprite，Type(例如solid类型)
    - 可能引用多个图块集，因此可先*载入并保存每个图块集的数据*（载入函数），其它瓦片层(以及对象层)也能继续引用
    - 通过data数组中的gid查找所需信息（查找函数），填充瓦片vector
    - 创建包含 TileLayerComponent 的游戏对象(持有瓦片vector)
    
  - 如果 type : objectgroup ……