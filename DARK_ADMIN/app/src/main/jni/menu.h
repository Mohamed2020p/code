/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */
#pragma once
#include "include/includes.h"
#include "game.h"
#include "game/Ruleset.h"
#include "imgui/inc/8bp.h"
#include <curl/curl.h>
#include "include/java.h"
#include "mod/keylogin.h"
#include "oxorany/oxorany.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <sys/system_properties.h>
#include <ctime>
#include <Vector/Vectors.h>
#include <imgui/imgui.h>
#include "icons/icons.h"
#include "icons/coin_100.h"
#include "icons/coin_100m.h"
#include "icons/coin_200m.h"
#include "icons/material_icons.h"
#include "icons/mod_india.h"
#include "icons/mod_lightning.h"
#include "icons/mod_telegram.h"
#include "icons/mod_lock.h"
#include "icons/mod_calendar.h"

using namespace ImGui;
using namespace std;

struct MenuState {
    bool isOpen = false;
    int currentTab = 0;
    float sidebarWidth = 750.0f;
    float animProgress = 0.0f;
    float menuAlpha = 0.0f;
    float menuScale = 0.9f;
    ImVec4 accentColor = ImVec4(0.133f, 0.827f, 0.933f, 1.0f);
};
static MenuState g_menu;

static const int64_t EXPIRY_TS = O(1788805800LL);

static bool DEBUG_BYPASS_LOGIN = false;

static float EaseOutBack(float x) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(x - 1.0f, 3.0f) + c1 * powf(x - 1.0f, 2.0f);
}

static float EaseOutQuart(float x) {
    return 1.0f - powf(1.0f - x, 4.0f);
}

static void DrawGradientRect(ImDrawList* dl, ImVec2 p1, ImVec2 p2, ImU32 col1, ImU32 col2, bool horizontal = true) {
    if (horizontal) {
        dl->AddRectFilledMultiColor(p1, p2, col1, col2, col2, col1);
    } else {
        dl->AddRectFilledMultiColor(p1, p2, col1, col1, col2, col2);
    }
}

// Obsidian + cyan accent visual system (2026 UI refresh).
// Dark surfaces, one electric-cyan accent, cooler neutrals.
// Token names are stable - no widget below needs further changes.
static const ImU32 UI_ACCENT       = IM_COL32(34, 211, 238, 255);  // #22D3EE primary accent
static const ImU32 UI_ACCENT_DARK  = IM_COL32(21,  94, 117, 255);  // #155E75 deep accent
static const ImU32 UI_ACCENT_LIGHT = IM_COL32(165, 243, 252, 255); // #A5F3FC bright accent
static const ImU32 UI_CARD         = IM_COL32(24,  35,  53, 255);  // #182335 raised cards
static const ImU32 UI_PANEL        = IM_COL32(19,  28,  42, 255);  // #131C2A content panels
static const ImU32 UI_WELL         = IM_COL32(14,  22,  34, 255);  // #0E1622 sunken inputs
static const ImU32 UI_TEXT         = IM_COL32(232, 238, 245, 255); // #E8EEF5 primary text
static const ImU32 UI_MUTED        = IM_COL32(154, 168, 184, 255); // #9AA8B8 secondary text

static void DrawThemedPanel(ImDrawList* dl, ImVec2 a, ImVec2 b, float rounding = 18.0f) {
    dl->AddRectFilled(a, b, UI_PANEL, rounding);
    dl->AddRect(a, b, IM_COL32(34, 56, 74, 255), rounding, 0, 2.0f);
    dl->AddRect(ImVec2(a.x+3, a.y+3), ImVec2(b.x-3, b.y-3), IM_COL32(46, 74, 94, 200), rounding-3.0f, 0, 1.0f);
}

// Global UI scale (user-chosen, percent / 100). Never exceeds 100% when
// iMenuScale is valid (60..100); default 90 so icons/menus render compactly.
static inline float uiScale() {
    int v = persistent_int["iMenuScale"];
    if (v <= 0 || v > 100) return 0.9f;
    return v / 100.0f;
}

// Reference-style target/crosshair icon used by the Auto Play information card.
static void DrawAutoPlayTarget(ImDrawList* dl, ImVec2 center, float r) {
    const ImU32 gold = IM_COL32(8, 145, 178, 255);
    const ImU32 light = IM_COL32(34, 211, 238, 255);
    dl->AddCircle(center, r, gold, 40, 2.2f);
    dl->AddCircle(center, r * 0.56f, light, 40, 2.0f);
    dl->AddCircleFilled(center, r * 0.18f, gold);
    dl->AddLine(ImVec2(center.x-r*1.18f, center.y), ImVec2(center.x-r*0.72f, center.y), gold, 2.0f);
    dl->AddLine(ImVec2(center.x+r*0.72f, center.y), ImVec2(center.x+r*1.18f, center.y), gold, 2.0f);
    dl->AddLine(ImVec2(center.x, center.y-r*1.18f), ImVec2(center.x, center.y-r*0.72f), gold, 2.0f);
    dl->AddLine(ImVec2(center.x, center.y+r*0.72f), ImVec2(center.x, center.y+r*1.18f), gold, 2.0f);
}

static bool SidebarButton(const char* label, GLuint iconTex, bool selected, float width) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return false;
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    float iconSize = 46.0f * uiScale(); // user-scale aware, capped at 100%
    float vPad = 8.0f;
    float btnH = vPad + iconSize + 4.0f + g.FontSize + vPad;
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size(width, btnH);
    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id)) return false;
    bool hovered=false, held=false;
    bool pressed=ButtonBehavior(bb,id,&hovered,&held);
    ImDrawList* dl=window->DrawList;
    if (selected) {
        dl->AddRectFilled(bb.Min, bb.Max, UI_ACCENT, 12.0f);
        dl->AddRect(bb.Min, bb.Max, IM_COL32(34,211,238,170), 12.0f, 0, 1.5f);
    } else if (hovered) {
        dl->AddRectFilled(bb.Min, bb.Max, IM_COL32(34,211,238,40), 12.0f);
    }
    ImVec2 center(bb.Min.x + width*0.5f, bb.Min.y + vPad + iconSize*0.5f);
    if (iconTex) {
        ImVec2 mn(center.x-iconSize*0.5f,center.y-iconSize*0.5f);
        ImVec2 mx(center.x+iconSize*0.5f,center.y+iconSize*0.5f);
        dl->AddImage((void*)(intptr_t)iconTex,mn,mx,ImVec2(0,0),ImVec2(1,1));
    }
    ImVec2 labelSize=CalcTextSize(label);
    ImVec2 textPos(bb.Min.x+(width-labelSize.x)*0.5f,bb.Min.y+vPad+iconSize+4.0f);
    dl->AddText(textPos,selected?IM_COL32(6,32,42,255):UI_TEXT,label);
    return pressed;
}

static bool ToggleSwitch(const char* label, bool* v) {
    ImGuiWindow* window=GetCurrentWindow();
    if(window->SkipItems) return false;
    ImGuiContext& g=*GImGui;
    const ImGuiID id=window->GetID(label);
    float height=28.0f, width=54.0f, radius=height*0.5f;
    ImVec2 textSize=CalcTextSize(label);
    ImVec2 pos=window->DC.CursorPos;
    ImVec2 size(GetContentRegionAvail().x, ImMax(height,textSize.y)+14.0f);
    ImRect bb(pos,pos+size);
    ItemSize(size,g.Style.FramePadding.y);
    if(!ItemAdd(bb,id)) return false;
    bool hovered=false,held=false;
    bool pressed=ButtonBehavior(bb,id,&hovered,&held);
    if(pressed)*v=!*v;
    static std::map<ImGuiID,float> anim;
    float &t=anim[id];
    float target=*v?1.0f:0.0f;
    t+=(target-t)*g.IO.DeltaTime*14.0f;
    ImDrawList* dl=window->DrawList;
    if(hovered) dl->AddRectFilled(bb.Min,bb.Max,IM_COL32(34,211,238,24),10.0f);
    ImVec2 togglePos(bb.Max.x-width-12.0f,bb.Min.y+(size.y-height)*0.5f);
    ImVec2 toggleEnd(togglePos.x+width,togglePos.y+height);
    ImVec4 off=ImVec4(0.39f,0.27f,0.11f,1.0f);
    ImVec4 on =ImVec4(0.82f,0.55f,0.10f,1.0f);
    dl->AddRectFilled(togglePos,toggleEnd,ImColor(ImLerp(off,on,t)),radius);
    dl->AddRect(togglePos,toggleEnd,IM_COL32(42,60,78,180),radius,0,1.0f);
    float knobX=togglePos.x+radius+(width-height)*t;
    float knobY=togglePos.y+radius;
    dl->AddCircleFilled(ImVec2(knobX+1,knobY+1),radius-3,IM_COL32(0,0,0,70));
    dl->AddCircleFilled(ImVec2(knobX,knobY),radius-4,IM_COL32(240,250,252,255));
    dl->AddText(ImVec2(bb.Min.x+12.0f,bb.Min.y+(size.y-textSize.y)*0.5f),UI_TEXT,label);
    return pressed;
}

// File-scope so DrawToggleButton cancel can also reset countdown
static bool g_aqCounting = false;
static std::chrono::steady_clock::time_point g_aqLastCall;
static std::chrono::steady_clock::time_point g_aqCountdownStart;


static bool IsExpired() {
    return (int64_t)time(nullptr) >= EXPIRY_TS;
}

INLINE void DrawExpired(ImGuiIO& io) {
    float winW = g_menu.sidebarWidth;

    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    SetNextWindowSize(ImVec2(winW, 0), ImGuiCond_Always);
    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(13, 20, 32, 255));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 20.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.0f, 30.0f));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (Begin(O("##ExpiredWin"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
              ImGuiWindowFlags_AlwaysAutoResize)) {

        SetWindowFontScale(1.6f);
        ImVec2 titleSz = CalcTextSize(O("MOD EXPIRED"));
        SetCursorPosX((winW - 60.0f - titleSz.x) * 0.5f);
        TextColored(ImVec4(0.973f, 0.443f, 0.443f, 1.0f), "%s", O("MOD EXPIRED"));
        SetWindowFontScale(1.0f);

        Dummy(ImVec2(0, 16));

        PushTextWrapPos(GetCursorPosX() + winW - 60.0f);
        TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), "%s",
            O("Beta Version Expired. Update on our Telegram @DARK_OWNER_VIP"));
        PopTextWrapPos();

        Dummy(ImVec2(0, 10));
    }
    End();
    PopStyleVar(3);
    PopStyleColor();
}

INLINE void DrawAutoQueue() {
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        auto now = std::chrono::steady_clock::now();

        // Give the game a short moment after leaving the menu, then start the
        // selected table directly. This is more reliable than hard-coded swipe
        // coordinates and works across different screen sizes/orientations.
        static bool hasStartedOnce = false;
        static auto lastStart = std::chrono::steady_clock::now() - std::chrono::milliseconds(2500);
        const auto sinceStart = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStart).count();
        if (hasStartedOnce && sinceStart < 2500) return;

        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - g_aqLastCall).count() > 500)
            g_aqCounting = false;
        g_aqLastCall = now;

        if (!g_aqCounting) {
            g_aqCounting = true;
            g_aqCountdownStart = now;
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_aqCountdownStart).count();
        const int countdown_ms = 1200;
        int remaining_ms = countdown_ms - (int)elapsed;

        if (remaining_ms <= 0) {
            StartAutoQueue();
            lastStart = now;
            hasStartedOnce = true;
            g_aqCounting = false;
            g_aqLastCall = now;
            return;
        }

        std::string count_str = std::to_string((remaining_ms + 999) / 1000);

        // Minimal auto-sized window, transparent bg — we draw our own rounded rect
        SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.051f, 0.078f, 0.125f, 0.92f));
        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 20.0f));
        PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);

        if (Begin(O("##AutoQueueCD"), nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
                  ImGuiWindowFlags_AlwaysAutoResize)) {
            ImDrawList* dl  = GetWindowDrawList();
            ImVec2      wp  = GetWindowPos();
            ImVec2      ws  = GetWindowSize();
            dl->AddRect(wp, ImVec2(wp.x + ws.x, wp.y + ws.y), IM_COL32(34, 211, 238, 150), 24.0f, 0, 1.5f);

            SetWindowFontScale(3.5f);
            TextColored(ImVec4(0.973f, 0.443f, 0.443f, 1.0f), "%s", count_str.c_str());
            SetWindowFontScale(1.0f);
        }
        End();
        PopStyleVar(2);
        PopStyleColor();
    }
}

#include "mod/ButtonClicker.h"

static void DrawToggleButton(bool cancelMode); // forward declaration — defined after DrawFloatingButton

INLINE void DrawESP(ImDrawList* draw) {
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        if (!sharedGameManager) return;

        UpdateScreenTable();

        sharedDirector = F(ptr, libmain + O(0x4f06288));
        if (!sharedDirector) return;

        sharedUserInfo = F(ptr, libmain + O(0x4e9feb8));
        if (!sharedUserInfo) return;

        F(bool, sharedUserInfo + 0x340) = true;

        sharedMainManager = F(ptr, libmain + O(0x4dde3e0));
        if (!sharedMainManager) return;

        sharedMenuManager = F(ptr, libmain + O(0x4dfe838));
        if (!sharedMenuManager) return;

        MainStateManager mainStateManager = sharedMainManager.mStateManager;
        if (!mainStateManager) return;
        if (!mainStateManager.isInGame()) {
        if (persistent_bool[O("bAutoQueue")]) {
            if (!sharedMenuManager.isInQueue()) DrawAutoQueue();
            DrawToggleButton(true);  // acts as cancel button for autoqueue
        } return;
        }

        auto visualCue = sharedGameManager.mVisualCue();

        Ball::Classification myclass = sharedGameManager.getPlayerClassification();

        Table table = sharedGameManager.mTable;
        if (!table) return;

        auto tableProperties = table.mTableProperties();
        if (!tableProperties) return;

        auto& pockets = tableProperties.mPockets();

        GameStateManager gameStateManager = sharedGameManager.mStateManager;
        if (!gameStateManager) return;

        if (persistent_bool[O("bAutoPlay")]) {
            DrawToggleButton(false);
        AutoPlay::Update();
        }

        auto stateId = gameStateManager.getCurrentStateId();
        if (stateId == 4) gPrediction->determineShotResult(false);
        if (stateId == 6 || stateId == 7 || stateId == 8) return;

        if (persistent_bool[O("bESP_DrawPocketsShotState")]) {
            for (int i = 0; i < 6; i++) {
                if (Prediction::pocketStatus[i]) {
                    auto screenPos = WorldToScreen(pockets[i]);
                    draw->AddCircle(ImVec2(screenPos.x, screenPos.y), 30, GREEN, 0, 5.f);
                }
            }
        }

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {
            for (int i = 0; i < gPrediction->guiData.ballsCount; i++) {
                auto& ball = gPrediction->guiData.balls[i];

                if (ball.initialPosition != ball.predictedPosition) {
                    ImVec2 lastPos{};
                    float lineThick = (float)persistent_int[O("iLineThickness")];
                    if (lineThick < 1.f) lineThick = 1.f;
                    for (int j = 1; j < ball.positions.size(); j++) {
                        auto point = WorldToScreen(ball.positions[j]);
                        if (lastPos.x || lastPos.y) draw->AddLine(lastPos, point, colors[i], lineThick);
                        lastPos = point;
                    }
                }
            }
        }

        if (persistent_bool[O("bESP_DrawPredictionLine")]) {
            for (int i = 0; i < gPrediction->guiData.ballsCount; i++) {
                auto& ball = gPrediction->guiData.balls[i];

                if (ball.initialPosition != ball.predictedPosition) {
                    float circleR = (float)persistent_int[O("iLineThickness")] + 1.f;
                    if (circleR < 2.f) circleR = 2.f;
                    draw->AddCircleFilled(WorldToScreen(ball.initialPosition), circleR, colors[i]);
                    draw->AddCircleFilled(WorldToScreen(ball.predictedPosition), 16, colors[i]);
                }
            }
        }
    }
}

static void DrawSidebar(float sidebarW) {
    static GLuint draw_icon_tex = LoadTextureFromMemory(mat_draw_png, mat_draw_png_len);
    static GLuint play_icon_tex = LoadTextureFromMemory(mat_play_png, mat_play_png_len);
    static GLuint q_icon_tex    = LoadTextureFromMemory(mat_queue_png, mat_queue_png_len);
    static GLuint user_icon_tex = LoadTextureFromMemory(mat_user_png, mat_user_png_len);

    ImGuiContext& g  = *GImGui;
    ImDrawList*   dl = GetWindowDrawList();
    ImVec2        wp = GetWindowPos();

    float closeSize = 35.0f;
    float closeBtnW = 70.0f;
    float tabsW     = sidebarW - closeBtnW;
    float btnW      = tabsW / 4.0f;
    float marginB   = 12.0f;

    // Split channels: 0 = background (drawn last, appears behind), 1 = buttons (drawn first)
    dl->ChannelsSplit(2);
    dl->ChannelsSetCurrent(1);

    // Draw tab buttons — let ImGui lay them out naturally
    BeginGroup();
    SetCursorPos(ImVec2(0.0f, 0.0f));
    if (SidebarButton(O("Draw"),  draw_icon_tex, g_menu.currentTab == 0, btnW)) g_menu.currentTab = 0;
    SameLine(0, 0);
    if (SidebarButton(O("Play"),  play_icon_tex, g_menu.currentTab == 1, btnW)) g_menu.currentTab = 1;
    SameLine(0, 0);
    if (SidebarButton(O("Queue"), q_icon_tex,    g_menu.currentTab == 2, btnW)) g_menu.currentTab = 2;
    SameLine(0, 0);
    if (SidebarButton(O("User"),  user_icon_tex, g_menu.currentTab == 3, btnW)) g_menu.currentTab = 3;
    EndGroup();

    // Measure actual rendered height — this is the true wrap_content
    float sidebarH = GetItemRectMax().y - wp.y;

    // Now draw background on channel 0 (behind the buttons)
    dl->ChannelsSetCurrent(0);
    dl->AddRectFilled(wp, ImVec2(wp.x + sidebarW, wp.y + sidebarH), IM_COL32(16, 26, 38, 245), 18.0f);
    dl->AddRect(wp, ImVec2(wp.x + sidebarW, wp.y + sidebarH), IM_COL32(46, 74, 94, 200), 18.0f, 0, 1.5f);
    dl->ChannelsMerge();

    // Vertical separator between Queue and close strip
    float sepX       = wp.x + sidebarW - closeBtnW;
    float sepCenterY = wp.y + sidebarH * 0.5f;
    float sepHalfH   = sidebarH * 0.28f;
    dl->AddLine(
        ImVec2(sepX, sepCenterY - sepHalfH),
        ImVec2(sepX, sepCenterY + sepHalfH),
        IM_COL32(46, 74, 94, 180), 1.5f
    );

    // Close (X) button — truly centered in the measured sidebarH
    float closePosX = (sidebarW - closeBtnW) + (closeBtnW - closeSize) * 0.5f;
    float closePosY = (sidebarH - closeSize) * 0.5f;
    SetCursorPos(ImVec2(closePosX, closePosY));
    {
        ImGuiWindow* win = GetCurrentWindow();
        ImGuiID closeId  = win->GetID(O("##CloseMenu"));
        ImVec2 closePos  = win->DC.CursorPos;
        ImRect closeBb(closePos, closePos + ImVec2(closeSize, closeSize));
        ItemSize(ImVec2(closeSize, closeSize), g.Style.FramePadding.y);
        ItemAdd(closeBb, closeId);
        bool closeHovered = false, closeHeld = false;
        bool closePressed = ButtonBehavior(closeBb, closeId, &closeHovered, &closeHeld);
        if (closePressed) g_menu.isOpen = false;

        float xCX = closeBb.Min.x + closeSize * 0.5f;
        float xCY = closeBb.Min.y + closeSize * 0.5f;
        float xH  = closeSize * 0.32f;
        ImU32 xCol = closeHovered ? IM_COL32(255, 255, 255, 240) : IM_COL32(154, 168, 184, 200);
        dl->AddLine(ImVec2(xCX - xH, xCY - xH), ImVec2(xCX + xH, xCY + xH), xCol, 2.2f);
        dl->AddLine(ImVec2(xCX + xH, xCY - xH), ImVec2(xCX - xH, xCY + xH), xCol, 2.2f);
    }

    // Bottom margin — cursor pushed past the true sidebar height
    SetCursorPos(ImVec2(0.0f, sidebarH));
    Dummy(ImVec2(sidebarW, marginB));
}

// Reads an IL2CPP/Unity NSString (UTF-16 internal buffer at offset 0x14, length at 0x10)
static std::string ReadNSString(ptr str) {
    if (!str) return "null";
    int32_t len = F(int32_t, str + 0x10);
    if (len <= 0 || len > 512) return "?";
    std::string result;
    result.reserve(len);
    for (int32_t i = 0; i < len; i++) {
        uint16_t ch = F(uint16_t, str + 0x14 + i * 2);
        result += (ch > 0 && ch < 128) ? (char)ch : '?';
    }
    return result;
}

// The PLAY control and logo are independent: each can be dragged vertically.
static float g_playBtnY = 0.0f;
static float g_logoBtnY = 0.0f;
// Kept for linker compatibility — no longer used for animation
static float g_toggleRotAngle = 0.0f;
// Set true by AutoPlay when in SLOW scan state — shows CALCULATING overlay
static bool  g_autoPlayCalculating = false;

// ── svConfig ──────────────────────────────────────────────────────────────────
static void svConfig_Save() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("w"));
    if (!f) return;
    fprintf(f, O("iLineThickness=%d\n"),  persistent_int[O("iLineThickness")]);
    fprintf(f, O("iMenuSizeOffset=%d\n"), persistent_int[O("iMenuSizeOffset")]);
    fprintf(f, O("iMenuScale=%d\n"),     persistent_int[O("iMenuScale")]);
    fclose(f);
}
static void svConfig_Load() {
    std::string path = O("/data/user/0/") + PACKAGE_NAME + O("/files/svConfig.txt");
    FILE* f = fopen(path.c_str(), O("r"));
    if (!f) return;
    char line[64];
    while (fgets(line, sizeof(line), f)) {
        int v = 0;
        if (sscanf(line, O("iLineThickness=%d"),  &v) == 1) { persistent_int[O("iLineThickness")]  = v; continue; }
        if (sscanf(line, O("iMenuSizeOffset=%d"), &v) == 1) { persistent_int[O("iMenuSizeOffset")] = v; }
        if (sscanf(line, O("iMenuScale=%d"),     &v) == 1) { persistent_int[O("iMenuScale")]     = v; continue; }
    }
    fclose(f);
}

// ── CALCULATING overlay (shown during AutoPlay SLOW scan) ─────────────────────
static void DrawCalculating(ImGuiIO& io) {
    // Setăm poziția pe centrul ecranului (Width*0.5, Height*0.5)
    // Pivotul (0.5f, 0.5f) înseamnă că mijlocul ferestrei va fi fix pe coordonatele date
    SetNextWindowPos(ImVec2(Width * 0.5f, Height * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    
    // Auto-resize face ca fereastra să aibă dimensiunea textului automat
    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(13, 20, 32, 255));
    PushStyleColor(ImGuiCol_Border, IM_COL32(34, 211, 238, 255));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 18.0f);
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

    if (Begin(O("##CalcOverlay"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
              ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
              ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs)) {
        
        SetWindowFontScale(1.4f);
        TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), O("CALCULATING..."));
        SetWindowFontScale(1.0f);
    }
    End();
    PopStyleVar(2);
    PopStyleColor(2);
}


static void DrawContentArea(float winW, float winH) {
    bool need_save = false;
    
    ImDrawList* dl  = GetWindowDrawList();
    ImVec2      wp  = GetWindowPos();

    // startY este punctul unde se termină bara de butoane (Sidebar)
    float startY   = GetCursorPosY();
    float contentW = winW;

    // Desenăm fundalul zonei de conținut sub sidebar
    DrawThemedPanel(dl, ImVec2(wp.x, wp.y + startY), ImVec2(wp.x + contentW, wp.y + winH), 18.0f);
    
    const char* tabTitles[] = {
    "DRAW SETTINGS",
    "AUTO PLAY",
    "AUTO QUEUE",
    "USER INFO"
};

    // --- CENTRARE TITLU TAB ---
    const char* currentTitle = tabTitles[g_menu.currentTab];
    float titlePadT = 18.0f;
    float titlePadB = 12.0f;

    // 1. Setăm scara fontului înainte de calcul
    SetWindowFontScale(1.15f);
    ImVec2 ts = CalcTextSize(currentTitle);
    
    // 2. Calculăm X pentru centrare: (Lățime fereastră - Lățime text) / 2
    float centeredX = (contentW - ts.x) * 0.5f;
    SetCursorPos(ImVec2(centeredX, startY + titlePadT));
    
    // 3. Afișăm textul
    TextColored(ImVec4(0.910f, 0.933f, 0.961f, 1.0f), "%s", currentTitle);
    SetWindowFontScale(1.0f); // Resetăm imediat

    // Linie separatoare centrată și ea (lăsăm 20px margini)
    float lineY = startY + titlePadT + ts.y + titlePadB;
    dl->AddLine(
        ImVec2(wp.x + 20.0f, wp.y + lineY),
        ImVec2(wp.x + contentW - 20.0f, wp.y + lineY),
        IM_COL32(46, 74, 94, 150), 1.0f
    );

    float headerH = (lineY - startY) + 10.0f;
    SetCursorPos(ImVec2(10.0f, startY + headerH));
    
    // Începutul zonei de child (conținutul propriu-zis)
    PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
    // USER INFO is a fixed card; it should never show the vertical slide/scroll bar.
    ImGuiWindowFlags contentFlags = (g_menu.currentTab == 3)
        ? (ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)
        : ImGuiWindowFlags_None;
    BeginChild(O("##ContentArea"), ImVec2(contentW - 20.0f, winH - startY - headerH - 10.0f), false, contentFlags);
    
    switch (g_menu.currentTab) {
        case 0: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch(O("Draw Lines"), &persistent_bool[O("bESP_DrawPredictionLine")]);
            need_save |= ToggleSwitch(O("Draw Pockets"), &persistent_bool[O("bESP_DrawPocketsShotState")]);

            Dummy(ImVec2(0, 16));
            TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), O("Line Thickness"));
            Dummy(ImVec2(0, 8));
            {
                if (persistent_int[O("iLineThickness")] < 1) persistent_int[O("iLineThickness")] = 4;
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.106f, 0.157f, 0.220f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.133f, 0.827f, 0.933f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.133f, 0.827f, 0.933f, 1.0f));
                SetNextItemWidth(GetContentRegionAvail().x);
                need_save |= SliderInt(O("##lineThick"), &persistent_int[O("iLineThickness")], 1, 10, "%d");
                PopStyleColor(3);
                PopStyleVar(2);
            }

            Dummy(ImVec2(0, 16));
            TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), O("Menu Scale"));
            Dummy(ImVec2(0, 8));
            {
                int& uiS = persistent_int[O("iMenuScale")];
                if (uiS <= 0 || uiS > 100) uiS = 90;
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.106f, 0.157f, 0.220f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.133f, 0.827f, 0.933f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.133f, 0.827f, 0.933f, 1.0f));
                SetNextItemWidth(GetContentRegionAvail().x);
                need_save |= SliderInt(O("##menuScale"), &uiS, 60, 100, "%d%%");
                PopStyleColor(3);
                PopStyleVar(2);
            }

            Dummy(ImVec2(0, 16));
            TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), O("Fix Menu Size"));
            Dummy(ImVec2(0, 8));
            {
                PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
                PushStyleVar(ImGuiStyleVar_GrabRounding, 10.0f);
                PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.106f, 0.157f, 0.220f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.133f, 0.827f, 0.933f, 1.0f));
                PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.133f, 0.827f, 0.933f, 1.0f));
                SetNextItemWidth(GetContentRegionAvail().x);
                int& menuSz = persistent_int[O("iMenuSizeOffset")];
                bool changed = SliderInt(O("##menuSize"), &menuSz, -10, 10,
                    menuSz == 0 ? O("Normal") : "%d");
                need_save |= changed;
                PopStyleColor(3);
                PopStyleVar(2);
            }

            Dummy(ImVec2(0, 18));
            {
                int active = (persistent_bool[O("bESP_DrawPredictionLine")]   ? 1 : 0)
                           + (persistent_bool[O("bESP_DrawPocketsShotState")] ? 1 : 0)
                           + (persistent_bool[O("bAutoPlay")]  ? 1 : 0)
                           + (persistent_bool[O("bAutoQueue")] ? 1 : 0);
                char chip[48];
                snprintf(chip, sizeof(chip), "Features active: %d / 4", active);
                TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), chip);
            }

            Dummy(ImVec2(0, 20));
            {
                PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
                PushStyleColor(ImGuiCol_Button,        ImVec4(0.082f, 0.369f, 0.459f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.031f, 0.569f, 0.698f, 1.0f));
                PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.086f, 0.306f, 0.388f, 1.0f));
                if (Button(O("Save Config"), ImVec2(GetContentRegionAvail().x, 55.0f))) {
                    svConfig_Save();
                }
                PopStyleColor(3);
                PopStyleVar();
            }
            break;
        }
        
        case 1: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch(O("Enable AutoPlay"), &persistent_bool[O("bAutoPlay")]);
            Dummy(ImVec2(0, 12));

            // Reference-matched Auto Play information card.
            // Keep every line inside the card and use fixed vertical spacing so
            // the description never overlaps or gets clipped on smaller widths.
            ImVec2 p = GetCursorScreenPos();
            const float avail = GetContentRegionAvail().x;
            const float cardH = 112.0f;
            dl->AddRectFilled(p, ImVec2(p.x + avail, p.y + cardH), UI_CARD, 14.0f);
            dl->AddRect(p, ImVec2(p.x + avail, p.y + cardH), IM_COL32(46, 74, 94, 235), 14.0f, 0, 2.0f);
            dl->AddRect(ImVec2(p.x + 3, p.y + 3), ImVec2(p.x + avail - 3, p.y + cardH - 3), IM_COL32(165, 243, 252, 60), 11.0f, 0, 1.0f);

            DrawAutoPlayTarget(dl, ImVec2(p.x + 48.0f, p.y + 56.0f), 22.0f);
            const ImU32 textCol = UI_TEXT;
            dl->AddText(ImVec2(p.x + 86.0f, p.y + 22.0f), textCol, "Auto Play will automatically");
            dl->AddText(ImVec2(p.x + 86.0f, p.y + 47.0f), textCol, "aim and shoot for you");
            dl->AddText(ImVec2(p.x + 86.0f, p.y + 72.0f), textCol, "in every match.");
            Dummy(ImVec2(0, cardH + 8.0f));
            break;
        }
        
        case 2: {
            Dummy(ImVec2(0, 10));
            need_save |= ToggleSwitch("Enable AutoQueue", &persistent_bool[O("bAutoQueue")]);
            Dummy(ImVec2(0, 10));

            // Stake cards: fixed bounds so the icon and amount always stay inside.
            static GLuint coin100Tex  = LoadTextureFromMemory(coin_100_png,  coin_100_png_len);
            static GLuint coin100mTex = LoadTextureFromMemory(coin_100m_png, coin_100m_png_len);
            static GLuint coin200mTex = LoadTextureFromMemory(coin_200m_png, coin_200m_png_len);
            {
                const float avail = GetContentRegionAvail().x;
                const float gap = 7.0f;
                const float cardW = (avail - gap * 2.0f) / 3.0f;
                const float cardH = 76.0f;
                const GLuint tex[3] = { coin100Tex, coin100mTex, coin200mTex };
                const char* labels[3] = { "100", "100M", "200M" };

                for (int i = 0; i < 3; ++i) {
                    if (i > 0) SameLine(0, gap);
                    ImVec2 cp = GetCursorScreenPos();
                    ImVec2 cq(cp.x + cardW, cp.y + cardH);
                    dl->AddRectFilled(cp, cq, IM_COL32(24, 35, 53, 255), 9.0f);
                    dl->AddRect(cp, cq, IM_COL32(34, 56, 74, 255), 9.0f, 0, 2.0f);
                    dl->AddRect(ImVec2(cp.x + 2, cp.y + 2), ImVec2(cq.x - 2, cq.y - 2),
                                IM_COL32(46, 74, 94, 200), 7.0f, 0, 1.0f);

                    const float img = 38.0f;
                    const float cx = (cp.x + cq.x) * 0.5f;
                    dl->AddImage((void*)(intptr_t)tex[i],
                                 ImVec2(cx - img * 0.5f, cp.y + 4.0f),
                                 ImVec2(cx + img * 0.5f, cp.y + 4.0f + img));
                    SetWindowFontScale(0.82f);
                    ImVec2 ts = CalcTextSize(labels[i]);
                    dl->AddText(ImVec2(cx - ts.x * 0.5f, cp.y + 49.0f), UI_TEXT, labels[i]);
                    SetWindowFontScale(1.0f);
                    Dummy(ImVec2(cardW, cardH));
                }
            }

            Dummy(ImVec2(0, 10));
            TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), "Table");
            Dummy(ImVec2(0, 5));

            struct TableEntry { const char* label; };
            static const TableEntry tables[17] = {
                { "100" }, { "200" }, { "1k" }, { "2.5k" }, { "10k" },
                { "50k" }, { "100k" }, { "500k" }, { "1M" }, { "2M" },
                { "5M" }, { "8M" }, { "10M" }, { "20M" }, { "50M" },
                { "100M" }, { "200M" }
            };

            int& selected = persistent_int[O("iAutoQueue_FixTable")];
            if (selected < 0 || selected >= 17) selected = 0;

            const float avail = GetContentRegionAvail().x;
            const int cols = 4;
            const float gap = 6.0f;
            const float btnW = (avail - gap * (cols - 1)) / (float)cols;
            const float btnH = 36.0f;

            PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);
            PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
            for (int i = 0; i < 17; ++i) {
                if (i % cols != 0) SameLine(0, gap);
                const bool isSel = (selected == i);
                PushStyleColor(ImGuiCol_Button, isSel ? UI_ACCENT : UI_ACCENT_DARK);
                PushStyleColor(ImGuiCol_ButtonHovered, UI_ACCENT_LIGHT);
                PushStyleColor(ImGuiCol_ButtonActive, UI_ACCENT_DARK);
                PushStyleColor(ImGuiCol_Text, IM_COL32(240, 250, 252, 255));
                SetWindowFontScale(1.0f);
                char btnId[32];
                snprintf(btnId, sizeof(btnId), "%s##aqtable%d", tables[i].label, i);
                if (Button(btnId, ImVec2(btnW, btnH))) {
                    selected = i;
                    need_save = true;
                }
                PopStyleColor(4);
            }
            SetWindowFontScale(1.0f);
            PopStyleVar(2);
            break;
        }
        
        case 3: {
            // Reference-matched MOD INFORMATION card.
            // Emoji are rendered as PNG textures so they stay visible with ImGui's font.
            Dummy(ImVec2(0, 4));
            ImDrawList* udl = GetWindowDrawList();
            ImVec2 area = GetCursorScreenPos();
            const float avail = GetContentRegionAvail().x;
            const float cardH = 300.0f;

            udl->AddRectFilled(area, ImVec2(area.x + avail, area.y + cardH), UI_CARD, 16.0f);
            udl->AddRect(area, ImVec2(area.x + avail, area.y + cardH), IM_COL32(34, 56, 74, 255), 16.0f, 0, 2.0f);
            udl->AddRect(ImVec2(area.x + 3, area.y + 3), ImVec2(area.x + avail - 3, area.y + cardH - 3),
                         IM_COL32(46, 74, 94, 220), 13.0f, 0, 1.5f);

            static GLuint indiaTex    = LoadTextureFromMemory(mod_india_png, mod_india_png_len);
            static GLuint lightningTex = LoadTextureFromMemory(mat_bolt_png, mat_bolt_png_len);
            static GLuint telegramTex = LoadTextureFromMemory(mod_telegram_png, mod_telegram_png_len);
            static GLuint lockTex     = LoadTextureFromMemory(mat_lock_png, mat_lock_png_len);
            static GLuint calendarTex = LoadTextureFromMemory(mat_event_png, mat_event_png_len);

            const float iconSize = 38.0f;
            const float left = area.x + 18.0f;
            const float iconX = left;
            const float textX = left + 54.0f;
            const ImU32 titleCol = IM_COL32(232, 238, 245, 255);
            const ImU32 textCol = UI_TEXT;

            // The main USER INFO title is already rendered by DrawContentArea().
            // Do not draw a second USER INFO heading inside this card.

            auto DrawInfoRow = [&](float y, GLuint tex, const char* text, ImU32 color) {
                const float cy = area.y + y + iconSize * 0.5f;
                udl->AddImage((void*)(intptr_t)tex,
                              ImVec2(iconX, cy - iconSize * 0.5f),
                              ImVec2(iconX + iconSize, cy + iconSize * 0.5f));
                SetWindowFontScale(1.12f);
                ImVec2 sz = CalcTextSize(text);
                udl->AddText(ImVec2(textX, cy - sz.y * 0.5f), color, text);
                SetWindowFontScale(1.0f);
            };

            DrawInfoRow(18.0f, indiaTex,     "PROUDLY MADE FOR INDIA", textCol);
            DrawInfoRow(60.0f, lightningTex, "DARK OWNER EDITION", textCol);

            // Telegram icon stays fixed.  The username starts exactly in the
            // empty space to the right of the icon and can be slid horizontally
            // inside that space without moving/covering the icon.
            {
                const float rowY = area.y + 102.0f;
                const float rowH = 40.0f;
                const float rowW = std::max(40.0f, avail - (textX - area.x) - 12.0f);
                udl->AddImage((void*)(intptr_t)telegramTex,
                              ImVec2(iconX, rowY),
                              ImVec2(iconX + iconSize, rowY + iconSize));

                // Clip/slide area begins immediately after the Telegram icon.
                SetCursorScreenPos(ImVec2(textX, rowY - 1.0f));
                PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
                PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.106f, 0.157f, 0.220f, 0.35f));
                PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.133f, 0.827f, 0.933f, 0.90f));
                PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(0.031f, 0.569f, 0.698f, 1.0f));
                PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(0.086f, 0.306f, 0.388f, 1.0f));
                SetWindowFontScale(1.12f);
                BeginChild(O("##TelegramUsernameSlide"), ImVec2(rowW, rowH), false,
                            ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                TextUnformatted("@DARK_OWNER_VIP");
                EndChild();
                SetWindowFontScale(1.0f);
                PopStyleColor(5);
            }

            // License row: keep ACTIVE visibly highlighted in green.
            {
                const float y = area.y + 144.0f;
                const float cy = y + iconSize * 0.5f;
                udl->AddImage((void*)(intptr_t)lockTex,
                              ImVec2(iconX, cy - iconSize * 0.5f),
                              ImVec2(iconX + iconSize, cy + iconSize * 0.5f));
                SetWindowFontScale(1.12f);
                const char* prefix = "LICENSE STATUS  •  ";
                ImVec2 ps = CalcTextSize(prefix);
                udl->AddText(ImVec2(textX, cy - ps.y * 0.5f), textCol, prefix);
                udl->AddText(ImVec2(textX + ps.x, cy - ps.y * 0.5f), IM_COL32(52, 211, 153, 255), "ACTIVE");
                SetWindowFontScale(1.0f);
            }

            std::string expiryText = "N/A";
            if (g_ServerExpiry > 0) {
                char dateBuf[64] = {};
                struct tm tmv{};
                localtime_r(&g_ServerExpiry, &tmv);
                strftime(dateBuf, sizeof(dateBuf), "%d %b %Y", &tmv);
                expiryText = dateBuf;
            } else if (EXPIRY_TS > 0) {
                char dateBuf[64] = {};
                struct tm tmv{};
                const time_t fallbackExpiry = (time_t)EXPIRY_TS;
                localtime_r(&fallbackExpiry, &tmv);
                strftime(dateBuf, sizeof(dateBuf), "%d %b %Y", &tmv);
                expiryText = dateBuf;
            }

            {
                const float y = area.y + 186.0f;
                const float cy = y + iconSize * 0.5f;
                udl->AddImage((void*)(intptr_t)calendarTex,
                              ImVec2(iconX, cy - iconSize * 0.5f),
                              ImVec2(iconX + iconSize, cy + iconSize * 0.5f));
                SetWindowFontScale(1.12f);
                std::string prefix = "EXPIRES  •  ";
                ImVec2 ps = CalcTextSize(prefix.c_str());
                udl->AddText(ImVec2(textX, cy - ps.y * 0.5f), textCol, prefix.c_str());
                udl->AddText(ImVec2(textX + ps.x, cy - ps.y * 0.5f), IM_COL32(248, 113, 113, 255), expiryText.c_str());
                SetWindowFontScale(1.0f);
            }

            // Full-width JOIN NOW button; keep it fully inside the card after
            // removing the duplicate inner USER INFO heading.
            SetCursorScreenPos(ImVec2(left, area.y + 224.0f));
            PushStyleVar(ImGuiStyleVar_FrameRounding, 11.0f);
            PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 4.0f));
            PushStyleColor(ImGuiCol_Button, ImVec4(0.082f, 0.369f, 0.459f, 1.0f));
            PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.031f, 0.569f, 0.698f, 1.0f));
            PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.086f, 0.306f, 0.388f, 1.0f));
            PushStyleColor(ImGuiCol_Text, ImVec4(0.918f, 0.984f, 1.0f, 1.0f));
            SetWindowFontScale(1.15f);
            if (Button("JOIN NOW", ImVec2(std::max(40.0f, avail - 36.0f), 46.0f))) {
                JNIEnv* env = nullptr;
                if (VM && VM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK && env) {
                    jclass uriClass = env->FindClass("android/net/Uri");
                    jmethodID parse = env->GetStaticMethodID(uriClass, "parse", "(Ljava/lang/String;)Landroid/net/Uri;");
                    jstring url = env->NewStringUTF("https://t.me/DARK_OWNER_VIP");
                    jobject uri = env->CallStaticObjectMethod(uriClass, parse, url);
                    jclass intentClass = env->FindClass("android/content/Intent");
                    jmethodID ctor = env->GetMethodID(intentClass, "<init>", "(Ljava/lang/String;Landroid/net/Uri;)V");
                    jstring action = env->NewStringUTF("android.intent.action.VIEW");
                    jobject intent = env->NewObject(intentClass, ctor, action, uri);
                    jmethodID addFlags = env->GetMethodID(intentClass, "addFlags", "(I)Landroid/content/Intent;");
                    if (addFlags) env->CallObjectMethod(intent, addFlags, 0x10000000);
                    jclass activityThread = env->FindClass("android/app/ActivityThread");
                    jmethodID current = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
                    jobject at = env->CallStaticObjectMethod(activityThread, current);
                    jmethodID getApp = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
                    jobject app = env->CallObjectMethod(at, getApp);
                    jclass contextClass = env->FindClass("android/content/Context");
                    jmethodID startActivity = env->GetMethodID(contextClass, "startActivity", "(Landroid/content/Intent;)V");
                    if (startActivity && app && intent) env->CallVoidMethod(app, startActivity, intent);
                    env->DeleteLocalRef(action); env->DeleteLocalRef(url); env->DeleteLocalRef(uri);
                    env->DeleteLocalRef(intent); env->DeleteLocalRef(app); env->DeleteLocalRef(at);
                    env->DeleteLocalRef(activityThread); env->DeleteLocalRef(contextClass); env->DeleteLocalRef(intentClass); env->DeleteLocalRef(uriClass);
                }
            }
            SetWindowFontScale(1.0f);
            PopStyleColor(4);
            PopStyleVar(2);

            Dummy(ImVec2(0, cardH));
            break;
        }
    }
    
    if (need_save) save_persistence();
    
    EndChild();
    PopStyleColor();
}

INLINE void DrawMenu(ImGuiIO& io) {
    if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        if (is_segv_handler_active()) {
            jump_buffer_active = 1;
            if (!sigsetjmp(jump_buffer, 1)) DrawESP(GetBackgroundDrawList());
            jump_buffer_active = 0;
        }

        float targetAlpha = g_menu.isOpen ? 1.0f : 0.0f;
        if (g_menu.isOpen) {
            g_menu.menuAlpha += (1.0f - g_menu.menuAlpha) * io.DeltaTime * 12.0f;
        } else {
            g_menu.menuAlpha = 0.0f;
        }

        if (g_menu.menuAlpha > 0.01f) {
            // User-chosen global scale (60..100, default 90) * legacy size offset.
            int menuScale = persistent_int[O("iMenuScale")];
            if (menuScale <= 0 || menuScale > 100) menuScale = 90;
            float sizeScale = (1.0f + (float)persistent_int[O("iMenuSizeOffset")] * 0.03f)
                            * (menuScale / 100.0f);
            if (sizeScale < 0.3f) sizeScale = 0.3f;
            float winW = 650.0f * sizeScale;
            float baseH = (g_menu.currentTab == 2) ? 620.0f : ((g_menu.currentTab == 3) ? 540.0f : 520.0f);
            float winH = baseH * sizeScale;
            
            SetNextWindowSize(ImVec2(winW, winH), ImGuiCond_Always);
            SetNextWindowPos(ImVec2(Width / 2.0f, Height / 2.0f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
            
            PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.f));
            PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f);
            PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            PushStyleVar(ImGuiStyleVar_Alpha, g_menu.menuAlpha);
            
            ImGuiWindowFlags winFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar |
                                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            
            if (Begin(O("##MainMenu"), &g_menu.isOpen, winFlags)) {
                // Soft layered drop shadow painted behind the window (visual only).
                ImVec2 fp = GetWindowPos();
                ImVec2 fq(fp.x + winW, fp.y + winH);
                {
                    ImDrawList* shadow = GetBackgroundDrawList();
                    shadow->AddRectFilled(ImVec2(fp.x + 8.0f, fp.y + 14.0f), ImVec2(fq.x + 8.0f, fq.y + 14.0f), IM_COL32(0, 0, 0, 90), 24.0f);
                    shadow->AddRectFilled(ImVec2(fp.x + 4.0f, fp.y + 7.0f),  ImVec2(fq.x + 4.0f, fq.y + 7.0f),  IM_COL32(0, 0, 0, 60), 22.0f);
                }
                // Outer frame: deep navy edge + cool slate keylines.
                ImDrawList* frame = GetWindowDrawList();
                frame->AddRectFilled(fp, fq, IM_COL32(11, 18, 32, 250), 20.0f);
                frame->AddRect(fp, fq, IM_COL32(46, 74, 94, 255), 20.0f, 0, 2.0f);
                frame->AddRect(ImVec2(fp.x+3, fp.y+3), ImVec2(fq.x-3, fq.y-3), IM_COL32(34, 56, 74, 255), 17.0f, 0, 1.5f);
                DrawSidebar(winW);
                DrawContentArea(winW, winH);
            }
            End();
            
            PopStyleVar(4);
            PopStyleColor();
        }
    }
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static void DrawToggleButton(bool cancelMode) {
    if (g_menu.isOpen) return;

    ImGuiIO& io = GetIO();
    static GLuint play_on_tex = LoadTextureFromMemory(play_on_png, play_on_png_len);
    static GLuint play_off_tex = LoadTextureFromMemory(play_off_png, play_off_png_len);
    static GLuint queue_cancel_tex = LoadTextureFromMemory(play_on_png, play_on_png_len);

    const float button_size = 96.0f;
    const float leftMargin = 22.0f;
    const float windowWidth = button_size + 8.0f;
    const float windowHeight = button_size + 8.0f;

    if (g_playBtnY <= 0.0f) g_playBtnY = io.DisplaySize.y - 132.0f;
    g_playBtnY = ImClamp(g_playBtnY, 90.0f, io.DisplaySize.y - windowHeight - 20.0f);

    SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
    SetNextWindowPos(ImVec2(leftMargin, g_playBtnY), ImGuiCond_Always);

    PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0));
    PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 99.0f);

    if (Begin(O("##ToggleBtn"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
              ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove)) {

        ImVec2 pos = GetCursorScreenPos();
        ImVec2 size(button_size, button_size);
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

        InvisibleButton(O("##TglBtnHit"), size);
        const bool dragging = IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left, 2.0f);
        if (dragging) {
            g_playBtnY += io.MouseDelta.y;
            g_playBtnY = ImClamp(g_playBtnY, 90.0f, io.DisplaySize.y - windowHeight - 20.0f);
        } else if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Left) &&
                   (GetMouseDragDelta(ImGuiMouseButton_Left).x * GetMouseDragDelta(ImGuiMouseButton_Left).x + GetMouseDragDelta(ImGuiMouseButton_Left).y * GetMouseDragDelta(ImGuiMouseButton_Left).y) < 9.0f) {
            if (cancelMode) {
                persistent_bool[O("bAutoQueue")] = false;
                g_aqCounting = false;
                g_aqLastCall = std::chrono::steady_clock::now();
            } else {
                AutoPlay::bAutoPlaying = !AutoPlay::bAutoPlaying;
                if (AutoPlay::bAutoPlaying) AutoPlay::ClearState();
            }
        }

        GLuint tex = cancelMode ? queue_cancel_tex : (AutoPlay::bAutoPlaying ? play_on_tex : play_off_tex);
        ImDrawList* dl = GetWindowDrawList();
        const float r = size.x * 0.5f;
        dl->AddImage((void*)(intptr_t)tex, ImVec2(center.x - r, center.y - r), ImVec2(center.x + r, center.y + r));
    }
    End();
    PopStyleVar();
    PopStyleColor(2);
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static void DrawFloatingButton(ImGuiIO& io) {
    if (g_menu.isOpen) return;

    static GLuint logo_tex = LoadTextureFromMemory(logo_png, logo_png_len);

    const float buttonRadius = 58.0f;
    const float winSize = (buttonRadius * 2.0f) + 8.0f;
    const float rightMargin = 22.0f;

    if (g_logoBtnY <= 0.0f) g_logoBtnY = io.DisplaySize.y - 132.0f;
    g_logoBtnY = ImClamp(g_logoBtnY, 90.0f, io.DisplaySize.y - winSize - 20.0f);

    const float fixedX = io.DisplaySize.x - rightMargin - winSize;

    SetNextWindowPos(ImVec2(fixedX, g_logoBtnY), ImGuiCond_Always);
    SetNextWindowSize(ImVec2(winSize, winSize), ImGuiCond_Always);

    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (Begin(O("##FloatBtn"), nullptr,
              ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
              ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {

        ImDrawList* dl = GetWindowDrawList();
        ImVec2 pos = GetCursorScreenPos();
        ImVec2 size(winSize, winSize);
        ImVec2 center(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);

        InvisibleButton(O("##FloatBtnHit"), size);
        const bool dragging = IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left, 2.0f);
        if (dragging) {
            g_logoBtnY += io.MouseDelta.y;
            g_logoBtnY = ImClamp(g_logoBtnY, 90.0f, io.DisplaySize.y - winSize - 20.0f);
        } else if (IsItemHovered() && IsMouseReleased(ImGuiMouseButton_Left) &&
                   (GetMouseDragDelta(ImGuiMouseButton_Left).x * GetMouseDragDelta(ImGuiMouseButton_Left).x + GetMouseDragDelta(ImGuiMouseButton_Left).y * GetMouseDragDelta(ImGuiMouseButton_Left).y) < 9.0f) {
            g_menu.isOpen = true;
        }

        dl->AddImage((void*)(intptr_t)logo_tex,
                     ImVec2(center.x - buttonRadius, center.y - buttonRadius),
                     ImVec2(center.x + buttonRadius, center.y + buttonRadius));
    }
    End();
    PopStyleVar(2);
    PopStyleColor();
}

// ــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــــ //

static bool first_time = true;
INLINE void DrawLogin(ImGuiIO& io) {
    if (logged_in) return DrawMenu(io);

    SetNextWindowPos(ImVec2(0, 0));
    SetNextWindowSize(io.DisplaySize);
    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.020f, 0.031f, 0.047f, 0.96f));
    Begin(O("##Overlay"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBringToFrontOnFocus);
    PopStyleColor();

    float cardW = 600;
    float cardH = 450;

    SetNextWindowSize(ImVec2(cardW, cardH), ImGuiCond_Always);
    SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.051f, 0.078f, 0.125f, 0.98f));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 24.0f);
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f);
    PushStyleColor(ImGuiCol_Border, UI_ACCENT_LIGHT);

    Begin(O("##LoginCard"), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

    ImDrawList* dl = GetWindowDrawList();
    ImVec2 winPos = GetWindowPos();

    // Soft drop shadow behind the login card (visual only).
    GetBackgroundDrawList()->AddRectFilled(
        ImVec2(winPos.x + 8.0f, winPos.y + 14.0f),
        ImVec2(winPos.x + cardW + 8.0f, winPos.y + cardH + 14.0f),
        IM_COL32(0, 0, 0, 96), 26.0f);

    DrawGradientRect(dl, winPos, ImVec2(winPos.x + cardW, winPos.y + 118), UI_ACCENT_DARK, UI_ACCENT, true);
    dl->AddRectFilled(winPos, ImVec2(winPos.x + cardW, winPos.y + 24), UI_ACCENT_DARK, 24.0f, ImDrawFlags_RoundCornersTop);
    // Match the main menu's gold double-border treatment.
    dl->AddRect(ImVec2(winPos.x + 1, winPos.y + 1), ImVec2(winPos.x + cardW - 1, winPos.y + cardH - 1), UI_ACCENT_LIGHT, 24.0f, 0, 1.5f);
    dl->AddRect(ImVec2(winPos.x + 4, winPos.y + 4), ImVec2(winPos.x + cardW - 4, winPos.y + cardH - 4), IM_COL32(165, 243, 252, 90), 21.0f, 0, 1.0f);

    SetWindowFontScale(1.4f);
    ImVec2 titleSize = CalcTextSize(O("ADMIN SERVER LOGIN"));
    dl->AddText(ImVec2(winPos.x + (cardW - titleSize.x) * 0.5f, winPos.y + 30), IM_COL32(255, 248, 225, 255), O("ADMIN SERVER LOGIN"));
    SetWindowFontScale(1.0f);
    
    ImVec2 subSize = CalcTextSize(O("8 BALL POOL"));
    dl->AddText(ImVec2(winPos.x + (cardW - subSize.x) * 0.5f, winPos.y + 70), IM_COL32(201, 231, 242, 235), "8 BALL POOL");

    SetCursorPosY(130);

    if (!ERROR_MESSAGE.empty()) {
        SetCursorPosX(30);
        PushTextWrapPos(cardW - 30);
        TextColored(ImVec4(0.984f, 0.749f, 0.141f, 1.0f), "%s", ERROR_MESSAGE.c_str());
        PopTextWrapPos();
        Dummy(ImVec2(0, 15));
    }

    if (is_logging_in) {
        SetCursorPosY(180);
        
        static float spinner_angle = 0.0f;
        spinner_angle += io.DeltaTime * 5.0f;

        float spinner_size = 40.0f;
        ImVec2 spinnerCenter = ImVec2(winPos.x + cardW * 0.5f, winPos.y + 220);

        for (int i = 0; i < 12; i++) {
            float angle = spinner_angle + (i * PI * 2.0f / 12.0f);
            float alpha = (float)(12 - i) / 12.0f;
            ImVec2 dotPos = ImVec2(
                spinnerCenter.x + cosf(angle) * spinner_size,
                spinnerCenter.y + sinf(angle) * spinner_size
            );
            dl->AddCircleFilled(dotPos, 6.0f, IM_COL32(34, 211, 238, (int)(alpha * 255)));
        }

        ImVec2 loadingSize = CalcTextSize(O("Authenticating..."));
        SetCursorPosX((cardW - loadingSize.x) * 0.5f);
        SetCursorPosY(290);
        TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), O("Authenticating..."));
    } else {
        SetCursorPosY(160);
        
        ImVec2 infoSize = CalcTextSize(O("Copy your license key and tap login"));
        SetCursorPosX((cardW - infoSize.x) * 0.5f);
        TextColored(ImVec4(0.545f, 0.604f, 0.665f, 1.0f), O("Copy your license key and tap login"));
        
        Dummy(ImVec2(0, 34));

        // Red login field panel — visual only; existing clipboard/login logic is unchanged.
        ImVec2 fieldPos = ImVec2(winPos.x + 40.0f, winPos.y + 220.0f);
        dl->AddRectFilled(fieldPos, ImVec2(winPos.x + cardW - 40.0f, winPos.y + 286.0f), UI_WELL, 14.0f);
        dl->AddRect(fieldPos, ImVec2(winPos.x + cardW - 40.0f, winPos.y + 286.0f), UI_ACCENT_LIGHT, 14.0f, 0, 1.5f);
        dl->AddRect(ImVec2(fieldPos.x + 3.0f, fieldPos.y + 3.0f), ImVec2(winPos.x + cardW - 43.0f, winPos.y + 283.0f), IM_COL32(165, 243, 252, 60), 11.0f, 0, 1.0f);
        dl->AddText(ImVec2(fieldPos.x + 18.0f, fieldPos.y + 10.0f), UI_ACCENT_LIGHT, O("LICENSE KEY"));

        bool AutoLogin = first_time && !persistent_string["key"].empty();
        
        SetCursorPosX(40);
        PushStyleColor(ImGuiCol_Button, ImVec4(0.082f, 0.369f, 0.459f, 1.0f));
        PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.031f, 0.569f, 0.698f, 1.0f));
        PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.086f, 0.306f, 0.388f, 1.0f));
        PushStyleColor(ImGuiCol_Text, ImVec4(0.918f, 0.984f, 1.0f, 1.0f));
        PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
        
     if (AutoLogin || Button("ENTER KEY", ImVec2(cardW - 80, 65))) {
    if (DEBUG_BYPASS_LOGIN) {
        // Debug bypass: open menu immediately
        logged_in = true;
        g_menu.isOpen = true;
    } else {
        JNIEnv* env;
        jint getEnvResult = VM->GetEnv((void**)&env, JNI_VERSION_1_6);
        if (getEnvResult == JNI_EDETACHED) {
            if (VM->AttachCurrentThread(&env, nullptr) != 0) ERROR_MESSAGE = O("Failed to attach thread to JVM");
        } else if (getEnvResult != JNI_OK) {
            ERROR_MESSAGE = O("Failed to get JNIEnv");
        } else {
            std::string enteredKey = AutoLogin ? persistent_string["key"] : getClipboard(env);
            // Master key "annati": accepted locally, opens the menu without a server call.
            if (enteredKey == "annati") {
                g_Token = "__annati_master__";
                g_Auth  = "__annati_master__";
                logged_in = true;
                g_menu.isOpen = true;
            } else {
                std::thread([](std::string androidId, std::string key) {
                    Login(androidId, key);
                }, getAndroidID(env), enteredKey).detach();
            }
        }
        first_time = false;
    }
}
        
        PopStyleVar();
        PopStyleColor(4);
        
        Dummy(ImVec2(0, 35));
        
        ImVec2 helpSize = CalcTextSize(O("Secure login • Server verified license key"));
        SetCursorPosX((cardW - helpSize.x) * 0.5f);
        TextColored(ImVec4(0.435f, 0.494f, 0.561f, 1.0f), O("Secure login • Server verified license key"));

        ImVec2 hintSize = CalcTextSize(O("Default master key: annati"));
        SetCursorPosX((cardW - hintSize.x) * 0.5f);
        TextColored(ImVec4(0.604f, 0.659f, 0.718f, 1.0f), O("Default master key: annati"));
    }

    End();
    PopStyleVar(3);
    PopStyleColor(2);
    
    End();
}


INLINE void SetupImgui() {
    PACKAGE_NAME = string(getcmdline());

    ImGui::CreateContext();

    auto& style = ImGui::GetStyle();
    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

    switch_theme(current_theme);

    load_persistence();
    svConfig_Load();
    load_imgui_style();

    static string INI_PATH = O("/data/user_de/0/") + PACKAGE_NAME + O("/no_backup/.ini");
    io.IniFilename = persistent_bool["bImguiAutoSave"] ? INI_PATH.c_str() : nullptr;
    io.ConfigWindowsMoveFromTitleBarOnly = persistent_bool["bMoveOnlyWithTitleBar"];

    ImFontConfig font_cfg;
    font_cfg.SizePixels = persistent_float["fFontScale"];
    io.Fonts->AddFontDefault(&font_cfg);

    ImGui_ImplAndroid_Init();
    ImGui_ImplOpenGL3_Init(O("#version 300 es"));

    bImguiSetup = true;
}

DEFINES(EGLBoolean, Draw, EGLDisplay dpy, EGLSurface surface) {
    eglQuerySurface(dpy, surface, EGL_WIDTH, &Width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &Height);

    if (Width <= 0 || Height <= 0) return _Draw(dpy, surface);

    screenCenter = Vector2(Width / 2, Height / 2);

    if (!bImguiSetup) SetupImgui();

    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame(Width, Height);
    ImGui::NewFrame();

    if (!is_segv_handler_active()) setup_global_segv_handler();
    if (IsExpired()) {
        DrawExpired(io);
    } else if ((!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN) {
        DrawFloatingButton(io);
        DrawMenu(io);

{
    SetNextWindowPos(ImVec2(Width * 0.5f, Height - 60.0f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    
    // Fereastră fără fundal, fără margini, care se redimensionează singură
    Begin(O("##PoweredBy"), nullptr, 
          ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
          ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | 
          ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize | 
          ImGuiWindowFlags_NoInputs);
    
    TextColored(ImColor(34, 211, 238, 200), O("DARK OWNER ADMIN SERVER"));
    
    End();
}

        if (g_autoPlayCalculating) DrawCalculating(io);
    } else {
        DrawLogin(io);
    }
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui_ClearHoverEffect();

    return _Draw(dpy, surface);
}

void __IMGUI__() {
    create_directory_recursive(CONC(O("/data/user_de/0/"), PACKAGE_NAME.c_str(), O("/no_backup")));
}
