#ifndef CONFIG_H
#define CONFIG_H

#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>

class Config: public nlohmann::json {
    public:
        Config();
        void read();
        void write();
        bool changesMade;
        static Config inherit(Config c);
        bool ok;
        std::string errorMessage;
    protected:
        std::string configPath;
        std::string getConfigPath();
};

#endif //CONFIG_H