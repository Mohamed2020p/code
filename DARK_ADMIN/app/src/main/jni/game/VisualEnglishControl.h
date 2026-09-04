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
#include <Vector/Vectors.h>

struct VisualEnglishControl : Class {
    Field<0x3b0, Vec2d> mEnglish; // ShotSpin

    VisualEnglishControl(ptr instance = 0) : Class(instance), mEnglish(instance) {}
};
