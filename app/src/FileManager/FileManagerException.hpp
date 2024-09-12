#pragma once

#include <exception>
#include <string>

class FileManagerException: public std::exception {
    public:
        FileManagerException(const char* message);
        const char* what() const throw();
    private:
        std::string message;
};
