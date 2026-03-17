#include "ParsedURL.hpp"

#include "./Util.hpp"

#include <format>

std::expected<ParsedURL, std::string> ParsedURL::from_string(std::string_view url) {
    if (!Util::Strings::contains(url, "://")) { return std::unexpected(std::format("\"{}\" is not a valid url.", url)); }
    std::vector<std::string> parts = Util::Strings::split(url, "://");

    std::string protocol = "", domain = "";
    std::vector<std::string> path = {};
    std::map<const std::string, const std::string> params = {};

    protocol = parts[0];
    parts = Util::Strings::split(parts[1], "/");

    domain = parts[0];
    for (int i = 1; i < parts.size() - 1; i++) { path.push_back(parts[i]); }

    parts = Util::Strings::split(parts.back(), "?");
    path.push_back(parts[0]);

    if (parts.size() > 1 && !parts.back().empty()) {
        parts = Util::Strings::split(parts.back(), "&");
        for (int i = 0; i < parts.size(); i++) {
            std::vector<std::string> pair = Util::Strings::split(parts[i], "=");
            if (pair.size() != 2) { continue; }
            params.insert(std::make_pair(pair[0], pair[1]));
        }
    }

    return ParsedURL(protocol, domain, path, params);
}

ParsedURL::ParsedURL(std::string protocol, std::string domain, std::vector<std::string> path, std::map<const std::string, const std::string> params): m_protocol(protocol), m_domain(domain), m_path(path), m_params(params) {}

const std::string& ParsedURL::protocol() { return this->m_protocol; }

const std::string& ParsedURL::domain() { return this->m_domain; }

const std::vector<std::string>& ParsedURL::path() { return this->m_path; }

const std::map<const std::string, const std::string>& ParsedURL::params() { return this->m_params; }
