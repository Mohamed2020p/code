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
#include "StateManager.h"

struct MainManager : Class {
    Field<0x3b0, MainStateManager> mStateManager;
    
    MainManager(ptr instance = 0) : Class(instance), mStateManager(instance) {}

    operator bool() { return instance && this->isInstanceOf("MainManager"); }
};

static MainManager sharedMainManager;
