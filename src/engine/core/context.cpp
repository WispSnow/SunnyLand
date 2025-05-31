#include "context.h"
#include "../input/input_manager.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "../resource/resource_manager.h"
#include "../physics/physics_engine.h"
#include <spdlog/spdlog.h>

namespace engine::core {

Context::Context(engine::input::InputManager& input_manager, 
                 engine::render::Renderer& renderer,
                 engine::render::Camera& camera,
                 engine::resource::ResourceManager& resource_manager,
                 engine::physics::PhysicsEngine& physics_engine)
    : input_manager_(input_manager),
      renderer_(renderer),
      camera_(camera),
      resource_manager_(resource_manager),
      physics_engine_(physics_engine)
{
    spdlog::trace("上下文已创建并初始化，包含输入管理器、渲染器、相机和资源管理器。");
}

} // namespace engine::core 