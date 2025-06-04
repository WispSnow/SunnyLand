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
    - 通过data数组中的gid查找所需信息（查找函数，能获取自定义属性），填充瓦片vector
    - 创建包含 TileLayerComponent 的游戏对象(持有瓦片vector)
    
  - 如果 type : objectgroup 载入对象层
    - 对象数据在"objects"字段中，每个对象都创建一个游戏对象
    - 关注的要点：gid查找**源图片信息**，layer数据中确认目标位置（TransformComponent）信息
      - 源图片信息：如果是"solid"类型，则 为静止障碍物，碰撞盒大小为整个图片大小
      - 源图片信息：如果非"solid"类型，则 
        - 碰撞盒大小、偏移: 获取 "objectgroup" --> 获取 "objects" --> "x","y","width","height"
        - 获取其他指定自定义属性（模版函数），进行必要设置/组件添加。（例如tag、gravity）
    - 如果没有gid，则代表自己绘制的形状（可能是碰撞盒、触发器等，未来按需处理）

- 自定义属性的获取方法：图块集中 获取 "tiles" --> 获取 "properties"

- **对象层物体特别注意:** 信息分散在两个区块中：
  - 地图tmj中：           绘制区域、名字 (及针对当前地图添加的自定义属性)
  - gid对应的图块集tsj中:  源图片信息    (画在任何地图上都会有的属性)