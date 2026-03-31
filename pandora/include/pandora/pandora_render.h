#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

#ifdef _WIN32
#include <d3d11.h>
#endif

namespace Pandora {
namespace Render {

#ifdef _WIN32

struct SVGTexture {
    ID3D11ShaderResourceView* srv = nullptr;
    int width = 0;
    int height = 0;
};

class SVGCacheManager {
public:
    void SetDevice(ID3D11Device* device);
    void SetMaxCacheSize(size_t size);
    bool LoadSVG(const char* path, const char* units = "px", float dpi = 96.0f);
    SVGTexture GetScaledTexture(float scale);

private:
    using QuantizedScale = int;
    QuantizedScale Quantize(float scale) const;

    struct CachedTexture {
        SVGTexture texture;
        size_t lastUsedTime;
        float scale;
        bool IsValid() const { return texture.srv != nullptr; }
    };

    std::unordered_map<QuantizedScale, CachedTexture> _scaleCache;
    void* _svgSource = nullptr;
    ID3D11Device* _d3dDevice = nullptr;
    size_t _maxCacheSize = 5;
    std::mutex _cacheMutex;
};

struct LottieTexture {
    ID3D11ShaderResourceView* srv = nullptr;
    int width = 0;
    int height = 0;
};

class LottieAnimator {
public:
    bool Init(const char* jsonPath, ID3D11Device* device, int width, int height, bool loop = true);
    void Update(float deltaTime);
    void Render(ID3D11DeviceContext* context);
    LottieTexture GetTexture() const;
    void SetLoop(bool loop);
    void Play();
    void Pause();
    void Stop();

private:
    void* _animation = nullptr;
    ID3D11Device* _device = nullptr;
    ID3D11Texture2D* _texture = nullptr;
    ID3D11ShaderResourceView* _srv = nullptr;
    int _width = 0;
    int _height = 0;
    bool _loop = true;
    bool _playing = false;
    float _currentFrame = 0.0f;
    float _totalFrames = 0.0f;
    float _frameRate = 60.0f;
};

#endif

}
}
