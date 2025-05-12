#pragma once
#include "logger.h"
#include <string>

namespace jetfire27::Engine::JsonParser {
    template<typename T>
    class Parser {
    public:
        std::string Marshall(const T& obj);
        T UnMarshall(const std::string& jsonStr);
    };
}
