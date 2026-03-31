#pragma once
#include <imgui.h>

namespace Pandora {

enum PandoraWindowFlags_ : ImGuiDataType_U32 {
    PandoraWindowFlags_None              = 0,
    PandoraWindowFlags_DropFiles         = 1 << 0,
    PandoraWindowFlags_BlurBackground    = 1 << 1,
    PandoraWindowFlags_AttachedWindow    = 1 << 2,
    PandoraWindowFlags_ShadowWindow      = 1 << 3,
};
typedef ImGuiDataType_U32 PandoraWindowFlags;

struct PandoraWindowData {
    PandoraWindowFlags flags = PandoraWindowFlags_None;
    void* platformHandle = nullptr;
};

}
