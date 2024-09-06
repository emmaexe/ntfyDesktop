#include "ProtocolHandler.hpp"

std::vector<std::string> split(const std::string& string, const std::string& delimiter) {
    std::vector<std::string> parts;
    std::string string_c = std::string(string);
    size_t pos = 0;
    std::string token;
    while ((pos = string_c.find(delimiter)) != std::string::npos) {
        token = string_c.substr(0, pos);
        parts.push_back(token);
        string_c.erase(0, pos + delimiter.length());
    }
    parts.push_back(string_c);
    return parts;
}

ProtocolHandler::ProtocolHandler(const std::string& url) {
    if (url.find("://") == std::string::npos) {
        throw ProtocolParseException("String is not valid url.");
    }
    std::vector<std::string> parts = split(url, "://");

    this->internalProtocol = parts[0];
    parts = split(parts[1], "/");

    this->internalDomain = parts[0];
    for (int i = 1; i < parts.size()-1; i++) {
        this->internalPath.push_back(parts[i]);
    }

    parts = split(parts.back(), "?");
    this->internalPath.push_back(parts[0]);

    if (parts.size() > 1 && !parts.back().empty()) {
        parts = split(parts.back(), "&");
        for (int i = 0; i < parts.size(); i++) {
            std::vector<std::string> pair = split(parts[i], "=");
            if (pair.size() != 2) { continue; }
            this->internalParams.insert(std::make_pair(pair[0], pair[1]));
        }
    }
}

const std::string& ProtocolHandler::protocol() {
    return this->internalProtocol;
}

const std::string& ProtocolHandler::domain() {
    return this->internalDomain;
}

const std::vector<std::string>& ProtocolHandler::path() {
    return this->internalPath;
}

const std::map<const std::string, const std::string>& ProtocolHandler::params() {
    return this->internalParams;
}

