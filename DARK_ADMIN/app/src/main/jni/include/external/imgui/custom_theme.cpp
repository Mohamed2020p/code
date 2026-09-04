/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */
#pragma once
#include "inc/custom_theme.h"

void StyleColorsCustom(ImGuiStyle* _style) {
    ImGuiStyle& style = _style ? *_style : ImGui::GetStyle();
    style = ImGuiStyle();
    style.WindowRounding = 18.0f;
    style.ChildRounding = 14.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 14.0f;
    style.ScrollbarRounding = 10.0f;
    style.GrabRounding = 10.0f;
    style.TabRounding = 10.0f;
    style.WindowPadding = ImVec2(10, 10);
    style.FramePadding = ImVec2(10, 7);
    style.ItemSpacing = ImVec2(8, 8);
    style.ScrollbarSize = 13.0f;
    // Obsidian + cyan accent (2026 UI refresh) - same palette as menu.h tokens.
    style.Colors[ImGuiCol_Text]                 = ImVec4(0.910f,0.933f,0.961f,1.00f); // #E8EEF5
    style.Colors[ImGuiCol_TextDisabled]         = ImVec4(0.420f,0.478f,0.541f,1.00f); // #6B7A8A
    style.Colors[ImGuiCol_WindowBg]             = ImVec4(0.075f,0.110f,0.165f,1.00f); // #131C2A
    style.Colors[ImGuiCol_ChildBg]              = ImVec4(0.094f,0.137f,0.208f,1.00f); // #182335
    style.Colors[ImGuiCol_PopupBg]              = ImVec4(0.075f,0.110f,0.165f,0.98f); // #131C2A
    style.Colors[ImGuiCol_Border]               = ImVec4(0.180f,0.290f,0.369f,0.75f); // #2E4A5E
    style.Colors[ImGuiCol_BorderShadow]         = ImVec4(0.000f,0.000f,0.000f,0.25f); // #000000
    style.Colors[ImGuiCol_FrameBg]              = ImVec4(0.106f,0.157f,0.220f,1.00f); // #1B2836
    style.Colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.133f,0.220f,0.290f,1.00f); // #22384A
    style.Colors[ImGuiCol_FrameBgActive]        = ImVec4(0.180f,0.290f,0.369f,1.00f); // #2E4A5E
    style.Colors[ImGuiCol_TitleBg]              = ImVec4(0.051f,0.078f,0.125f,1.00f); // #0D1420
    style.Colors[ImGuiCol_TitleBgActive]        = ImVec4(0.082f,0.369f,0.459f,1.00f); // #155E75
    style.Colors[ImGuiCol_MenuBarBg]            = ImVec4(0.051f,0.078f,0.125f,1.00f); // #0D1420
    style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.106f,0.157f,0.220f,0.45f); // #1B2836
    style.Colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.133f,0.827f,0.933f,0.85f); // #22D3EE
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.243f,0.859f,0.949f,1.00f); // #3EDBF2
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.031f,0.569f,0.698f,1.00f); // #0891B2
    style.Colors[ImGuiCol_CheckMark]            = ImVec4(0.133f,0.827f,0.933f,1.00f); // #22D3EE
    style.Colors[ImGuiCol_SliderGrab]           = ImVec4(0.133f,0.827f,0.933f,1.00f); // #22D3EE
    style.Colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.243f,0.859f,0.949f,1.00f); // #3EDBF2
    style.Colors[ImGuiCol_Button]               = ImVec4(0.082f,0.369f,0.459f,1.00f); // #155E75
    style.Colors[ImGuiCol_ButtonHovered]        = ImVec4(0.031f,0.569f,0.698f,1.00f); // #0891B2
    style.Colors[ImGuiCol_ButtonActive]         = ImVec4(0.086f,0.306f,0.388f,1.00f); // #164E63
    style.Colors[ImGuiCol_Header]               = ImVec4(0.110f,0.431f,0.549f,0.35f); // #1C6E8C @35%
    style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(0.133f,0.827f,0.933f,0.55f); // #22D3EE @55%
    style.Colors[ImGuiCol_HeaderActive]         = ImVec4(0.031f,0.569f,0.698f,0.70f); // #0891B2 @70%
    style.Colors[ImGuiCol_Tab]                  = ImVec4(0.051f,0.078f,0.125f,1.00f); // #0D1420
    style.Colors[ImGuiCol_TabHovered]           = ImVec4(0.031f,0.569f,0.698f,1.00f); // #0891B2
    style.Colors[ImGuiCol_TabActive]            = ImVec4(0.082f,0.369f,0.459f,1.00f); // #155E75
    style.Colors[ImGuiCol_TabUnfocused]         = ImVec4(0.051f,0.078f,0.125f,1.00f); // #0D1420
    style.Colors[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.106f,0.157f,0.220f,1.00f); // #1B2836
    style.Colors[ImGuiCol_Separator]            = ImVec4(0.180f,0.290f,0.369f,0.60f); // #2E4A5E @60%
    style.Colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.133f,0.827f,0.933f,0.85f); // #22D3EE @85%
    style.Colors[ImGuiCol_SeparatorActive]      = ImVec4(0.133f,0.827f,0.933f,1.00f); // #22D3EE
    style.Colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.020f,0.031f,0.047f,0.60f); // #05080C @60%
}
