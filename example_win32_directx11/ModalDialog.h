#pragma once

#include <imgui.h>
#include <string>
#include <vector>

// 模块信息结构
struct ModuleInfo {
    std::string name;
    std::string size;
    std::string baseAddress;
    bool selected;
};

// 模态对话框状态
struct ModalDialogState {
    bool is_open = false;
    float open_animation = 0.0f;  // 0.0 - 1.0
    float target_animation = 0.0f;
    
    // 搜索缓冲区
    char search_buffer[256] = "";
    
    // 全选状态
    bool select_all = false;
    
    // 模块列表
    std::vector<ModuleInfo> modules;
    
    // 滚动位置
    float scroll_y = 0.0f;
    
    // 构造函数初始化示例模块
    ModalDialogState();
    
    // 打开/关闭对话框
    void Open();
    void Close();
    
    // 更新动画
    void UpdateAnimation(float delta_time);
    
    // 获取过滤后的模块列表
    std::vector<size_t> GetFilteredIndices() const;
    
    // 获取选中的模块数量
    int GetSelectedCount() const;
    
    // 切换全选
    void ToggleSelectAll();
};

// 渲染模态对话框
// 返回true表示点击了DUMP按钮，false表示取消或关闭
bool RenderModalDialog(ModalDialogState& state, ImFont* font_title, ImFont* font_body, ImFont* font_small);
