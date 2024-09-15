#include "ProtocolHandler.hpp"

#include "../Util/Util.hpp"

ProtocolHandler::ProtocolHandler(const std::string& url) {
    if (url.find("://") == std::string::npos) { throw ProtocolParseException("String is not valid url."); }
    std::vector<std::string> parts = Util::split(url, "://");

    this->internalProtocol = parts[0];
    parts = Util::split(parts[1], "/");

    this->internalDomain = parts[0];
    for (int i = 1; i < parts.size() - 1; i++) { this->internalPath.push_back(parts[i]); }

    parts = Util::split(parts.back(), "?");
    this->internalPath.push_back(parts[0]);

    if (parts.size() > 1 && !parts.back().empty()) {
        parts = Util::split(parts.back(), "&");
        for (int i = 0; i < parts.size(); i++) {
            std::vector<std::string> pair = Util::split(parts[i], "=");
            if (pair.size() != 2) { continue; }
            this->internalParams.insert(std::make_pair(pair[0], pair[1]));
        }
    }
}

const std::string& ProtocolHandler::protocol() { return this->internalProtocol; }

const std::string& ProtocolHandler::domain() { return this->internalDomain; }

const std::vector<std::string>& ProtocolHandler::path() { return this->internalPath; }

const std::map<const std::string, const std::string>& ProtocolHandler::params() { return this->internalParams; }
