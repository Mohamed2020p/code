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

struct State : Class {
    Field<0x18, int32_t> mStateId;

    State(ptr instance = 0) : Class(instance), mStateId(instance) {}

    operator bool() { return instance && isInstanceOf("State"); }
};