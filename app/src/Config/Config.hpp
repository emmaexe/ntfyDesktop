#pragma once

#include <nlohmann/json.hpp>

#include <optional>

/**
 * @brief A static global class, wrapper for the json config file.
 */
class Config {
    public:
        Config() = delete;
        /**
         * @brief The json configuration data.
         *
         * @return nlohmann::json& data
         */
        static nlohmann::json& data();
        /**
         * @brief Read the configuration data from the json file into the internal variable.
         */
        static void read();
        /**
         * @brief Write the configuration data from the internal variable into the json file.
         */
        static void write();
        /**
         * @brief Check if the config was initialized successfully.
         */
        static bool ready();
        /**
         * @brief Reset the config to the default values.
         */
        static void reset();
        /**
         * @brief Get the error message if the config was not initialized successfully. Otherwise, returns an empty string.
         *
         * @return const std::string& errorMessage
         */
        static const std::string& getError();
    private:
        static bool initialized, ok;
        static nlohmann::json internalData;
        static std::string internalError;
        static const std::string getConfigPath();
        static const std::string getConfigFile();
};
