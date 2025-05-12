#pragma once
#include "logger.h"
#include <string>

namespace jetfire27::Engine {
    enum class Mode { AutoStart, Service };

    class Daemonizer {
    public:
        static void Setup(const std::string& binPath, Mode mode);
        static void Remove(Mode mode);
        static bool IsSingleInstance(); 
    };
}
