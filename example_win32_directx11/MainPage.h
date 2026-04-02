#pragma once

#include "imgui.h"
#include "InitializingPage.h"
#include "ModalDialog.h"
#include <vector>
#include <string>
#include <ctime>

// 主页面状态
struct MainPageState {
    // 当前选中的导航标签
    int selected_tab = 0; // 0: DASHBOARD, 1: PROCESSES, 2: RESOURCES
    
    // 搜索栏文本
    char search_buffer[256] = "";
    
    // 选中的进程索引
    int selected_process = 0;
    
    // 进程类型枚举
    enum ProcessType {
        PROCESS_TYPE_GAME,      // 游戏进程
        PROCESS_TYPE_SYSTEM,    // 系统进程
        PROCESS_TYPE_APP,       // 应用进程
        PROCESS_TYPE_MARKED     // 标记进程
    };
    
    // 进程列表数据
    struct ProcessInfo {
        std::string name;
        std::string pid;
        std::string icon; // 图标名称
        bool is_active;
        ProcessType type; // 进程类型
    };
    std::vector<ProcessInfo> processes;
    
    // 动画状态
    float tab_indicator_x = 20.0f; // 下划线指示器位置
    float tab_indicator_target = 20.0f;
    
    // 日志条目结构
    struct LogEntry {
        std::string timestamp;
        std::string message;
        std::string status;  // SUCCESS, READY, ERROR等
    };
    std::vector<LogEntry> logs;
    
    // 模态对话框状态
    ModalDialogState modal_dialog_state;
    
    // 添加日志的方法
    void AddLog(const std::string& message, const std::string& status = "") {
        // 获取当前时间
        time_t now = time(0);
        struct tm* timeinfo = localtime(&now);
        char time_buffer[16];
        strftime(time_buffer, sizeof(time_buffer), "[%H:%M:%S]", timeinfo);
        
        logs.push_back({time_buffer, message, status});
        // 限制日志数量，最多保留50条
        if (logs.size() > 50) {
            logs.erase(logs.begin());
        }
    }
    
    // 构造函数初始化进程列表和日志
    MainPageState() {
        processes = {
            {"VALORANT-Win64-Shipping.exe", "14202", "terminal", true, PROCESS_TYPE_GAME},
            {"Discord.exe", "08812", "deployed_code", false, PROCESS_TYPE_APP},
            {"chrome.exe", "22104", "browser_updated", false, PROCESS_TYPE_APP},
            {"steam.exe", "04419", "sports_esports", false, PROCESS_TYPE_GAME},
            {"lsass.exe", "00662", "memory", false, PROCESS_TYPE_SYSTEM}
        };
        
        // 初始化一些示例日志
        AddLog("Requesting handle for PID 14202...", "SUCCESS");
        AddLog("Mapping internal_bypass_v2.dll into virtual space...", "READY");
        AddLog("Waiting for user trigger...", "");
    }
};

// 渲染主页面
void RenderMainPage(MainPageState& state, float window_width, float window_height,
                    ImFont* font_logo,           // Inter Semibold 18 - LOGO
                    ImFont* font_nav,            // Inter Semibold 12 - 导航
                    ImFont* font_title,          // Manrope ExtraBold 48 - 进程标题
                    ImFont* font_body,           // Inter Medium 16 - PID等一行
                    ImFont* font_small,          // Inter Semibold 10 - 小字体
                    ImFont* font_mono,           // 等宽字体（日志）
                    // 新增字体参数
                    ImFont* font_process_selected,   // Inter Semibold 20 - 选中项
                    ImFont* font_process_unselected, // Inter Medium 14 - 未选中项
                    ImFont* font_inject_title,       // Inter Semibold 11 - INJECTION TARGET
                    ImFont* font_dll_name,           // Inter Semibold 14 - DLL名
                    ImFont* font_browse,             // Inter Semibold 12 - BROWSE
                    ImFont* font_file_info,          // Inter Regular 11 - Size等文件信息
                    ImFont* font_button_inject,      // Manrope Bold 18 - 注入按钮
                    ImFont* font_button_other);      // Manrope Bold 14 - 其他按钮

// 渲染顶部导航栏
void RenderTopBar(MainPageState& state, float window_width, ImVec2 window_pos,
                  ImFont* font_logo, ImFont* font_nav);

// 渲染底部栏（独立于主内容区域）
void RenderBottomBar(MainPageState& state, float window_width, float window_height, 
                     float bar_height, ImVec2 window_pos, ImFont* font_small);

// 渲染侧边栏内容
void RenderSidebarContent(MainPageState& state, float width, float height,
                          ImFont* font_title, ImFont* font_body, ImFont* font_small,
                          ImFont* font_process_selected, ImFont* font_process_unselected);

// 渲染主内容区域
void RenderMainContentArea(MainPageState& state, float width, float height,
                           ImFont* font_title, ImFont* font_body, 
                           ImFont* font_small, ImFont* font_mono,
                           ImFont* font_inject_title, ImFont* font_dll_name,
                           ImFont* font_browse, ImFont* font_file_info,
                           ImFont* font_button_inject, ImFont* font_button_other);

// 为了保持兼容性，保留旧函数声明
void RenderSidebar(MainPageState& state, float width, float height,
                   ImFont* font_title, ImFont* font_body, ImFont* font_small);
void RenderMainContent(MainPageState& state, float width, float height,
                       ImFont* font_title, ImFont* font_body, 
                       ImFont* font_small, ImFont* font_mono);
