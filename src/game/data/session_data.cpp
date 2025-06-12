#include "session_data.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <glm/common.hpp>

namespace game::data {

void SessionData::setCurrentHealth(int health) {
    // 将生命值限制在 0 和 max_health_ 之间
    current_health_ = glm::clamp(health, 0, max_health_);
}

void SessionData::setMaxHealth(int max_health) {
    if (max_health > 0) {
        max_health_ = max_health;
        // 确保当前生命值不超过新的最大生命值
        setCurrentHealth(current_health_);
    } else {
        spdlog::warn("尝试将最大生命值设置为非正数: {}", max_health);
    }
}

void SessionData::addScore(int score_to_add) {
    current_score_ += score_to_add;
    setHighScore(glm::max(high_score_, current_score_)); // 如果当前分数超过最高分，则更新最高分
}

void SessionData::reset() {
    current_health_ = max_health_;
    current_score_ = 0;
    level_health_ = 3;
    level_score_ = 0;
    map_path_ = "assets/maps/level1.tmj";
    spdlog::info("SessionData reset.");
}

void SessionData::setNextLevel(const std::string &map_path)
{
    map_path_ = map_path;
    level_health_ = current_health_;
    level_score_ = current_score_;
}

bool SessionData::saveToFile(const std::string& filename) const {
    nlohmann::json j;
    try {
        // 将成员变量序列化到 JSON 对象中
        j["level_score"] = level_score_;
        j["level_health"] = level_health_;
        j["max_health"] = max_health_;
        j["high_score"] = high_score_;
        j["map_path"] = map_path_;

        // 打开文件进行写入
        std::ofstream ofs(filename);
        if (!ofs.is_open()) {
            spdlog::error("无法打开存档文件进行写入: {}", filename);
            return false;
        }

        // 将 JSON 对象写入文件（使用4个空格进行美化输出）
        ofs << j.dump(4);
        ofs.close(); // 确保文件关闭

        spdlog::info("游戏数据成功存储到: {}", filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("存档时出现错误 {}: {}", filename, e.what());
        return false;
    }
}

bool SessionData::loadFromFile(const std::string& filename) {
    try {
        // 打开文件进行读取
        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            spdlog::warn("读档时找不到文件: {}", filename);
            // 如果存档文件不存在，这不一定是错误
            return false;
        }

        // 从文件解析 JSON 数据
        nlohmann::json j;
        ifs >> j;
        ifs.close(); // 读取完成后关闭文件

        current_score_ = level_score_ = j.value("level_score", 0);
        current_health_ = level_health_ = j.value("level_health", 3);
        max_health_ = j.value("max_health", 3); // 使用合理的默认值
        high_score_ = j.value("high_score", 0);
        map_path_ = j.value("map_path", "assets/maps/level1.tmj"); // 默认起始地图

        spdlog::info("游戏数据成功加载: {}", filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("读档时出现错误 {}: {}", filename, e.what());
        reset();
        return false;
    }
}

} // namespace game::state 