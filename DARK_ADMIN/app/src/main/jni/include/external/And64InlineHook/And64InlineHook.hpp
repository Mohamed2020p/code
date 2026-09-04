/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */
#pragma once

#include <cstdint>

#include "include/obfuscation.h"

EXPORT void *A64HookFunctionV(void *const symbol, void *const replace, void *const rwx, const uintptr_t rwx_size);
EXPORT void A64HookFunction(void *const symbol, void *const replace, void **result) asm(GET_N(6));
EXPORT bool A64RestoreHook(void *const symbol) asm(GET_N(7));
EXPORT int A64RestoreAllHooks() asm(GET_N(8));