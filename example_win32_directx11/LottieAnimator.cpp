                                                                                                                                                                                                                                                                                                                                                                                           #include "LottieAnimator.h"

LottieAnimator::LottieAnimator() {}
LottieAnimator::~LottieAnimator() { Release(); }


bool LottieAnimator::Init(const char* jsonPath,ID3D11Device* device,const int width,const int height,bool upSampling) {
    _animation = rlottie::Animation::loadFromFile(jsonPath);
    _width = width;
    _height = height;
    return _InitializeD3DResources(device);
}

bool LottieAnimator::InitFromData(const char* jsonData, ID3D11Device* device, const int width, const int height, const bool upSampling) {
    _animation = rlottie::Animation::loadFromData(jsonData, "");
    _width = width;
    _height = height;
    return _InitializeD3DResources(device);
}

ImVec2 LottieAnimator::GetSize()
{
    return ImVec2(_width, _height);
}

ID3D11Texture2D* LottieAnimator::GetTexture() {
    return _texture;
}

bool LottieAnimator::_InitializeD3DResources(ID3D11Device* device) {
    if (!_animation || !device) return false;


    device->GetImmediateContext(&_context);
    if (!_context) return false;

    D3D11_TEXTURE2D_DESC desc;

    desc.Width = _width;
    desc.Height = _height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
   // desc.MipLevels = 0; //全链mipmap
    //desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;

    HRESULT hr = device->CreateTexture2D(&desc, nullptr, &_texture);
    if (FAILED(hr))
        return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {
       desc.Format,
       D3D11_SRV_DIMENSION_TEXTURE2D,
       { 0, 1 }
    };
    bool result = SUCCEEDED(device->CreateShaderResourceView(_texture, &srvDesc, &_srv));

    return result;
}


void LottieAnimator::Update(float deltaTime) {
    if (!_animation || !_isPlaying) return;

    _progress += deltaTime * _speed / _animation->duration();
    if (_progress > 1.0f) _progress = 0.0f;
    _UpdateTexture();
}


void LottieAnimator::Seek(float progress) {
    _progress = std::clamp(progress, 0.0f, 1.0f);
    size_t targetFrame = static_cast<size_t>(_progress * _animation->totalFrame());
    _UpdateTexture();
}

std::unique_ptr<rlottie::Animation> LottieAnimator::GetAnimation() {
    return std::move(_animation);
}

void LottieAnimator::Stop() {

    _progress = 0.0f;
    _isPlaying = false;
    if (_animation) {
        std::vector<uint32_t> buffer(_width * _height);
        rlottie::Surface surface(
            buffer.data(),
            _width,
            _height,
            _width * 4
        );
        _animation->renderSync(0, surface);
        _UpdateTexture();
    }
}

void LottieAnimator::_UpdateTexture() {
    if (!_animation || !_texture || !_context) return;

    //渲染到缓冲区
    std::vector<uint32_t> buffer(_width * _height);
    rlottie::Surface surface(
        buffer.data(),
        _width,
        _height,
        _width * 4     //步长 = width x 4bytes
    );
    _animation->renderSync(static_cast<size_t>(_progress * _animation->totalFrame()), surface);

    //更新D3D纹理
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (FAILED(_context->Map(_texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) return;

    uint8_t* dst = reinterpret_cast<uint8_t*>(mapped.pData);
    const uint8_t* src = reinterpret_cast<uint8_t*>(buffer.data());
    const uint32_t srcRowPitch = _width * 4;
    //这里解决渲染错误问题 （长宽和json内width height过大时渲染错位）
    //逐行拷贝 考虑实际RowPitch
    for (UINT y = 0; y < _height; ++y) {
        memcpy(dst, src, srcRowPitch);
        dst += mapped.RowPitch;  //使用D3D11返回的RowPitch
        src += srcRowPitch;
    }

    _context->Unmap(_texture, 0);


}

// 释放资源
void LottieAnimator::Release() {
    if (_srv) { _srv->Release(); _srv = nullptr; }
    if (_texture) { _texture->Release(); _texture = nullptr; }
    _animation.reset();
    if (_context) { _context->Release(); _context = nullptr; }
}
