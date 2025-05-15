#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>

int main(int, char**) {
    try {
        // 1. 载入JSON文件
        std::ifstream input_file("assets/json_example.json");
        nlohmann::ordered_json json_data = nlohmann::ordered_json::parse(input_file);
        input_file.close();
        spdlog::info("JSON 成功载入!");

        // 2. 获取不同类型的数据
        // 2.1 字符串 (String)
        std::string name = json_data["name"].get<std::string>();
        spdlog::info("Name: {}", name);

        // 2.2 数字 (Number)
        int age = json_data["age"].get<int>();
        double height = json_data["height_meters"].get<double>();
        spdlog::info("Age: {}, Height: {}", age, height);

        // 2.3 布尔值 (Boolean)
        bool isStudent = json_data["isStudent"].get<bool>();
        spdlog::info("Is Student: {}", isStudent);
        // 2.4 null 值
        // 检查是否为null
        if (json_data["middleName"].is_null()) {
            spdlog::info("Middle Name: null");
        } else {
            spdlog::info("Middle Name: {}", json_data["middleName"].get<std::string>());
        }

        // 2.5 另外一种方法：使用 .at() 方法访问
        std::string email = json_data.at("email").get<std::string>();
        spdlog::info("Email: {}", email);

        // 3 安全访问的方法
        // 3.1 .contains() 检查某个键是否存在，如果不存在则返回 false
        if (json_data.contains("email"))
        {
            std::string email = json_data.at("email").get<std::string>();
            spdlog::info("Email: {}", email);
        }
        if (json_data.contains("nonExistentKey"))
        {
            spdlog::info("nonExistentKey found!"); // 不会执行
        }
        else
        {
            spdlog::info("'nonExistentKey' not found.");
        }
        
        // 3.2 .value() 获取一个可能存在的键，如果不存在则返回指定默认值（第二个参数）
        std::string optional_value = json_data.value("optionalKey", "default_string_value");
        int optional_int = json_data.value("optionalNumber", 42);
        spdlog::info("Optional Key (string): {}", optional_value);
        spdlog::info("Optional Key (int): {}", optional_int);

        // 4 对象 (Object)
        nlohmann::ordered_json address_obj = json_data["address"];
        std::string street = address_obj["street"].get<std::string>();
        std::string city = address_obj.value("city", "Unknown City"); // 使用 .value() 提供默认值
        bool isPrimaryAddr = address_obj.value("isPrimary", false);   // 访问对象内的布尔值
        spdlog::info("Address: {}, {}", street, city);
        spdlog::info("Is Primary Address: {}", isPrimaryAddr);    
        
        // 5.1 数组 (Array) - 字符串数组
        spdlog::info("Hobbies:");
        nlohmann::ordered_json hobbies_array = json_data["hobbies"];
        for (const auto &hobby : hobbies_array)
        {
            spdlog::info("  - {}", hobby.get<std::string>());
        }

        // 5.2 数组 (Array) - 数字数组
        spdlog::info("Scores:");
        for (const auto &score_item : json_data["scores"])
        {
            if (score_item.is_number_integer())
            {
                spdlog::info("  - {} (integer)", score_item.get<int>());
            }
            else if (score_item.is_number_float())
            {
                spdlog::info("  - {} (float)", score_item.get<double>());
            }
        }

        // 5.3 数组 (Array) - 对象数组
        spdlog::info("Projects:");
        nlohmann::ordered_json projects_array = json_data["projects"];
        for (const auto &project : projects_array)
        {
            std::string projectName = project["projectName"].get<std::string>();
            std::string status = project["status"].get<std::string>();
            double budget = project.value("budget", 0.0); // 使用 value 获取，若不存在则为0.0
            bool isActive = project.value("isActive", false);

            spdlog::info("  ProjectName: {}", projectName);
            spdlog::info("  Status: {}", status);
            spdlog::info("  Budget: {}", budget);
            spdlog::info("  Is Active: {}", isActive);
            if (project.contains("deadline") && project["deadline"].is_null())
            {
                spdlog::info("  Deadline: null");
            }
            else if (project.contains("deadline"))
            {
                spdlog::info("  Deadline: {}", project["deadline"].get<std::string>());
            }
            spdlog::info("--------------------------------");
        }
        // 5.4 直接访问更深层嵌套的对象和数组
        double metadata_version = json_data["metadata"]["version"].get<double>();
        spdlog::info("Metadata Version: {}", metadata_version);
        spdlog::info("Metadata Tags:");
        for (const auto &tag_json : json_data["metadata"]["tags"])
        {
            std::string tag = tag_json.get<std::string>();
            spdlog::info("  - {}", tag);
        }

        // 6 将json数据保存为文件
        std::ofstream output_file("assets/save_json.json");
        output_file << json_data.dump(4); // 使用 dump(4) 进行格式化输出，缩进为4个空格
        output_file.close();
        spdlog::info("JSON 数据已保存到文件 assets/save_json.json");
        
    } catch (const std::exception &e) {
        spdlog::error("Exception: {}", e.what());
        // return EXIT_FAILURE;
    }


    return 0;
}