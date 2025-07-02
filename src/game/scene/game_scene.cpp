#include "game_scene.h"
#include "menu_scene.h"
#include "end_scene.h"
#include "../component/player_component.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/object/game_object.h"
#include "../../engine/component/transform_component.h"
#include "../../engine/component/sprite_component.h"
#include "../../engine/component/physics_component.h"
#include "../../engine/component/collider_component.h"
#include "../../engine/component/tilelayer_component.h"
#include "../../engine/component/animation_component.h"
#include "../../engine/component/health_component.h"
#include "../../engine/physics/physics_engine.h"
#include "../../engine/scene/level_loader.h"
#include "../../engine/scene/scene_manager.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/render/camera.h"
#include "../../engine/render/animation.h"
#include "../../engine/render/text_renderer.h"
#include "../../engine/audio/audio_player.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_panel.h"
#include "../../engine/ui/ui_label.h"
#include "../../engine/ui/ui_image.h"
#include "../../engine/utils/math.h"
#include "../component/ai_component.h"
#include "../component/command/up_command.h"
#include "../component/command/down_command.h"
#include "../component/command/right_command.h"
#include "../component/command/left_command.h"
#include "../component/command/jump_command.h"
#include "../data/session_data.h"
#include "../object/object_builder_sl.h"
#include <spdlog/spdlog.h>

namespace game::scene {

GameScene::~GameScene() = default;

GameScene::GameScene(engine::core::Context& context, 
                     engine::scene::SceneManager& scene_manager,
                     std::shared_ptr<game::data::SessionData> data)
    : Scene("GameScene", context, scene_manager), game_session_data_(std::move(data)) {
    if (!game_session_data_) {      // 如果没有传入SessionData，则创建一个默认的
        game_session_data_ = std::make_shared<game::data::SessionData>();
        spdlog::info("未提供 SessionData，使用默认值。");
    }
    // 这里不再需要清空SessionData中的观察者，Observer在析构时会自动解除联系    
    spdlog::trace("GameScene 构造完成。");
}

void GameScene::onNotify(const engine::interface::EventType event, const std::any&) {
    /* NOTE: 如果我们封装HealthUI为新的类，那么GameScene就不需要设为Observer了
             而是在HealthUI中实现Observer接口（进一步解耦）
             这里我们简化，直接在GameScene中处理    */
    if (event == engine::interface::EventType::HEALTH_CHANGED) {
        updateHealthWithUI();
    } else if (event == engine::interface::EventType::MAX_HEALTH_CHANGED) {
        createHealthUI();
    }
}

void GameScene::init() {
    if (is_initialized_) {
        spdlog::warn("GameScene 已经初始化过了，重复调用 init()。");
        return;
    }
    spdlog::trace("GameScene 初始化开始...");
    context_.getGameState().setState(engine::core::State::Playing);
    game_session_data_->syncHighScore("assets/save.json");      // 更新最高分

    if (!initLevel()) {
        spdlog::error("关卡初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    if (!initPlayer()) {
        spdlog::error("玩家初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    if (!initUI()) {
        spdlog::error("UI初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    // 设置命令映射表
    if (auto player_component = player_->getComponent<game::component::PlayerComponent>(); player_component) {  
        setCommandMap(*player_component);
    } else {
        spdlog::error("玩家对象缺少 PlayerComponent 组件，无法设置命令映射表");
        context_.getInputManager().setShouldQuit(true);
        return;
    }

    // 播放背景音乐 (循环，淡入1秒)
    context_.getAudioPlayer().playMusic("assets/audio/hurry_up_and_run.ogg", true, 1000);

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
}

void GameScene::update(float delta_time) {
    Scene::update(delta_time);
    handleObjectCollisions();
    handleTileTriggers();

    // 玩家掉出地图下方则判断为失败
    if (player_ ) {
        auto pos = player_->getComponent<engine::component::TransformComponent>()->getPosition();
        auto world_rect = context_.getPhysicsEngine().getWorldBounds();
        // 多100像素冗余量
        if (world_rect && pos.y > world_rect->position.y + world_rect->size.y + 100.0f) {
            spdlog::debug("玩家掉出地图下方，游戏失败");
            showEndScene(false);
        }
    }
}

void GameScene::render() {
    Scene::render();
}

void GameScene::handleInput() {
    Scene::handleInput();
    // 检查暂停动作
    if (context_.getInputManager().isActionPressed("pause")) {
        spdlog::debug("在GameScene中检测到暂停动作，正在推送MenuScene。");
        scene_manager_.requestPushScene(std::make_unique<MenuScene>(context_, scene_manager_, game_session_data_));
    }
    // --- 将之前PlayerComponent的handleInput方法逻辑移到这里 ---
    // 判断左右移动操作
    if (context_.getInputManager().isActionDown("move_left")) {
        if (auto command = command_map_.find("move_left"); command != command_map_.end()) {
            command->second->execute();
        }
    } else if (context_.getInputManager().isActionDown("move_right")) {
        if (auto command = command_map_.find("move_right"); command != command_map_.end()) {
            command->second->execute();
        }
    }
    // 判断跳跃或上下移动操作（可以和左右操作同时进行）
    if (context_.getInputManager().isActionPressed("jump")) {
        if (auto command = command_map_.find("jump"); command != command_map_.end()) {
            command->second->execute();
        }
    } else if (context_.getInputManager().isActionDown("move_up")) {
        if (auto command = command_map_.find("move_up"); command != command_map_.end()) {
            command->second->execute();
        }
    } else if (context_.getInputManager().isActionDown("move_down")) {
        if (auto command = command_map_.find("move_down"); command != command_map_.end()) {
            command->second->execute();
        }
    }
}

void GameScene::clean() {
    Scene::clean();
}

bool GameScene::initLevel()
{
    // 创建关卡加载器
    engine::scene::LevelLoader level_loader(context_);

    // 设置新的生成器（针对当前游戏），替代默认的生成器
    auto builder = std::make_unique<game::object::ObjectBuilderSL>(level_loader, context_);
    level_loader.setObjectBuilder(std::move(builder)); 

    // 加载关卡
    auto level_path = game_session_data_->getMapPath();
    if (!level_loader.loadLevel(level_path, *this)){
        spdlog::error("关卡加载失败");
        return false;
    }

    // 注册"main"层到物理引擎
    auto* main_layer = findGameObjectByName("main");
    if (!main_layer) {
        spdlog::error("未找到\"main\"层");
        return false;
    }
    auto* tile_layer = main_layer->getComponent<engine::component::TileLayerComponent>();
    if (!tile_layer) {
        spdlog::error("\"main\"层没有 TileLayerComponent 组件");
        return false;
    }
    context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
    spdlog::info("注册\"main\"层到物理引擎");
    
    // 设置相机边界
    auto world_size = main_layer->getComponent<engine::component::TileLayerComponent>()->getWorldSize();
    context_.getCamera().setLimitBounds(engine::utils::Rect(glm::vec2(0.0f), world_size));
    context_.getCamera().setPosition(glm::vec2(0.0f));     // 开始时重置相机位置，以免切换场景时晃动

    // 设置世界边界
    context_.getPhysicsEngine().setWorldBounds(engine::utils::Rect(glm::vec2(0.0f), world_size));

    spdlog::trace("关卡初始化完成。");
    return true;
}

bool GameScene::initPlayer()
{
    // 获取玩家对象
    player_ = findGameObjectByName("player");
    if (!player_) {
        spdlog::error("未找到玩家对象");
        return false;
    }

    // 从SessionData中更新玩家生命值
    if (auto health_component = player_->getComponent<engine::component::HealthComponent>(); health_component) {
        health_component->setMaxHealth(game_session_data_->getMaxHealth());
        health_component->setCurrentHealth(game_session_data_->getCurrentHealth());
        // 注册订阅关系（GameScene 观察 HealthComponent 的生命值改变事件）
        health_component->addObserver(this);
        // SessionData 也同步更新 HealthComponent 的生命值改变事件
        health_component->addObserver(game_session_data_.get());
    } else {
        spdlog::error("玩家对象缺少 HealthComponent 组件，无法设置生命值");
        return false;
    }

    return true;
}

bool GameScene::initUI()
{
    if (!ui_manager_->init(context_.getGameState().getLogicalSize())) return false;
    
    createScoreUI();
    createHealthUI();
    return true;
}

void GameScene::setCommandMap(game::component::PlayerComponent& player_component)
{
    // 使用 operator[] 进行覆盖性插入
    command_map_["move_up"] = std::make_unique<game::component::command::UpCommand>(player_component);
    command_map_["move_down"] = std::make_unique<game::component::command::DownCommand>(player_component);
    command_map_["move_left"] = std::make_unique<game::component::command::LeftCommand>(player_component);
    command_map_["move_right"] = std::make_unique<game::component::command::RightCommand>(player_component);
    command_map_["jump"] = std::make_unique<game::component::command::JumpCommand>(player_component);
}

void GameScene::handleObjectCollisions()
{
    // 从物理引擎中获取碰撞对
    auto collision_pairs = context_.getPhysicsEngine().getCollisionPairs();
    for (const auto& pair : collision_pairs) {
        auto* obj1 = pair.first;
        auto* obj2 = pair.second;

        // 处理玩家与敌人的碰撞
        if (obj1->getName() == "player" && obj2->getTag() == "enemy") {
            playerVSEnemyCollision(obj1, obj2);
        } else if (obj2->getName() == "player" && obj1->getTag() == "enemy") {
            playerVSEnemyCollision(obj2, obj1);
        }
        // 处理玩家与道具的碰撞
        else if (obj1->getName() == "player" && obj2->getTag() == "item") {
            playerVSItemCollision(obj1, obj2);
        } else if (obj2->getName() == "player" && obj1->getTag() == "item") {
            playerVSItemCollision(obj2, obj1);
        }
        // 处理玩家与"hazard"对象碰撞
        else if (obj1->getName() == "player" && obj2->getTag() == "hazard") {
            handlePlayerDamage(1);
            spdlog::debug("玩家 {} 受到了 HAZARD 对象伤害", obj1->getName());
        } else if (obj2->getName() == "player" && obj1->getTag() == "hazard") {
            handlePlayerDamage(1);
            spdlog::debug("玩家 {} 受到了 HAZARD 对象伤害", obj2->getName());
        }
        // 处理玩家与关底触发器碰撞
        else if (obj1->getName() == "player" && obj2->getTag() == "next_level") {
            toNextLevel(obj2);
        } else if (obj2->getName() == "player" && obj1->getTag() == "next_level") {
            toNextLevel(obj1);
        }
        // 处理玩家与结束触发器碰撞
        else if (obj1->getName() == "player" && obj2->getName() == "win") {
            showEndScene(true);
        } else if (obj2->getName() == "player" && obj1->getName() == "win") {
            showEndScene(true);
        }
    }
}

void GameScene::handleTileTriggers()
{
    const auto& tile_trigger_events = context_.getPhysicsEngine().getTileTriggerEvents();
    for (const auto& event : tile_trigger_events) {
        auto* obj = event.first;        // 瓦片触发事件的对象
        auto tile_type = event.second;  // 瓦片类型
        if (tile_type == engine::component::TileType::HAZARD) {
            // 玩家碰到到危险瓦片，受伤
            if (obj->getName() == "player") {       
                handlePlayerDamage(1);
                spdlog::debug("玩家 {} 受到了 HAZARD 瓦片伤害", obj->getName());
            } 
            // TODO: 其他对象类型的处理，目前让敌人无视瓦片伤害
        }
    }
}

void GameScene::handlePlayerDamage(int damage)
{
    auto player_component = player_->getComponent<game::component::PlayerComponent>();
    if (!player_component->takeDamage(damage)) { // 没有受伤，直接返回
        return;
    }
    if (player_component->isDead()) {
        spdlog::info("玩家 {} 死亡", player_->getName());
        // TODO: 可能的死亡逻辑处理
    }
    // 不再需要手动更新生命值及HealthUI，因为HealthComponent会自动通知UI更新
}

void GameScene::playerVSEnemyCollision(engine::object::GameObject *player, engine::object::GameObject *enemy)
{
    // --- 踩踏判断逻辑：1. 玩家中心点在敌人上方    2. 重叠区域：overlap.x > overlap.y
    auto player_aabb = player->getComponent<engine::component::ColliderComponent>()->getWorldAABB();
    auto enemy_aabb = enemy->getComponent<engine::component::ColliderComponent>()->getWorldAABB();
    auto player_center = player_aabb.position + player_aabb.size / 2.0f;
    auto enemy_center = enemy_aabb.position + enemy_aabb.size / 2.0f;
    auto overlap = glm::vec2(player_aabb.size / 2.0f + enemy_aabb.size / 2.0f) - glm::abs(player_center - enemy_center);

    // 踩踏判断成功，敌人受伤
    if (overlap.x > overlap.y && player_center.y < enemy_center.y) {    
        spdlog::info("玩家 {} 踩踏了敌人 {}", player->getName(), enemy->getName());
        auto enemy_health = enemy->getComponent<engine::component::HealthComponent>();
        if (!enemy_health) {
            spdlog::error("敌人 {} 没有 HealthComponent 组件，无法处理踩踏伤害", enemy->getName());
            return;
        }
        enemy_health->takeDamage(1);  // 造成1点伤害
        if (!enemy_health->isAlive()) {
            spdlog::info("敌人 {} 被踩踏后死亡", enemy->getName());
            enemy->setNeedRemove(true);  // 标记敌人为待删除状态
            createEffect(enemy_center, enemy->getTag());  // 创建（死亡）特效
        }
        // 玩家跳起效果
        player->getComponent<engine::component::PhysicsComponent>()->velocity_.y = -300.0f;  // 向上跳起
        // 播放音效 (此音效完全可以放在玩家的音频组件中，这里示例另一种用法：直接用AudioPlayer播放，传入文件路径)
        context_.getAudioPlayer().playSound("assets/audio/punch2a.mp3");
        // 加分 (这里直接调用SessionData的addScore方法，它会自动通知UI更新)
        game_session_data_->addScore(10);
    }
    // 踩踏判断失败，玩家受伤
    else {
        spdlog::info("敌人 {} 对玩家 {} 造成伤害", enemy->getName(), player->getName());
        handlePlayerDamage(1);
    }
}

void GameScene::playerVSItemCollision(engine::object::GameObject*, engine::object::GameObject * item)
{
    if (item->getName() == "fruit") {
        player_->getComponent<engine::component::HealthComponent>()->heal(1);
    } else if (item->getName() == "gem") {
        game_session_data_->addScore(5);
    }
    item->setNeedRemove(true);  // 标记道具为待删除状态
    auto item_aabb = item->getComponent<engine::component::ColliderComponent>()->getWorldAABB();
    createEffect(item_aabb.position + item_aabb.size / 2.0f, item->getTag());  // 创建特效
    context_.getAudioPlayer().playSound("assets/audio/poka01.mp3");         // 播放音效
}

void GameScene::toNextLevel(engine::object::GameObject *trigger)
{
    auto scene_name = trigger->getName();
    auto map_path = levelNameToPath(scene_name);
    game_session_data_->setNextLevel(map_path);     // 设置下一个关卡信息
    auto next_scene = std::make_unique<game::scene::GameScene>(context_, scene_manager_, game_session_data_);
    scene_manager_.requestReplaceScene(std::move(next_scene));
}

void GameScene::showEndScene(bool is_win)
{
    spdlog::debug("显示结束场景，游戏 {}", is_win ? "胜利" : "失败");
    game_session_data_->setIsWin(is_win);
    auto end_scene = std::make_unique<game::scene::EndScene>(context_, scene_manager_, game_session_data_);
    scene_manager_.requestPushScene(std::move(end_scene));
}

void GameScene::createEffect(glm::vec2 center_pos, std::string_view tag)
{
    // --- 创建游戏对象和变换组件 ---
    auto effect_obj = std::make_unique<engine::object::GameObject>("effect_" + std::string(tag));
    effect_obj->addComponent<engine::component::TransformComponent>(std::move(center_pos));

    // --- 根据标签创建不同的精灵组件和动画--- 
    auto animation = std::make_unique<engine::render::Animation>("effect", false);
    if (tag == "enemy") {
        effect_obj->addComponent<engine::component::SpriteComponent>("assets/textures/FX/enemy-deadth.png", 
                                                                    context_.getResourceManager(),
                                                                    engine::utils::Alignment::CENTER);
        for (auto i = 0; i < 5; ++i) {
            animation->addFrame({static_cast<float>(i * 40), 0.0f, 40.0f, 41.0f}, 0.1f);
        }
    } else if (tag == "item") {
        effect_obj->addComponent<engine::component::SpriteComponent>("assets/textures/FX/item-feedback.png", 
                                                                    context_.getResourceManager(),
                                                                    engine::utils::Alignment::CENTER);
        for (auto i = 0; i < 4; ++i) {
            animation->addFrame({static_cast<float>(i * 32), 0.0f, 32.0f, 32.0f}, 0.1f);
        }
    } else {
        spdlog::warn("未知特效类型: {}", tag);
        return;
    }

    // --- 根据创建的动画，添加动画组件，并设置为单次播放 ---
    auto* animation_component = effect_obj->addComponent<engine::component::AnimationComponent>();
    animation_component->addAnimation(std::move(animation));
    animation_component->setOneShotRemoval(true);
    animation_component->playAnimation("effect");
    safeAddGameObject(std::move(effect_obj));  // 安全添加特效对象
    spdlog::debug("创建特效: {}", tag);
}

void GameScene::createScoreUI() {
    // 创建得分标签
    auto score_text = "Score: " + std::to_string(game_session_data_->getCurrentScore());
    auto score_label = std::make_unique<engine::ui::UILabel>(context_.getTextRenderer(), 
                                                         score_text, 
                                                         "assets/fonts/VonwaonBitmap-16px.ttf", 
                                                         16);
    score_label_ = score_label.get();           // 成员变量赋值（获取裸指针）
    auto screen_size = ui_manager_->getRootElement()->getSize();        // 获取屏幕尺寸
    score_label_->setPosition(glm::vec2(screen_size.x - 100.0f, 10.0f));
    ui_manager_->addElement(std::move(score_label));

    // 注册订阅关系（UILabel 观察 SessionData 的得分改变事件）
    game_session_data_->addObserver(score_label_);
}

void GameScene::createHealthUI() {
    // 获取当前生命值和最大生命值（不用再管SessionData，它会自动跟着HealthComponent更新）
    auto health_component = player_->getComponent<engine::component::HealthComponent>();
    int max_health = health_component->getMaxHealth();
    int current_health = health_component->getCurrentHealth();

    // 设置UI位置和大小
    float start_x = 10.0f;
    float start_y = 10.0f;
    float icon_width = 20.0f;
    float icon_height = 18.0f;
    float spacing = 5.0f;
    std::string full_heart_tex = "assets/textures/UI/Heart.png";
    std::string empty_heart_tex = "assets/textures/UI/Heart-bg.png";

    // 创建一个默认的UIPanel (不需要背景色，因此大小无所谓，只用于定位)
    auto health_panel = std::make_unique<engine::ui::UIPanel>();   
    health_panel_ = health_panel.get();           // 成员变量赋值（获取裸指针）

    // --- 根据最大生命值，循环创建生命值图标(添加到UIPanel中) ---
    for (int i = 0; i < max_health; ++i) {          // 创建背景图标
        glm::vec2 icon_pos = {start_x + i * (icon_width + spacing), start_y};
        glm::vec2 icon_size = {icon_width, icon_height};

        auto bg_icon = std::make_unique<engine::ui::UIImage>(empty_heart_tex, icon_pos, icon_size);
        health_panel_->addChild(std::move(bg_icon));
    }
    for (int i = 0; i < max_health; ++i) {          // 创建前景图标
        glm::vec2 icon_pos = {start_x + i * (icon_width + spacing), start_y};
        glm::vec2 icon_size = {icon_width, icon_height};

        auto fg_icon = std::make_unique<engine::ui::UIImage>(full_heart_tex, icon_pos, icon_size);
        bool is_visible = (i < current_health);  // 前景图标的可见性取决于当前生命值
        fg_icon->setVisible(is_visible);         // 设置前景图标的可见性
        health_panel_->addChild(std::move(fg_icon));
    }
    // 将UIPanel添加到UI管理器中
    ui_manager_->addElement(std::move(health_panel));
}

void GameScene::updateHealthWithUI()
{
    if (!player_ || !health_panel_) {
        spdlog::error("玩家对象或 HealthPanel 不存在，无法更新生命值UI");
        return;
    }
    // 获取当前生命值并更新UI（不用再管SessionData，它会自动跟着HealthComponent更新）
    auto health_component = player_->getComponent<engine::component::HealthComponent>();
    auto current_health = health_component->getCurrentHealth();
    auto max_health = health_component->getMaxHealth();

    // 前景图标是后添加的，因此设置后半段的可见性即可
    for (auto i = max_health; i < max_health * 2; ++i) {
        health_panel_->getChildren()[i]->setVisible(i - max_health < current_health);
    }
}

} // namespace game::scene 