#pragma once

#include <string>
#include <vector>
#include <map>

#include "ProtocolParseException.hpp"

class ProtocolHandler {
    public:
        ProtocolHandler(const std::string& url);
        const std::string& protocol();
        const std::string& domain();
        const std::vector<std::string>& path();
        const std::map<const std::string, const std::string>& params();
    private:
        std::string internalProtocol = "", internalDomain = "";
        std::vector<std::string> internalPath = {};
        std::map<const std::string, const std::string> internalParams = {};
};
