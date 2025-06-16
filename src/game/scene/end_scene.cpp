#include "end_scene.h"
#include "title_scene.h"
#include "game_scene.h"
#include "../data/session_data.h"
#include "../../engine/core/context.h"
#include "../../engine/core/game_state.h"
#include "../../engine/input/input_manager.h"
#include "../../engine/scene/scene_manager.h"
#include "../../engine/ui/ui_manager.h"
#include "../../engine/ui/ui_label.h"
#include "../../engine/ui/ui_button.h"
#include "../../engine/utils/math.h"
#include <spdlog/spdlog.h>

namespace game::scene {

EndScene::EndScene(engine::core::Context& context,
                   engine::scene::SceneManager& scene_manager,
                   std::shared_ptr<game::data::SessionData> session_data)
    : engine::scene::Scene("EndScene", context, scene_manager),
      session_data_(std::move(session_data)) {
    if (!session_data_) {
        spdlog::error("错误：结束场景收到了空的游戏数据！");
    }
    spdlog::trace("EndScene (胜利：{}) 创建.", session_data_->getIsWin() ? "是" : "否");
}

void EndScene::init() {
    if (is_initialized_) {
        return;
    }

    // 设置游戏状态为 GameOver
    context_.getGameState().setState(engine::core::State::GameOver);

    createUI();

    Scene::init();
    spdlog::info("EndScene 初始化完成。");
}

void EndScene::createUI() {
    auto window_size = context_.getGameState().getLogicalSize();
    if (!ui_manager_->init(window_size)) {
         spdlog::error("错误：结束场景的 UI 管理器初始化失败！");
         return;
    }
    auto is_win = session_data_->getIsWin();

    // --- 主文字标签 ---
    std::string main_message = is_win ? "YOU WIN! CONGRATS!" : "YOU DIED! TRY AGAIN!";
    // 赢了是绿色，输了是红色
    engine::utils::FColor message_color = is_win ? engine::utils::FColor{0.0f, 1.0f, 0.0f, 1.0f} : 
                                                   engine::utils::FColor{1.0f, 0.0f, 0.0f, 1.0f};

    auto main_label = std::make_unique<engine::ui::UILabel>(context_.getTextRenderer(),
                                                            main_message,
                                                            "assets/fonts/VonwaonBitmap-16px.ttf",
                                                            48,
                                                            message_color);
    // 标签居中
    glm::vec2 label_size = main_label->getSize();
    glm::vec2 main_label_pos = {(window_size.x - label_size.x) / 2.0f, window_size.y * 0.3f};
    main_label->setPosition(main_label_pos);
    ui_manager_->addElement(std::move(main_label));


    // --- 得分标签 ---
    int current_score = session_data_->getCurrentScore();
    int high_score = session_data_->getHighScore();
    engine::utils::FColor score_color = {1.0f, 1.0f, 1.0f, 1.0f};
    int score_font_size = 24;

    // 当前得分
    std::string score_text = "Score: " + std::to_string(current_score);
    auto score_label = std::make_unique<engine::ui::UILabel>(context_.getTextRenderer(),
                                                             score_text,
                                                             "assets/fonts/VonwaonBitmap-16px.ttf",
                                                             score_font_size,
                                                             score_color);
    glm::vec2 score_label_size = score_label->getSize();
    // x方向居中，y方向在主标签下方20像素
    glm::vec2 score_label_pos = {(window_size.x - score_label_size.x) / 2.0f, main_label_pos.y + label_size.y + 20.0f};
    score_label->setPosition(score_label_pos);
    ui_manager_->addElement(std::move(score_label));

    // --- 最高分 ---
    std::string high_score_text = "High Score: " + std::to_string(high_score);
    auto high_score_label = std::make_unique<engine::ui::UILabel>(context_.getTextRenderer(),
                                                                  high_score_text,
                                                                  "assets/fonts/VonwaonBitmap-16px.ttf",
                                                                  score_font_size,
                                                                  score_color);
    glm::vec2 high_score_label_size = high_score_label->getSize();
    // x方向居中，y方向在当前得分下方10像素
    glm::vec2 high_score_label_pos = {(window_size.x - high_score_label_size.x) / 2.0f, score_label_pos.y + score_label_size.y + 10.0f};
    high_score_label->setPosition(high_score_label_pos);
    ui_manager_->addElement(std::move(high_score_label));

    // --- UI按钮 ---
    glm::vec2 button_size = {120.0f, 40.0f};        // 让按钮更大一点
    float button_spacing = 20.0f;
    float total_button_width = button_size.x * 2 + button_spacing;

    // 按钮放在右下角，与边缘间隔30像素
    float buttons_x = window_size.x - total_button_width - 30.0f;
    float buttons_y = window_size.y - button_size.y - 30.0f;

    // Back Button
    auto back_button = std::make_unique<engine::ui::UIButton>(context_,
                                                              "assets/textures/UI/buttons/Back1.png",
                                                              "assets/textures/UI/buttons/Back2.png",
                                                              "assets/textures/UI/buttons/Back3.png",
                                                              glm::vec2{buttons_x, buttons_y},
                                                              button_size,
                                                              [this]() { this->onBackClick(); });
    ui_manager_->addElement(std::move(back_button));

    // Restart Button
    buttons_x += button_size.x + button_spacing;
    auto restart_button = std::make_unique<engine::ui::UIButton>(context_,
                                                                 "assets/textures/UI/buttons/Restart1.png",
                                                                 "assets/textures/UI/buttons/Restart2.png",
                                                                 "assets/textures/UI/buttons/Restart3.png",
                                                                 glm::vec2{buttons_x, buttons_y},
                                                                 button_size,
                                                                 [this]() { this->onRestartClick(); });
    ui_manager_->addElement(std::move(restart_button));
}

void EndScene::onBackClick() {
    spdlog::info("返回按钮被点击。");
    scene_manager_.requestReplaceScene(std::make_unique<TitleScene>(context_, scene_manager_, session_data_));
}

void EndScene::onRestartClick() {
    spdlog::info("重新开始按钮被点击。");
    // 重新开始游戏
    session_data_->reset();
    scene_manager_.requestReplaceScene(std::make_unique<GameScene>(context_, scene_manager_, session_data_));
}

} // namespace game::scene