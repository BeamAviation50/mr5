#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <Windows.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

static std::string strip_link(const std::string& link) {
    std::string key = "privateServerLinkCode=";
    size_t pos = link.find(key);
    if (pos != std::string::npos) {
        return link.substr(pos + key.length());
    }
    return link;
}

class Config {
public:
    std::string version;
    std::string webhook;
    std::string roleid;

    std::vector<std::string> priv_links;
    std::vector<std::string> raw_links;

    int stp;
    int vtp;
    int srh;
    int cape;
    int timeout;
    int success_wait;

    bool shared;

    bool load(const std::string& filename) {
        std::ifstream file(filename);

        if (!file.is_open()) {
            ordered_json default_cfg;
            default_cfg["version"] = "1.22";
            default_cfg["priv_link_1"] = "";
            default_cfg["webhook"] = "";
            default_cfg["roleid"] = "";
            default_cfg["shared"] = false;
            default_cfg["stp"] = 1;
            default_cfg["vtp"] = 1;
            default_cfg["srh"] = 1;
            default_cfg["cape"] = 1;
            default_cfg["timeout"] = 30;
            default_cfg["success_wait"] = 300;

            std::ofstream out(filename);
            if (out.is_open()) {
                out << default_cfg.dump(4);
                out.close();
                printf("created config file, please fill up: %s\n", filename.c_str());
            }
            else {
                printf("failed to write config file, please try running as administrator\n");
                std::cin.get();
                return false;
            }

            return false;
        }

        try {
            json j;
            file >> j;

            version = j.value("version", "1.22");
            roleid = j.value("roleid", "");
            stp = j.value("stp", 0);
            vtp = j.value("vtp", 0);
            srh = j.value("srh", 0);
            cape = j.value("cape", 0);
            timeout = j.value("timeout", 30);
            success_wait = j.value("success_wait", 300);
            shared = j.value("shared", false);

            if (shared) {
                roleid = "1464539393858474056";
                webhook = "https://discord.com/api/webhooks/1523675040455987240/0H_MIFMGBqfyHwy2M4JqWmcXZKqvfH67izmnPIv86nbnqR2RJaBXEc0C_I-F--jxjjkX";
            }
            else {
                webhook = j.value("webhook", "");
            }

            priv_links.clear();
            raw_links.clear();

            for (auto& [key, value] : j.items()) {
                if (key.find("priv_link_") == 0 && value.is_string()) {
                    std::string raw = value.get<std::string>();
                    if (!raw.empty()) {
                        raw_links.push_back(raw);
                        priv_links.push_back(strip_link(raw));
                    }
                }
            }

            std::string error_msg = "";
            if (webhook.empty()) error_msg += "- webhook url is missing\n";
            if (priv_links.empty()) error_msg += "- vip links not found (priv_link_1, etc.)\n";

            if (!error_msg.empty()) {
                std::string full_msg = "config.json is incomplete:\n\n" + error_msg + "\nplease fix these issues and restart.";
                printf("%s\n", full_msg.c_str());
                std::cin.get();
                return false;
            }
        }
        catch (const std::exception& e) {
            printf("json parse error: %s\n", e.what());
            return false;
        }

        return true;
    }
};