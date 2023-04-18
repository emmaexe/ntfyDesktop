#include "config.h"

Config::Config(): ok(true), changesMade(false), configPath(getConfigPath()) {
    read();
}

void Config::read() {
    std::ifstream configStream(configPath);
    if (configStream.is_open()) {
        try {
            update(parse(configStream));
        } catch(parse_error e) { ok = false; errorMessage = e.what(); }
    } else {
        ok = false; errorMessage = "The config file does not exist.\nRecommended action: Reset all configuration.";
    }
    configStream.close();
}

void Config::write() {
    std::string configSerialized = dump();
    std::ofstream configStream;
    configStream.open(configPath, std::ios::trunc);
    for (char c: configSerialized) {
        configStream.put(c);
    }
    configStream.close();
}

Config Config::inherit(Config c) {
    Config config;
    config.ok = c.ok;
    if (!c.errorMessage.empty()) { config.errorMessage = c.errorMessage; }
    config.update(c);
    return config;
}

std::string Config::getConfigPath() {
    std::string homePath = getenv("HOME");
    std::string configPath = homePath + "/.config/ntfyDesktop/config.json";
    return configPath;
}