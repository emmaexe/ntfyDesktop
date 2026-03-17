#pragma once

#include <expected>
#include <map>
#include <string>
#include <vector>

/**
 * @brief Struct representing a URL. Creates an immutable object that holds parsed data from a URL.
 */
struct ParsedURL {
    public:
        static std::expected<ParsedURL, std::string> from_string(std::string_view url);
        /**
         * @brief The protocol of the parsed URL. (e.g. for `https://www.example.org/some/path/?help=true&data=abc` this would be `https`)
         */
        const std::string& protocol();
        /**
         * @brief The domain of the parsed URL. (e.g. for `https://www.example.org/some/path/?help=true&data=abc` this would be `www.example.org`)
         */
        const std::string& domain();
        /**
         * @brief The path of the parsed URL. (e.g. for `https://www.example.org/some/path/?help=true&data=abc` this would be `{"some","path",""}`)
         */
        const std::vector<std::string>& path();
        /**
         * @brief The parameters of the parsed URL. (e.g. for `https://www.example.org/some/path?help=true&data=abc` this would be `{"help":"true", "data":"abc"}`)
         */
        const std::map<const std::string, const std::string>& params();
    private:
        ParsedURL(std::string protocol, std::string domain, std::vector<std::string> path, std::map<const std::string, const std::string> params);
        std::string m_protocol = "", m_domain = "";
        std::vector<std::string> m_path = {};
        std::map<const std::string, const std::string> m_params = {};
};
