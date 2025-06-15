#include "context.h"
#include "../input/input_manager.h"
#include "../render/renderer.h"
#include "../render/camera.h"
#include "../render/text_renderer.h"
#include "../resource/resource_manager.h"
#include "../physics/physics_engine.h"
#include "../audio/audio_player.h"
#include <spdlog/spdlog.h>

namespace engine::core {

Context::Context(engine::input::InputManager& input_manager, 
                 engine::render::Renderer& renderer,
                 engine::render::Camera& camera,
                 engine::render::TextRenderer& text_renderer,
                 engine::resource::ResourceManager& resource_manager,
                 engine::physics::PhysicsEngine& physics_engine,
                 engine::audio::AudioPlayer& audio_player,
                 engine::core::GameState& game_state)     
    : input_manager_(input_manager),
      renderer_(renderer),
      camera_(camera),
      text_renderer_(text_renderer),
      resource_manager_(resource_manager),
      physics_engine_(physics_engine),
      audio_player_(audio_player),
      game_state_(game_state)
{
    spdlog::trace("上下文已创建并初始化。");
}

} // namespace engine::core 