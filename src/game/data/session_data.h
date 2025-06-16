#pragma once
#include <string>
#include <memory>
#include <nlohmann/json.hpp> 

namespace game::data {

/**
 * @brief 管理不同游戏场景之间的游戏状态
 *
 * 存储玩家生命值、分数、当前关卡等信息，
 * 使这些数据在场景切换时能够保持。
 */
class SessionData final {
private:
    int current_health_ = 3;
    int max_health_ = 3;
    int current_score_ = 0;
    int high_score_ = 0;
    bool is_win_ = false;
    
    int level_health_ = 3;          ///< @brief 进入关卡时的生命值（读/存档用）
    int level_score_ = 0;           ///< @brief 进入关卡时的得分（读/存档用）
    std::string map_path_ = "assets/maps/level1.tmj";

public:
    SessionData() = default;
    ~SessionData() = default;

    // 删除复制和移动操作以防止意外复制
    SessionData(const SessionData&) = delete;
    SessionData& operator=(const SessionData&) = delete;
    SessionData(SessionData&&) = delete;
    SessionData& operator=(SessionData&&) = delete;

    // --- Getters ---
    int getCurrentHealth() const { return current_health_; }
    int getMaxHealth() const { return max_health_; }
    int getCurrentScore() const { return current_score_; }
    int getHighScore() const { return high_score_; }
    int getLevelHealth() const { return level_health_; }
    int getLevelScore() const { return level_score_; }
    const std::string& getMapPath() const { return map_path_; }
    bool getIsWin() const { return is_win_; }

    // --- Setters ---
    void setCurrentHealth(int health);
    void setMaxHealth(int max_health);
    void addScore(int score_to_add);
    void setHighScore(int high_score) { high_score_= high_score; }
    void setLevelHealth(int level_health) {level_health_ = level_health; }
    void setLevelScore(int level_score) {level_score_ = level_score; }
    void setMapPath(const std::string& map_path) { map_path_ = map_path; }
    void setIsWin(bool is_win) { is_win_ = is_win; }

    void reset();                                           ///< @brief 重置游戏数据以准备开始新游戏（保留最高分）
    void setNextLevel(const std::string& map_path);         ///< @brief 设置下一个场景信息（地图、关卡开始时的得分生命）
    bool saveToFile(const std::string& filename) const;     ///< @brief 将当前游戏数据保存到JSON文件（存档）
    bool loadFromFile(const std::string& filename);         ///< @brief 从JSON文件中读取游戏数据（读档）
    bool syncHighScore(const std::string& filename);        ///< @brief 同步最高分(文件与当前分数取最大值)
};

} // namespace game::state