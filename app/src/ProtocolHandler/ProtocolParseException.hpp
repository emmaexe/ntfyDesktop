#pragma once

#include <exception>
#include <string>

class ProtocolParseException: public std::exception {
    public:
        ProtocolParseException(const char* message);
        const char* what() const throw();
    private:
        std::string message;
};
