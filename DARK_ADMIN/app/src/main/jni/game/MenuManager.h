/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */

#pragma once

#include "Types.h"

struct MenuManager : Class {
    MenuManager(ptr instance = 0) : Class(instance) {}

    // 39 game loading
    // 4 menu main
    // 82 first? popup

    int getMenuStateId() { return M(int, libmain + 0x305114c, ptr)(instance); }
    
    bool isInQueue() { return getMenuStateId() == 12; }

    operator bool() { return instance && this->isInstanceOf("MenuManager"); }
};

static MenuManager sharedMenuManager;
