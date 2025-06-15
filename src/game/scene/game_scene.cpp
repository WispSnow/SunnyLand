#include "game_scene.h"
#include "menu_scene.h"
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
#include "../../engine/ui/ui_button.h"
#include "../../engine/utils/math.h"
#include "../component/ai_component.h"
#include "../component/ai/patrol_behavior.h"
#include "../component/ai/updown_behavior.h"
#include "../component/ai/jump_behavior.h"
#include "../data/session_data.h"
#include <spdlog/spdlog.h>

namespace game::scene {

GameScene::GameScene(engine::core::Context& context, 
                     engine::scene::SceneManager& scene_manager,
                     std::shared_ptr<game::data::SessionData> data)
    : Scene("GameScene", context, scene_manager), game_session_data_(std::move(data)) {
    if (!game_session_data_) {      // 如果没有传入SessionData，则创建一个默认的
        game_session_data_ = std::make_shared<game::data::SessionData>();
        spdlog::info("未提供 SessionData，使用默认值。");
    }
    spdlog::trace("GameScene 构造完成。");
}

void GameScene::init() {
    if (is_initialized_) {
        spdlog::warn("GameScene 已经初始化过了，重复调用 init()。");
        return;
    }
    spdlog::trace("GameScene 初始化开始...");
    context_.getGameState().setState(engine::core::State::Playing);

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
    if (!initEnemyAndItem()) {
        spdlog::error("敌人和道具初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    if (!initUI()) {
        spdlog::error("UI初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }

    // 播放背景音乐 (循环，淡入1秒)
    // context_.getAudioPlayer().playMusic("assets/audio/hurry_up_and_run.ogg", true, 1000);

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
}

void GameScene::update(float delta_time) {
    Scene::update(delta_time);
    handleObjectCollisions();
    handleTileTriggers();
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
}

void GameScene::clean() {
    Scene::clean();
}

bool GameScene::initLevel()
{
    // 加载关卡（level_loader通常加载完成后即可销毁，因此不存为成员变量）
    engine::scene::LevelLoader level_loader;
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

    // 添加PlayerComponent到玩家对象
    auto* player_component = player_->addComponent<game::component::PlayerComponent>();
    if (!player_component) {
        spdlog::error("无法添加 PlayerComponent 到玩家对象");
        return false;
    }

    // 相机跟随玩家
    auto* player_transform = player_->getComponent<engine::component::TransformComponent>();
    if (!player_transform) {
        spdlog::error("玩家对象没有 TransformComponent 组件, 无法设置相机目标");
        return false;
    }
    context_.getCamera().setTarget(player_transform);
    spdlog::trace("Player初始化完成。");
    return true;
}

bool GameScene::initEnemyAndItem()
{
    bool success = true;
    for (auto& game_object : game_objects_){
        if (game_object->getName() == "eagle"){
            if (auto* ai_component = game_object->addComponent<game::component::AIComponent>(); ai_component){
                auto y_max = game_object->getComponent<engine::component::TransformComponent>()->getPosition().y;
                auto y_min = y_max - 80.0f;    // 让鹰的飞行范围 (当前位置与上方80像素 的区域)
                ai_component->setBehavior(std::make_unique<game::component::ai::UpDownBehavior>(y_min, y_max));
            }
        }
        if (game_object->getName() == "frog"){
            if (auto* ai_component = game_object->addComponent<game::component::AIComponent>(); ai_component){
                auto x_max = game_object->getComponent<engine::component::TransformComponent>()->getPosition().x - 10.0f;
                auto x_min = x_max - 90.0f;    // 青蛙跳跃范围（右侧 - 10.0f 是为了增加稳定性）
                ai_component->setBehavior(std::make_unique<game::component::ai::JumpBehavior>(x_min, x_max));
            }
        }
        if (game_object->getName() == "opossum"){
            if (auto* ai_component = game_object->addComponent<game::component::AIComponent>(); ai_component){
                auto x_max = game_object->getComponent<engine::component::TransformComponent>()->getPosition().x;
                auto x_min = x_max - 200.0f;    // 负鼠巡逻范围
                ai_component->setBehavior(std::make_unique<game::component::ai::PatrolBehavior>(x_min, x_max));
            }
        }
        if (game_object->getTag() == "item"){
            if (auto* ac = game_object->getComponent<engine::component::AnimationComponent>(); ac){
                ac->playAnimation("idle");
            } else {
                spdlog::error("Item对象缺少 AnimationComponent，无法播放动画。");
                success = false;
            }
        }
    }
    return success;
}

bool GameScene::initUI()
{
    if (!ui_manager_->init(context_.getGameState().getLogicalSize())) return false;
    
    createScoreUI();
    createHealthUI();
    return true;
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
    // 更新生命值及HealthUI
    updateHealthWithUI();
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
        // 加分
        addScoreWithUI(10);
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
        healWithUI(1);        // 加血
    } else if (item->getName() == "gem") {
        addScoreWithUI(5);    // 加5分
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

void GameScene::createEffect(const glm::vec2& center_pos, const std::string &tag)
{
    // --- 创建游戏对象和变换组件 ---
    auto effect_obj = std::make_unique<engine::object::GameObject>("effect_" + tag);
    effect_obj->addComponent<engine::component::TransformComponent>(center_pos);

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
}

void GameScene::createHealthUI() {
    int max_health = game_session_data_->getMaxHealth();
    int current_health = game_session_data_->getCurrentHealth();
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
    for (int i = 0; i < current_health; ++i) {      // 创建前景图标
        glm::vec2 icon_pos = {start_x + i * (icon_width + spacing), start_y};
        glm::vec2 icon_size = {icon_width, icon_height};

        auto fg_icon = std::make_unique<engine::ui::UIImage>(full_heart_tex, icon_pos, icon_size);
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

    // 获取当前生命值并更新游戏数据
    auto current_health = player_->getComponent<engine::component::HealthComponent>()->getCurrentHealth();
    game_session_data_->setCurrentHealth(current_health);
    auto max_health = game_session_data_->getMaxHealth();

    // 前景图标是后添加的，因此设置后半段的可见性即可
    for (auto i = max_health; i < max_health * 2; ++i) {
        health_panel_->getChildren()[i]->setVisible(i - max_health < current_health);
    }
}

void GameScene::addScoreWithUI(int score)
{
    game_session_data_->addScore(score);
    auto score_text = "Score: " + std::to_string(game_session_data_->getCurrentScore());
    spdlog::info("得分: {}", score_text);
    score_label_->setText(score_text);
}

void GameScene::healWithUI(int amount)
{
    player_->getComponent<engine::component::HealthComponent>()->heal(amount);
    updateHealthWithUI();                              // 更新生命值与UI
}

} // namespace game::scene 