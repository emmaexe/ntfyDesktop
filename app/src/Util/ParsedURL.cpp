#include "ParsedURL.hpp"

#include "./Util.hpp"

ParsedURLException::ParsedURLException(const char* message): message(message) {}

const char* ParsedURLException::what() const throw() { return this->message.c_str(); }

ParsedURL::ParsedURL(const std::string& url) {
    if (!Util::Strings::contains(url, "://")) {
        const std::string err = "\"" + url + "\" is not a valid url.";
        throw ParsedURLException(err.c_str());
    }
    std::vector<std::string> parts = Util::Strings::split(url, "://");

    this->internalProtocol = parts[0];
    parts = Util::Strings::split(parts[1], "/");

    this->internalDomain = parts[0];
    for (int i = 1; i < parts.size() - 1; i++) { this->internalPath.push_back(parts[i]); }

    parts = Util::Strings::split(parts.back(), "?");
    this->internalPath.push_back(parts[0]);

    if (parts.size() > 1 && !parts.back().empty()) {
        parts = Util::Strings::split(parts.back(), "&");
        for (int i = 0; i < parts.size(); i++) {
            std::vector<std::string> pair = Util::Strings::split(parts[i], "=");
            if (pair.size() != 2) { continue; }
            this->internalParams.insert(std::make_pair(pair[0], pair[1]));
        }
    }
}

const std::string& ParsedURL::protocol() { return this->internalProtocol; }

const std::string& ParsedURL::domain() { return this->internalDomain; }

const std::vector<std::string>& ParsedURL::path() { return this->internalPath; }

const std::map<const std::string, const std::string>& ParsedURL::params() { return this->internalParams; }
