#pragma once
#include <functional>
#include <memory>
#include <cmath>
#include <vector>
#include <imgui.h>

namespace Pandora {
namespace Animation {

#define PANDORA_PI 3.14159265358979323846f

template<typename T>
const T& clamp(const T& value, const T& low, const T& high) {
    return (value < low) ? low : (high < value) ? high : value;
}

namespace Ease {
    inline float Linear(float t) { return t; }
    inline float InQuad(float t) { return t * t; }
    inline float OutQuad(float t) { return t * (2 - t); }
    inline float InOutQuad(float t) {
        return t < 0.5f ? 2 * t * t : -1 + (4 - 2 * t) * t;
    }
    inline float InCubic(float t) { return t * t * t; }
    inline float OutCubic(float t) { return (--t) * t * t + 1; }
    inline float InOutCubic(float t) {
        return t < 0.5f ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1;
    }
    inline float InQuart(float t) { return t * t * t * t; }
    inline float OutQuart(float t) { return 1 - (--t) * t * t * t; }
    inline float InOutQuart(float t) {
        return t < 0.5f ? 8 * t * t * t * t : 1 - 8 * (--t) * t * t * t;
    }
    inline float InSine(float t) {
        return 1 - std::cos(t * PANDORA_PI / 2);
    }
    inline float OutSine(float t) {
        return std::sin(t * PANDORA_PI / 2);
    }
    inline float InOutSine(float t) {
        return -(std::cos(PANDORA_PI * t) - 1) / 2;
    }
    inline float InExpo(float t) {
        return t == 0 ? 0 : std::pow(2, 10 * (t - 1));
    }
    inline float OutExpo(float t) {
        return t == 1 ? 1 : 1 - std::pow(2, -10 * t);
    }
    inline float InOutExpo(float t) {
        if (t == 0) return 0;
        if (t == 1) return 1;
        return t < 0.5f
            ? std::pow(2, 20 * t - 10) / 2
            : (2 - std::pow(2, -20 * t + 10)) / 2;
    }
    inline float InCirc(float t) {
        return 1 - std::sqrt(1 - t * t);
    }
    inline float OutCirc(float t) {
        return std::sqrt(1 - (--t) * t);
    }
    inline float InOutCirc(float t) {
        return t < 0.5f
            ? (1 - std::sqrt(1 - 4 * t * t)) / 2
            : (std::sqrt(1 - (2 * t - 2) * (2 * t - 2)) + 1) / 2;
    }
    inline float InElastic(float t) {
        constexpr float c4 = (2 * PANDORA_PI) / 3;
        return t == 0 ? 0 : t == 1 ? 1
            : -std::pow(2, 10 * t - 10) * std::sin((t * 10 - 10.75f) * c4);
    }
    inline float OutElastic(float t) {
        constexpr float c4 = (2 * PANDORA_PI) / 3;
        return t == 0 ? 0 : t == 1 ? 1
            : std::pow(2, -10 * t) * std::sin((t * 10 - 0.75f) * c4) + 1;
    }
    inline float InOutElastic(float t) {
        constexpr float c5 = (2 * PANDORA_PI) / 4.5f;
        return t == 0 ? 0 : t == 1 ? 1 : t < 0.5f
            ? -(std::pow(2, 20 * t - 10) * std::sin((20 * t - 11.125f) * c5)) / 2
            : (std::pow(2, -20 * t + 10) * std::sin((20 * t - 11.125f) * c5)) / 2 + 1;
    }
    inline float OutBounce(float t) {
        constexpr float n1 = 7.5625f;
        constexpr float d1 = 2.75f;
        if (t < 1 / d1) {
            return n1 * t * t;
        }
        else if (t < 2 / d1) {
            return n1 * (t -= 1.5f / d1) * t + 0.75f;
        }
        else if (t < 2.5 / d1) {
            return n1 * (t -= 2.25f / d1) * t + 0.9375f;
        }
        else {
            return n1 * (t -= 2.625f / d1) * t + 0.984375f;
        }
    }
    inline float InBounce(float t) {
        return 1 - OutBounce(1 - t);
    }
    inline float InOutBounce(float t) {
        return t < 0.5f
            ? (1 - OutBounce(1 - 2 * t)) / 2
            : (1 + OutBounce(2 * t - 1)) / 2;
    }
}

enum State { Stopped, Running, Paused };

struct Params {
    float duration = 1.0f;
    float delay = 0.0f;
    int loopCount = 1;
    bool pingPong = false;
    std::function<float(float)> easing = Ease::Linear;
};

class IAnimatable {
public:
    virtual void Update() = 0;
    virtual ~IAnimatable() = default;
};

class AnimationCallback {
public:
    template<typename T>
    using CallbackType = std::function<void(const T&, float)>;
    virtual ~AnimationCallback() = default;
    virtual void Execute(const void* data, float t) = 0;
};

template<typename T>
class TypedCallback : public AnimationCallback {
public:
    explicit TypedCallback(CallbackType<T> func) : m_func(std::move(func)) {}
    void Execute(const void* data, float t) override {
        if (m_func)
            m_func(*static_cast<const T*>(data), t);
    }
private:
    CallbackType<T> m_func;
};

template<typename T>
class Animation : public IAnimatable {
public:
    using TriggerAction = std::function<void()>;
    using TriggerPoint = std::pair<float, TriggerAction>;

    void Start(const T& startVal, const T& endVal, const Params& params = {}) {
        m_start = startVal;
        m_end = endVal;
        m_params = params;
        m_state = Running;
        m_startTime = ImGui::GetTime();
        m_progress = 0.0f;
    }

    void Update() override {
        if (m_state != Running) return;
        const float lastProgress = m_progress;
        const float now = ImGui::GetTime();
        const float elapsed = now - m_startTime - m_params.delay;
        if (elapsed < 0) return;
        m_progress = clamp(elapsed / m_params.duration, 0.0f, 1.0f);
        const float t = m_params.easing(m_progress);
        T current = Lerp(m_start, m_end, t);
        for (auto& [progress, action] : m_triggers) {
            if (lastProgress < progress && m_progress >= progress) {
                action();
            }
        }
        if (m_callback)
            m_callback->Execute(&current, m_progress);
        if (m_progress >= 1.0f)
            HandleLoop();
        m_currentValue = current;
    }

    T GetCurrentValue() const { return m_currentValue; }
    State GetState() const { return m_state; }
    void AddTrigger(float progress, TriggerAction action) {
        m_triggers.emplace_back(progress, action);
    }
    void SetCallback(std::shared_ptr<AnimationCallback> callback) {
        m_callback = std::dynamic_pointer_cast<TypedCallback<T>>(callback);
    }

private:
    T Lerp(const T& a, const T& b, float t) {
        if constexpr (std::is_arithmetic_v<T>) {
            return a + (b - a) * t;
        }
        else if constexpr (std::is_same_v<T, ImVec2>) {
            return ImVec2(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        }
        else if constexpr (std::is_same_v<T, ImVec4>) {
            return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t,
                          a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
        }
        return a;
    }

    void HandleLoop() {
        bool loopFinished = false;
        if (m_params.loopCount == -1) {
            loopFinished = false;
        } else {
            loopFinished = (m_loopCount >= m_params.loopCount - 1);
        }
        if (!loopFinished) {
            if (m_params.pingPong) {
                std::swap(m_start, m_end);
                m_progress = 0.0f;
                m_startTime = ImGui::GetTime();
                m_loopCount++;
            } else {
                m_progress = 0.0f;
                m_startTime = ImGui::GetTime();
                m_loopCount++;
            }
        } else {
            m_state = Stopped;
            if (m_callback && m_params.loopCount != -1) {
                m_callback->Execute(&m_end, 1.0f);
            }
        }
    }

    Params m_params;
    State m_state = Stopped;
    T m_start{}, m_end{};
    float m_startTime = 0.0f;
    float m_progress = 0.0f;
    int m_loopCount = 0;
    T m_currentValue;
    std::shared_ptr<TypedCallback<T>> m_callback;
    std::vector<TriggerPoint> m_triggers;
};

class AnimationController {
public:
    template<typename T>
    void AddAnimation(std::shared_ptr<Animation<T>> anim) {
        m_activeAnims.emplace_back(anim);
    }

    void UpdateAll() {
        auto it = m_activeAnims.begin();
        while (it != m_activeAnims.end()) {
            if (auto anim = it->lock()) {
                anim->Update();
                ++it;
            } else {
                it = m_activeAnims.erase(it);
            }
        }
    }

private:
    std::vector<std::weak_ptr<IAnimatable>> m_activeAnims;
};

}
}
