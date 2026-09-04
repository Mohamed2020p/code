/*
 * DARK OWNER ADMIN SERVER
 * Crafted with passion & dedication.
 * Original Credits: DARK OWNER ADMIN SERVER
 * Telegram: @DARK_OWNER_VIP
 * Private Source & Support: DM @DARK_OWNER_VIP
 * Proudly Made for India
 */
#pragma once

// StartMatch hook declaration shared by main.cpp and AutoQueue.
// Keep the signature consistent with the existing AutoQueue call site.
static void (*_StartMatch)(void*, int, string, int, int, int, int, int, int, uint64_t, uint32_t) = nullptr;
static void StartMatch(void* manager, int arg2, string mode, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, uint64_t arg10, uint32_t arg11) {
    if (_StartMatch) {
        _StartMatch(manager, arg2, mode, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
    }
}

// AutoQueue is table-only. Last Selected and Smart modes are removed.
std::map<string, int64_t> modeBets = {
    {"M1", 50}, {"M2", 100}, {"M3", 500}, {"M4", 2500},
    {"M5", 10000}, {"M6", 50000}, {"M7", 100000}, {"M8", 250000},
    {"M9", 500000}, {"M10", 1000000}, {"M11", 2500000}, {"M12", 4000000},
    {"M13", 5000000}, {"M14", 10000000}, {"M15", 15000000},
    {"M16", 25000000}, {"M17", 100000000},
};

void StartAutoQueue() {
    LOGI("StartAutoQueue");
    if (!_StartMatch) {
        LOGI("AutoQueue: StartMatch hook is not ready");
        return;
    }
    if (!sharedMenuManager) {
        LOGI("AutoQueue: MenuManager not ready");
        return;
    }

    static const int64_t fixedTableValues[17] = {
        100, 200, 1000, 2500, 10000, 50000, 100000, 500000, 1000000,
        2000000, 5000000, 8000000, 10000000, 20000000, 30000000,
        50000000, 200000000
    };

    int selected = persistent_int["iAutoQueue_FixTable"];
    if (selected < 0 || selected >= 17) selected = 0;

    const int64_t requestedBet = fixedTableValues[selected];
    string selectedMode;
    int64_t selectedBet = 0;
    for (const auto& [modeName, bet] : modeBets) {
        if (bet == requestedBet) {
            selectedMode = modeName;
            selectedBet = bet;
            break;
        }
    }

    if (selectedMode.empty()) {
        LOGI("AutoQueue: no exact game mode mapping for requested table=%lld; match not started", requestedBet);
        return;
    }

    LOGI("AutoQueue: table=%lld -> mode=%s", selectedBet, selectedMode.c_str());
    _StartMatch((void*)sharedMenuManager.instance, 0, selectedMode, 0, 0, 0, 0, 0, 0, 0x7100000001ULL, 0xffffffffU);
}

DEFINE(int64_t, popMenuState, int64_t arg1, int64_t arg2, int32_t arg3, int64_t arg4) {
    LOGI("popMenuState arg1 %p, arg2 %p, arg3 %d, arg4 %p", arg1, arg2, arg3, arg4);
    return _popMenuState(arg1, arg2, arg3, arg4);
}

void PopMenuState(int stateId) {
    LOGI("PopMenuState %d", stateId);
    auto _popMenuState = M(int64_t, libmain + 0x3051f00, int64_t, int64_t, int32_t, int64_t); // MenuManager::popMenuState:withScene:
    _popMenuState(sharedMenuManager.instance, 0, stateId, sharedMenuManager.instance);
}