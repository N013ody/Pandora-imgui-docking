#pragma once
#include <chrono>

namespace Pandora {
namespace Platform {

inline unsigned long long GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

}
}
