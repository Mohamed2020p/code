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

struct CCDirector : Class {

    CCDirector(ptr instance = 0) : Class(instance) {}

    operator bool() { return instance && this->isInstanceOf("CCDirector"); }
};

static CCDirector sharedDirector;
