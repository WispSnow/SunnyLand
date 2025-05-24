#ifndef ENGINE_UTILS_ALIGNMENT_H
#define ENGINE_UTILS_ALIGNMENT_H

namespace engine::utils {

/**
 * @enum class Alignment
 * @brief 定义对象或组件相对于其位置参考点的对齐方式。
 */
enum class Alignment {
    NONE,           // 不指定对齐方式，偏移量通常为 (0,0) 或手动设置
    TOP_LEFT,       // 左上角
    TOP_CENTER,     // 顶部中心
    TOP_RIGHT,      // 右上角
    CENTER_LEFT,    // 中心左侧
    CENTER,         // 正中心 (几何中心)
    CENTER_RIGHT,   // 中心右侧
    BOTTOM_LEFT,    // 左下角
    BOTTOM_CENTER,  // 底部中心
    BOTTOM_RIGHT    // 右下角
};

} // namespace engine::utils

#endif // ENGINE_UTILS_ALIGNMENT_H 