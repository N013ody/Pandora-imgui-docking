#pragma once

#include <d3d11.h>
#include <Images.h>

static ID3D11ShaderResourceView* playImagePtr = nullptr;
static ID3D11ShaderResourceView* icon_logo = nullptr;
static ID3D11Resource* textureResource = nullptr;
HRESULT GuiMainInit();

void RenderLoginWindow();

void RenderMainWindow();


