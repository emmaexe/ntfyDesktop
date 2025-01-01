#pragma once

#include <exception>
#include <map>
#include <string>
#include <vector>

class ParsedURLException: public std::exception {
    public:
        ParsedURLException(const char* message);
        const char* what() const throw();
    private:
        std::string message;
};

/**
 * @brief Struct representing a URL. Creates an immutable object that holds parsed data from a URL.
 */
struct ParsedURL {
    public:
        ParsedURL(const std::string& url);
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
        std::string internalProtocol = "", internalDomain = "";
        std::vector<std::string> internalPath = {};
        std::map<const std::string, const std::string> internalParams = {};
};
