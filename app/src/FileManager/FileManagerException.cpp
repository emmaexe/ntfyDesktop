#include "FileManagerException.hpp"

FileManagerException::FileManagerException(const char* message): message(message) {}

const char* FileManagerException::what() const throw() { return this->message.c_str(); }
