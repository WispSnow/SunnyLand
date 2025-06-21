#pragma once
#include "../component/component.h" 
#include <string_view>
#include <memory>
#include <unordered_map>
#include <typeindex>        // 用于类型索引
#include <utility>          // 用于完美转发
#include <spdlog/spdlog.h>

namespace engine::core {
    class Context;
}

namespace engine::object {

/**
 * @brief 游戏对象类，负责管理游戏对象的组件。
 * 
 * 该类管理游戏对象的组件，并提供添加、获取、检查和移除组件的功能。
 * 它还提供更新和渲染游戏对象的方法。
 */
class GameObject final {
private:
    std::string name_;          ///< @brief 名称
    std::string tag_;           ///< @brief 标签
    std::unordered_map<std::type_index, std::unique_ptr<engine::component::Component>> components_;  ///< @brief 组件列表
    bool need_remove_ = false;  ///< @brief 延迟删除的标识，将来由场景类负责删除

public:

    GameObject(std::string_view name = "", std::string_view tag = "");  ///< @brief 构造函数。默认名称为空，标签为空

    // 禁止拷贝和移动，确保唯一性 (通常游戏对象不应随意拷贝)
    GameObject(const GameObject&) = delete;
    GameObject& operator=(const GameObject&) = delete;
    GameObject(GameObject&&) = delete;
    GameObject& operator=(GameObject&&) = delete;

    // setters and getters
    void setName(std::string_view name) { name_ = name; }                 ///< @brief 设置名称
    std::string_view getName() const { return name_; }                    ///< @brief 获取名称
    void setTag(std::string_view tag) { tag_ = tag; }                     ///< @brief 设置标签
    std::string_view getTag() const { return tag_; }                      ///< @brief 获取标签
    void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }    ///< @brief 设置是否需要删除
    bool isNeedRemove() const { return need_remove_; }                      ///< @brief 获取是否需要删除

    /**
     * @brief 添加组件 (里面会完成组件的init())
     * 
     * @tparam T 组件类型
     * @tparam Args 组件构造函数参数类型
     * @param args 组件构造函数参数
     * @return 组件指针
     */
    template <typename T, typename... Args>
    T* addComponent(Args&&... args) {
        // 检测组件是否合法。  /*  static_assert(condition, message)：静态断言，在编译期检测，无任何性能影响 */
                            /* std::is_base_of<Base, Derived>::value -- 判断 Base 类型是否是 Derived 类型的基类 */
        static_assert(std::is_base_of<engine::component::Component, T>::value, "T 必须继承自 Component");
        // 获取类型标识。     /* typeid(T) -- 用于获取一个表达式或类型的运行时类型信息 (RTTI), 返回 std::type_info& */
                            /* std::type_index -- 针对std::type_info对象的包装器，主要设计用来作为关联容器（如 std::map）的键。*/
        auto type_index = std::type_index(typeid(T));
        // 如果组件已经存在，则直接返回组件指针
        if (hasComponent<T>()) {
            return getComponent<T>();
        }
        // 如果不存在则创建组件     /* std::forward -- 用于实现完美转发。传递多个参数的时候使用...标识 */
        auto new_component = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = new_component.get();                               // 先获取裸指针以便返回
        new_component->setOwner(this);                              // 设置组件的拥有者
        components_[type_index] = std::move(new_component);         // 移动组件   （new_component 变为空，不可再使用）
        ptr->init();                                                // 初始化组件 （因此必须用ptr而不能用new_component）
        spdlog::debug("GameObject::addComponent: {} added component {}", name_, typeid(T).name());
        return ptr;                                                 // 返回非拥有指针
    }

    /**
     * @brief 获取组件
     * 
     * @tparam T 组件类型
     * @return 组件指针
     */
    template <typename T>
    T* getComponent() const {
        static_assert(std::is_base_of<engine::component::Component, T>::value, "T 必须继承自 Component");
        auto type_index = std::type_index(typeid(T));
        auto it = components_.find(type_index);
        if (it != components_.end()) {
            // 返回unique_ptr的裸指针。(肯定是T类型, static_cast其实并无必要，但保留可以使我们意图更清晰)
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    /**
     * @brief 检查是否存在组件
     * 
     * @tparam T 组件类型
     * @return 是否存在组件
     */
    template <typename T>
    bool hasComponent() const {
        static_assert(std::is_base_of<engine::component::Component, T>::value, "T 必须继承自 Component");
        // contains方法为 C++20 新增
        return components_.contains(std::type_index(typeid(T)));
    }

    /**
     * @brief 移除组件
     * 
     * @tparam T 组件类型
     */
    template <typename T>
    void removeComponent() {
        static_assert(std::is_base_of<engine::component::Component, T>::value, "T 必须继承自 Component");
        auto type_index = std::type_index(typeid(T));
        auto it = components_.find(type_index);
        if (it != components_.end()) {
            it->second->clean();
            components_.erase(it);
        }
    }

    // 关键循环函数
    void update(float delta_time, engine::core::Context& context);              ///< @brief 更新所有组件
    void render(engine::core::Context& context);                                ///< @brief 渲染所有组件
    void clean();                                                               ///< @brief 清理所有组件
    void handleInput(engine::core::Context& context);                           ///< @brief 处理输入

};

} // namespace engine::object

