#include "session_data.h"
#include <fstream>
#include <filesystem>
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

    // 通知观察者
    notifyObservers(engine::interface::EventType::SCORE_CHANGED, current_score_);
}

void SessionData::reset() {
    current_health_ = max_health_;
    current_score_ = 0;
    level_health_ = 3;
    level_score_ = 0;
    is_win_ = false;
    map_path_ = "assets/maps/level1.tmj";
    spdlog::info("SessionData reset.");
}

void SessionData::setNextLevel(std::string_view map_path)
{
    map_path_ = map_path;
    level_health_ = current_health_;
    level_score_ = current_score_;
}

bool SessionData::saveToFile(std::string_view filename) const {
    nlohmann::json j;
    try {
        // 将成员变量序列化到 JSON 对象中
        j["level_score"] = level_score_;
        j["level_health"] = level_health_;
        j["max_health"] = max_health_;
        j["high_score"] = high_score_;
        j["map_path"] = map_path_;

        // 打开文件进行写入

        auto path = std::filesystem::path(filename);
        std::ofstream ofs(path);
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

bool SessionData::loadFromFile(std::string_view filename) {
    try {
        // 打开文件进行读取
        auto path = std::filesystem::path(filename);
        std::ifstream ifs(path);
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
        high_score_ = glm::max(j.value("high_score", 0), high_score_);  // 文件中的最高分，与当前最高分取最大值
        map_path_ = j.value("map_path", "assets/maps/level1.tmj"); // 默认起始地图

        spdlog::info("游戏数据成功加载: {}", filename);
        return true;
    } catch (const std::exception& e) {
        spdlog::error("读档时出现错误 {}: {}", filename, e.what());
        reset();
        return false;
    }
}

bool SessionData::syncHighScore(std::string_view filename)
{
    try {
        // 打开文件进行读取
        auto path = std::filesystem::path(filename);
        std::fstream fs(path);
        if (!fs.is_open()) {
            spdlog::warn("找不到文件: {}, 无法进行同步", filename);
            return false;
        }

        // 从文件解析 JSON 数据
        nlohmann::json j;
        fs >> j;
        auto high_score_in_file = j.value("high_score", 0);

        // 根据文件中的最高分和当前最高分来决定处理方式
        if (high_score_in_file < high_score_) {     // 文件中的最高分 低于 当前最高分
            j["high_score"] = high_score_;
            fs.seekp(0);                // 文件指针回到文件开头
            fs << j.dump(4);            // 将JSON对象写入文件
            spdlog::debug("最高分高于存档文件，已将最高分保存到存档中");
        } else if (high_score_in_file > high_score_) {  // 文件中的最高分 高于 当前最高分
            high_score_ = high_score_in_file;
            spdlog::debug("存档文件中的最高分高于当前最高分，已更新当前最高分");
        } else {
            spdlog::debug("存档文件中的最高分与当前最高分相同，无需更新");
        }
        fs.close();
        return true;
    } catch (const std::exception& e) {
        spdlog::error("同步最高分时出现错误 {}: {}", filename, e.what());
        return false;
    }
}

} // namespace game::state 