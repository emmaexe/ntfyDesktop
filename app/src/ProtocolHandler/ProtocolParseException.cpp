#include "ProtocolParseException.hpp"

ProtocolParseException::ProtocolParseException(const char* message): message(message) {}

const char* ProtocolParseException::what() const throw() {
    return this->message.c_str();
}
