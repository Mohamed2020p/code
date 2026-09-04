# 8BP_SRC_PUBLIC_BY_DARK_OWNER (3).zip — Static Reverse-Engineering & Analysis Report

> **Task type:** Educational static analysis (no dynamic execution, no modification of the original archive).
> **Analysis date:** 2026-09-04 (UTC).
> **Analyst:** Automated agent on Arena.ai (Agent Mode).
> **Subject archive:** `8BP_SRC_PUBLIC_BY_DARK_OWNER (3).zip` — an Android (ARM64) game-modification source tree branded **"DARK OWNER ADMIN SERVER"** targeting the mobile game **8 Ball Pool** (`com.miniclip.eightballpool`).

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Scope & Methodology](#2-scope--methodology)
3. [Archive Inventory](#3-archive-inventory)
4. [Project Structure](#4-project-structure)
5. [Architecture Overview](#5-architecture-overview)
6. [Source-Code Analysis](#6-source-code-analysis)
7. [Function Inventory](#7-function-inventory)
8. [Important Functions](#8-important-functions)
9. [Binary Analysis](#9-binary-analysis)
10. [Important Offsets & Locations (Addresses)](#10-important-offsets--locations-addresses)
11. [Strings & Constants](#11-strings--constants)
12. [Features](#12-features)
13. [UI/UX Analysis](#13-uiux-analysis)
14. [Colors & Visual Resources](#14-colors--visual-resources)
15. [Dependencies](#15-dependencies)
16. [Networking/Communication Analysis](#16-networkingcommunication-analysis)
17. [Configuration Analysis](#17-configuration-analysis)
18. [Security-Relevant Observations](#18-security-relevant-observations)
19. [Resource Analysis](#19-resource-analysis)
20. [Build System](#20-build-system)
21. [Timeline / Execution Flow](#21-timeline--execution-flow)
22. [Important Files](#22-important-files)
23. [Key Findings](#23-key-findings)
24. [Limitations](#24-limitations)
25. [Final Assessment](#25-final-assessment)

---

## 1. Executive Summary

The archive contains a **complete, buildable Android Studio/AndroidIDE project named `DARK_ADMIN`** (505 files, ~20.3 MiB extracted). It is the source code of a **paid/cheated "mod menu" for the game 8 Ball Pool** that is distributed under the brand **"DARK OWNER ADMIN SERVER"** (Telegram `@DARK_OWNER_VIP`).

High-level conclusions, all evidence-backed in the body of this report:

| Aspect | Finding | Confidence |
|---|---|---|
| Product type | Android game cheat/mod ("aim prediction lines", AutoPlay bot, AutoQueue bot) for 8 Ball Pool | **Confirmed** |
| App identity | `com.eightballpool.bp`, `versionCode 1`, `versionName "1.0"`, label "DARK OWNER ADMIN SERVER" | **Confirmed** |
| Native payload | `libMarkXit.so` (arm64-v8a only), an ImGui-over-OpenGL ES 3 overlay injected into the **game process** | **Confirmed** |
| Injection technique | Inline hooks (And64InlineHook) on game functions + PLT/GOT hooks (xhook) on `eglSwapBuffers` and the game's touch JNI entry points; code expects to run **inside** `com.miniclip.eightballpool` | **Confirmed (mechanism); how the .so enters the process is not present in the source (inferred loader/Zygisk/root)** |
| Monetization | License-key login against a remote key-auth panel (`axlmods.myvippanel.shop`) using libcurl; license stored on device; hard build expiry 2026-09-07 | **Confirmed** |
| Anti-analysis | String obfuscation (3 layers), runtime-hashed symbol lookup, decoy functions, VPN fail-closed check, `kill.h` tampering with the host process's `dlsym`/`malloc` resolution, ProGuard log-stripping | **Confirmed** |
| Origin/provenance | Derived from generic FPS-cheat templates (PUBG leftovers incl. `game=PUBG` POST field) and an **Intel-based ("cici") "Bloodstrike-static-v2"** build tree (paths leaked inside prebuilt xhook objects) | **Confirmed (from leaked strings)** |
| Binaries present | No final APK/`.so` shipped; 7 prebuilt static libraries (curl×2, OpenSSL×2 sets, xhook) + 7 relocatable objects + Gradle wrapper JARs | **Confirmed** |
| Malicious toward device owner? | No evidence of data theft, persistence beyond a mod menu, or device compromise. Potential harms: game ToS violation, auth-server data exposure (device fingerprint upload), network plaintext-auth risk | **High confidence** |

The project **cannot be built to a working cheat in a sandbox without the game itself**, and no attempt to run or build it was made (static analysis only).

---

## 2. Scope & Methodology

### 2.1 Scope

* Analysis covers **everything inside `8BP_SRC_PUBLIC_BY_DARK_OWNER (3).zip`** — all 505 files were enumerated, hashed and classified; every project source file was read; every prebuilt binary was inspected with ELF tooling and (where useful) AArch64 disassembly.
* The report **documents** implementation for educational understanding. It deliberately avoids operational "how to use/build/deploy this cheat" guidance and does not publish secrets (see §2.4).

### 2.2 Environment & tools

| Tool | Version/Notes | Used for |
|---|---|---|
| `unzip` | system | archive extraction into dedicated working dir `_8bp_extracted/` |
| `sha256sum`/`md5sum` | system | integrity hashing of archive and all 505 files |
| Python 3.11 | stdlib + custom scripts | inventory CSV, PNG header parsing, XOR decoding, archive statistics |
| `readelf`, `nm`, `ar`, `strings`, `objdump` | GNU binutils 2.40 (x86-64 host) | ELF headers, sections, symbols, string extraction |
| Capstone 5.0.x (pip, `--user`) | AArch64 disassembler engine | instruction-level verification of object files (`xhook.o`, `xh_core.o`) |
| manual source review | — | every first-party `.java/.cpp/.h/.hpp/.mk/.gradle/.xml` file |

### 2.3 Method

1. Located archive at repo root → verified type/size/hashes.
2. Extracted to `_8bp_extracted/` (original archive untouched).
3. Full recursive inventory → `8bp_analysis/FILE_INVENTORY.csv` (path, size, classification, SHA-256).
4. Read all first-party sources completely; skimmed/verified all third-party trees for versions and provenance.
5. Binary triage of prebuilt `.a/.o/.jar` artifacts (architecture, toolchain, strings, symbols, sections, selected disassembly with **file offsets measured from the actual files**).
6. Cross-referenced constants/offsets into structured tables (§9–§11).
7. Sanity/secret sweep for credentials and keys (§18).

### 2.4 Secret-handling policy

No live credentials exist in the archive (verified in §18). The license-login endpoint URL is documented because it is required to understand the system (and it is recoverable from any shipped build); user license **keys are not present** in the source and none are reproduced.

### 2.5 Terminology & address notation

* **File offset** — byte position inside a file on disk (hex, prefixed `file+0x…`).
* **RVA / libmain+0x…** — offset relative to the runtime base of the **game's** main native library (called `libmain` in the source), used by the mod to hook/read the game. These are **source-level constants**, not file offsets; the game binary is not part of the archive.
* **VA** — virtual address. Relocatable objects (`.o`) have section VAs of 0 by definition; final VAs exist only after linking.
* `O(...)` / `V(...)` — oxorany-style compile-time obfuscation macros in the source (numbers/strings are XOR-encrypted at compile time and decrypted at runtime).

---

## 3. Archive Inventory

### 3.1 Archive-level facts

| Property | Value |
|---|---|
| Filename | `8BP_SRC_PUBLIC_BY_DARK_OWNER (3).zip` |
| Size | 6,552,207 bytes (6.25 MiB) |
| MD5 | `c55993560057e4bb6ec3ad5de5f49bcb` |
| SHA-1 | `9856a2d6e043fff2f7b6b1adac87ff80408673f5` |
| SHA-256 | `e6271e2f89893ead8745aa140713a129a5d29383af378a37dc2f82082e90664c` |
| Extracted contents | **505 files**, **77 directories**, 21,281,793 bytes (~20.3 MiB) |
| Top-level entries | `DARK_ADMIN/` (the entire project), `Importent_Notice.txt` (branding notice; identical copy also embedded in 7 project subfolders) |

### 3.2 Content classification (extracted tree)

| Category | Count | Size (approx.) | Notes |
|---|---:|---:|---|
| C/C++ headers (`.h`) | 272 | 6.4 MiB | ~60 first-party + icon byte-arrays + third-party |
| C sources (`.c`) | 109 | 431 KiB | all third-party (libzip, Substrate, xhook, hde64) |
| C++ sources (`.cpp`) | 20 | 1.6 MiB | `main.cpp` + ImGui/curl glue/third-party |
| C++ headers (`.hpp`) | 15 | 1.0 MiB | json.hpp, Substrate, Vector, dlfcn, zygisk |
| Prebuilt static libs (`.a`) | 7 | 10.0 MiB | libcurl×2, libssl×2, libcrypto×2, libxhook×1 |
| Relocatable objects (`.o`) | 7 | 177 KiB | prebuilt xhook objects (incl. `.d` dep files) |
| Java sources | 1 | 8.5 KiB | `MainActivity.java` (only Java file) |
| PNG images | 8 | 359 KiB | 2 app drawables + 6 icons (many more icons embedded as `.h` arrays) |
| XML resources | 7 | 4.1 KiB | manifest, 2 styles, colors, strings, layout, vector drawable |
| Build/config (gradle/mk/properties/toml/pro/bat/jar…) | ~25 | ~110 KiB | Gradle wrapper, NDK makefiles, ProGuard rules |
| Text/docs (`Importent_Notice.txt`×8, LICENSE) | 8–9 | ~47 KiB | GPLv3 license + branding notices |
| OpenSSL config/perl/pkgconfig/libtool leftovers | ~24 | ~92 KiB | shipped alongside prebuilt OpenSSL |
| Empty junk (`.rartemp*`, `.bak`) | ~5 | 0 bytes | extraction artifacts of the RAR→ZIP pipeline |

Exact per-file data: **`8bp_analysis/FILE_INVENTORY.csv`** and **`8bp_analysis/HASHES_SHA256.txt`** (all 505 files, SHA-256 each).

### 3.3 File-type verification notes

* Every `.png` carries a valid PNG signature and IHDR (dimensions listed in §19).
* All `.a` archives are GNU `ar` archives of object files; member architectures verified via `readelf` (§9).
* Both gradle-wrapper JARs are ZIP/Java archives (standard wrapper shim classes).
* `.androidide/editor/openedFiles.json.bak` is **0 bytes** (AndroidIDE state file; nothing to analyze).
* `.rartemp*` files are empty leftovers from a WinRAR extraction on the author's side.

---

## 4. Project Structure

```
8BP_SRC_PUBLIC_BY_DARK_OWNER (3).zip
├── Importent_Notice.txt                     # branding notice (Telegram advert)
└── DARK_ADMIN/                              # Android application module root
    ├── build.gradle                         # root build script (AGP 8.2.1, jcenter, jitpack)
    ├── settings.gradle                      # include ':app'
    ├── gradle.properties                    # AndroidX, Jetifier, UTF-8 enforcement
    ├── gradle/libs.versions.toml            # version catalog (largely unused by app)
    ├── gradle/wrapper/*                     # Gradle 8.5 wrapper
    ├── gradlew / gradlew.bat                # wrapper scripts
    ├── LICENSE                              # GPLv3 (stock text)
    ├── .gitignore                           # standard Android gitignore
    ├── .androidide/editor/openedFiles.json.bak  # empty IDE state
    └── app/
        ├── build.gradle                     # app module: namespace com.eightballpool.bp,
        │                                    #   NDK-build, arm64-v8a, minify+shrink, libsu+Glide
        ├── proguard-rules.pro               # aggressive obfuscation / log stripping
        ├── gradlew(.bat), gradle/wrapper/*  # second, app-local wrapper copy
        └── src/main/
            ├── AndroidManifest.xml          # 5 permissions, 1 exported activity
            ├── java/com/eightballpool/bp/MainActivity.java   # launcher UI
            ├── res/                         # minimal resources (see §19)
            └── jni/                         # ===== the real payload =====
                ├── Android.mk               # module libMarkXit (BUILD_SHARED_LIBRARY)
                ├── Application.mk           # arm64-v8a, c++_static, C++20, release, PIE
                ├── main.cpp                 # JNI_OnLoad + hooks + JNI config exports
                ├── menu.h                   # ImGui overlay: login, menu, ESP, autoplay UI
                ├── game.h                   # aggregates game/ class model + game/inc logic
                ├── game/                    # C++ mirror of the game's ObjC++ object model
                │   ├── Types.h, Foundation.h           # Instance/Field/NSArray templates
                │   ├── GameManager.h, Table.h, Ball.h, VisualCue.h, VisualEnglishControl.h
                │   ├── StateManager.h, MainManager.h, MenuManager.h, UserInfo.h
                │   ├── Ruleset.h (+ "Ruleset old.h"), FrictionProperties.h, CCNode.h, CCDirector.h, State.h
                │   └── inc/                 # derived gameplay logic
                │       ├── Prediction.h / Prediction.fast.h (physics shot predictor)
                │       ├── Prediction.update.h / Prediction.update.offsets.h (stale variants)
                │       ├── AutoPlay.h, AutoAim.h, AutoQueue.h, ScreenTable.h
                │       ├── GameConstants.h, NumberUtils.h, game.h
                ├── mod/
                │   ├── keylogin.h           # license-key auth client (curl + JSON)
                │   ├── kill.h               # anti-analysis: hooks loader's dlsym/malloc
                │   ├── ButtonClicker.h      # synthetic tap driver (pocket nomination)
                │   ├── PowerSlider.h        # synthetic drag driver (shot power)
                │   ├── RangeSelector.h      # on-screen range picker (debug/calib)
                │   └── zygisk.hpp           # Zygisk API header (present, not wired in)
                ├── icons/                   # PNGs + PNGs-as-C-arrays (logo, tabs, coins…)
                └── include/
                    ├── includes.h, hook.h, pointers.h      # core macros, /proc maps parsing, hook glue
                    ├── java.h, input.h                      # JNI helpers, touch capture+injection
                    ├── logger.h, stacktrace.h, syscall.h    # logging, unwind, raw svc syscalls
                    ├── manual_dlsym.h                       # own ELF symbol resolver (GNU-hash)
                    ├── obfuscate.h, obfuscation.h, random_names.h, random_defs.h, features.h
                    ├── generator.h                          # C++20 coroutine helper (unused)
                    ├── input.old.h                          # stale variant
                    └── external/              # bundled third-party (see §15)
                        ├── And64InlineHook/   # ARM64 inline hooker (compiled)
                        ├── Substrate/         # Cydia Substrate MSHookFunction (compiled)
                        ├── xhook/             # PLT/GOT hooker: source + prebuilt .a/.o
                        ├── dlfcn/             # fdlopen/fdlsym/fdlclose shim (compiled)
                        ├── imgui/             # Dear ImGui 1.85 WIP + Android/GLES3 backends + theme
                        ├── curl/…             # prebuilt libcurl (arm64+armv7) + headers
                        │   └── openssl-…      # prebuilt libssl/libcrypto (arm64+armv7)
                        ├── json/              # nlohmann/json 3.11.2 (header-only, used)
                        ├── Vector/            # 2D/3D vector math (used)
                        ├── oxorany/, obfy/, obfusheader/    # compile-time obfuscation (2 used)
                        └── libzip/            # 1.0.1 sources (NOT referenced by build — dead weight)
```

**Dead/unreferenced content (confirmed by reading the build files):** `libzip/` sources, `xhook/src/*.c` (source tree — the build uses `xhook/xhook.c`), `armeabi-v7a` copies of curl/OpenSSL (ABI filter is `arm64-v8a` only), `input.old.h`, `Ruleset old.h`, `Prediction.update*.h`, `obfusheader/cpp_tests.cpp`, `obfy/main.cpp`, `AutoAim.h` (commented out in `game.h`), `features.h`, `generator.h`, `game/UserSettingsManager.h` (unused include in nothing — present but unreferenced), the WebSocket helper set inside `keylogin.h` (defined, never called).

---

## 5. Architecture Overview

### 5.1 The two layers

The project is really **two programs glued into one APK**:

1. **Java carrier app (`com.eightballpool.bp`)** — a minimal `AppCompatActivity` whose only visible function is a draggable red floating pill labeled "DARK OWNER ADMIN SERVER" that opens the author's Telegram channel. It never calls `setContentView` and declares no `System.loadLibrary` in the visible sources. Its role is branding/plausible deniability.
2. **Native payload (`libMarkXit.so`)** — this is the product. It is written to execute **inside the process of the game `com.miniclip.eightballpool`**:
   * discovers the game's main library by scanning `/proc/self/maps` for the game's package path and an ELF magic (`get8BPbase()`);
   * hooks the game's render loop (`eglSwapBuffers` via xhook) to draw an **ImGui overlay** on top of the game's OpenGL ES surface;
   * hooks game functions and touch-input JNI exports to read game state and inject input;
   * implements a **license-key login gate** (remote HTTP panel) before any cheat UI is shown.

How the `.so` enters the game process is **not implemented in the visible sources** — there is no injector code. Adjacent evidence suggests the intended delivery was via root tooling: `android.permission.ACCESS_SUPERUSER` in the manifest, `libsu` (topjohnwu) dependencies in Gradle, a **Zygisk API header** (`mod/zygisk.hpp`, unreferenced), and `kill.h`, which manipulates the *host* process's dynamic-linker behavior. Confidence: **medium** (inferred, not implemented here).

### 5.2 Internal architecture of `libMarkXit.so`

```
                ┌─────────────────────────── game process ───────────────────────────┐
                │                                                                      │
JNI_OnLoad ───► │  pthread __1__                                                       │
(from host      │    ├─ create /data/user_de/0/<pkg>/no_backup  (__IMGUI__)            │
 loader)        │    ├─ get8BPbase()  → libmain (game lib base)                        │
                │    ├─ __HOOKS__()                                                    │
                │    │    ├─ A64HookFunction(libmain+0x2d911e0, setActiveVisualCue)    │
                │    │    ├─ A64HookFunction(libmain+0x3068c94, StartMatch)            │
                │    │    └─ xhook eglSwapBuffers → Draw()                             │
                │    ├─ __INPUT__()                                                    │
                │    │    └─ xhook Java_com_miniclip_input_MCInput_nativeTouches*      │
                │    └─ signal handlers (segv/abrt/ill/fpe/bus/sys/trap)               │
                │                                                                      │
render frame ─► │  Draw() [every eglSwapBuffers]                                       │
                │    ├─ SetupImgui() (once): fonts, theme, persistence load            │
                │    ├─ expired? → DrawExpired (kill-switch)                           │
                │    ├─ not logged in? → DrawLogin → Login(androidID, clipboard key)   │
                │    └─ logged in:                                                     │
                │         ├─ DrawFloatingButton (logo) / DrawToggleButton (play/stop)  │
                │         ├─ DrawMenu (4 tabs) ──► persistence (JSON)                  │
                │         └─ DrawESP ──► Prediction engine ──► overlay lines/circles   │
                │              ├─ AutoPlay::Update → ScanFast/ScanSlow → takeShot      │
                │              │     (writes aim angle/power, calls game shot fn,      │
                │              │      drives ButtonClicker for pocket nomination)      │
                │              └─ AutoQueue → StartAutoQueue → hooked StartMatch()     │
                │                                                                      │
keylogin ─────► │  Login(): VPN guard → device fingerprint (UUID(key+aid+model+brand)) │
                │         → HTTPS POST (libcurl, TLS-verify OFF) → JSON token/expiry   │
                │         → g_Token/g_Auth booleans unlock UI                          │
                └──────────────────────────────────────────────────────────────────────┘
```

### 5.3 Data flow

* **Game state → overlay:** the `setActiveVisualCue` hook captures the live `GameManager*`. From it the predictor reads `Table → Ball[] / TableProperties / VisualCue(aim) / VisualEnglishControl(spin) / StateManager`, simulates the shot physics, and produces per-ball trajectories + pocket outcomes, which `DrawESP` renders through screen-mapped coordinates (`UpdateScreenTable` / `WorldToScreen` on a 1280×640 reference frame).
* **Overlay → game state:** AutoPlay writes `VisualGuide.mAimAngle` (+0x28) and `VisualCue.mPower` (+0x3b0), optionally writes `Ruleset.nominatedPocket` (+0x118), invokes the game's shot routine (`libmain+0x2dc0c58`), and injects synthetic taps via the hooked `nativeTouches*` chain. AutoQueue calls the game's `StartMatch` (hooked) with a mode string (`"M1"…"M17"`).
* **Persistence:** settings saved as JSON to the *host* app's `no_backup` dir (against adb backup) and `files/svConfig.txt`; license key under persistence key `"key"`.

### 5.4 Module relationships

`main.cpp` → includes everything through `include/includes.h` + `menu.h` → `menu.h` pulls in the game model (`game.h`), the login (`mod/keylogin.h`), icons and ImGui. The native build compiles **exactly 8 translation units** (`main.cpp` + 7 third-party `.cpp/.c` files per `Android.mk`); all first-party code is header-only (a common single-TU mod-menu pattern).

---

## 6. Source-Code Analysis

### 6.1 Java layer — `app/src/main/java/com/eightballpool/bp/MainActivity.java`

The only Java class. Breakdown:

| Member | Kind | Purpose |
|---|---|---|
| `floatingButtonView` | `private static View` | singleton guard for the overlay view |
| `onCreate(Bundle)` | override | calls `floatingbtn(this)`; **never sets a content view** |
| `floatingbtn(Activity)` | public static | builds a draggable pill (LinearLayout + heart icon + bold white text "DARK OWNER ADMIN SERVER") via `WindowManager.addView` using `TYPE_APPLICATION` (activity-scoped, so no overlay permission strictly needed); click opens `https://t.me/DARK_OWNER_VIP`; implements custom drag logic with 10 px threshold and 200 ms click distinction |
| `removeFloatingBtn(Activity)` | public static | cleanup |
| `onDestroy()` | override | calls remover |
| `createRoundedBackground()` | private static | `GradientDrawable`, radius 60, color `0xFFB00000` ("login red") |

Interesting details: comments are in **Romanian** ("Creează și afișează butonul floating…"), the package comment "// ← Schimbă cu package-ul tău" ("change to your package") proves it was copied from a shared tutorial/template. Uses `R.drawable.offline_heart` and `R.string.app_name`. `Activity`/`AppCompatActivity` imports are consistent; `androidx.appcompat` is required (see §15).

### 6.2 Native entry — `jni/main.cpp`

| Element | Detail |
|---|---|
| Includes | `include/includes.h`, `include/hook.h`, `include/input.h`, `include/java.h`, `include/obfuscation.h`, `include/manual_dlsym.h`, `include/random_defs.h`, `menu.h` |
| `DEFINES(int32_t, setActiveVisualCue, ptr arg1)` | hook body: stores `sharedGameManager = arg1`, chains to original `_setActiveVisualCue` |
| `__HOOKS__()` | installs 2 inline hooks (offsets `libmain+0x2d911e0`, `libmain+0x3068c94`) and the xhook `eglSwapBuffers`→`Draw` registration with regex `".*/com.miniclip.eightballpool/.*"`; logs failure of `xhook_refresh` |
| `__1__()` | startup thread: sleep 2 s → `PACKAGE_NAME = getcmdline()` → `__IMGUI__()` → sleep 10 s → `libmain = get8BPbase()` → hooks, input hooks, SEGV/abort handlers |
| `JNI_OnLoad` | stores `JavaVM* VM`; `CALL(0)` (invokes hidden export `_N(0)` = `__KILL__` via hashed dlsym, see §6.8); spawns `__1__`; returns `JNI_VERSION_1_6` |
| `Java_android_service_SurfaceView_onSendConfig` | JNI bridge mapping string keys (`ESP::LINES/POCKETS/STATES`, `AUTO::PLAY/QUEUE`) onto `persistent_bool`, then `save_persistence()` — implies a (not-included) companion UI app could remote-configure the mod |
| …`_onCanvasDraw` | empty stub |
| …`_getExpTime` | returns `g_ExpTime` ("N/A" or server expiry string) |
| …`_MenuColor` | returns `logged_in` boolean |

Note the deliberately misleading JNI package prefix `android.service.SurfaceView` — disguising callbacks as framework classes.

### 6.3 Overlay & cheat logic — `jni/menu.h` (1,296 lines)

* **`MenuState g_menu`** — menu open flag, current tab (0–3), widths, alpha/scale animation state, accent color.
* **Kill-switch constant:** `static const int64_t EXPIRY_TS = O(1788805800LL);` (= **2026-09-07 18:30:00 UTC**). If `time(nullptr) ≥ EXPIRY_TS`, `DrawExpired` shows "MOD EXPIRED / Beta Version Expired. Update on our Telegram @DARK_OWNER_VIP" and nothing else — every frame, all features dead.
* **Auth gate:** everywhere the code checks `(!g_Token.empty() && !g_Auth.empty() && g_Token == g_Auth) || DEBUG_BYPASS_LOGIN`. After a successful login these are both set to the literal `"1"` (§6.4), so **the comparison is cosmetic** — `logged_in` is the real state.
* **`DrawESP(ImDrawList*)`** — the core per-frame routine:
  1. refreshes singleton pointers from the game image (`sharedDirector` @`libmain+0x4f06288`, `sharedUserInfo` @`+0x4e9feb8`, `sharedMainManager` @`+0x4dde3e0`, `sharedMenuManager` @`+0x4dfe838`) and **forces boolean at `sharedUserInfo+0x340` to `true`** every frame (purpose unknown — plausibly a "show guideline/aim aid" client flag; *not determined precisely*);
  2. if not in game state 4 (player turn) and AutoQueue on → countdown UI → `StartAutoQueue()`;
  3. pulls `Table/TableProperties/pockets`, `GameStateManager`;
  4. when `stateId == 4`, runs `gPrediction->determineShotResult(false)`;
  5. draws pocket-state rings (green circle per pocket) and multi-segment trajectory polylines (`positions[]` per ball) + filled start/end dots with per-ball `colors[16]` palette; line thickness from `iLineThickness`.
* **UI structure:** `DrawLogin` (clipboard/license key → `Login()` thread) → `DrawFloatingButton` (logo button opens menu) → `DrawMenu` (animated window) → `DrawSidebar` (4 image tabs + close) → `DrawContentArea` (tab bodies) + `DrawToggleButton` (floating play/cancel) + `DrawCalculating` (AutoPlay SLOW-scan overlay).
* **`SetupImgui()`** — context, touch mode, theme, persistence + `svConfig` load, ini path under `/data/user_de/0/<pkg>/no_backup/.ini`, font scale 40 px default, Android+GLES3 backends (`#version 300 es`).
* **`Draw`** (the hooked `eglSwapBuffers`) — queries surface size, runs ImGui frame, calls `ImGui_ClearHoverEffect()`, then the original `_Draw(dpy, surface)`.
* **JOIN NOW button** — opens Telegram via raw JNI (`ActivityThread.currentActivityThread().getApplication().startActivity(VIEW intent)`) — no Java helper needed.
* Config helpers `svConfig_Save/Load` persist `iLineThickness`, `iMenuSizeOffset` to `/data/user/0/<pkg>/files/svConfig.txt`.

### 6.4 License client — `jni/mod/keylogin.h` (1,526 lines)

| Component | Behavior |
|---|---|
| `IsVpnOrTunnelActive(JNIEnv*)` | Three-stage fail-closed VPN check: `NetworkCapabilities.hasTransport(TRANSPORT_VPN)` on active network; same check across **all** networks; legacy `getNetworkInfo(TYPE_VPN)`; plus `/proc/net/dev` scan for `tun0-5/wg0-2/ppp0-1/ipsec0-1`. If detected → error **"Server Off"**, auth aborted |
| `xor_encrypt/xor_decrypt(data,key)` | repeating-key XOR |
| `base64_encode/decode` | own implementations |
| `ParseExpiryTime(exp)` | accepts unix ts / `YYYY-MM-DD HH:MM:SS` / `YYYY-MM-DD` (UTC via `timegm`) |
| `getDt(offsetSeconds)` | UTC **+3h** formatted timestamp (comment claims IST — actually +3, an author bug) |
| `gToken/decryptData` | XOR+base64 wrapper/unwrap helpers (`decryptData` parses `{"data": "..."}`) |
| WebSocket helpers (`generateWebSocketKey`, `createWebSocketFrame`, `parseWebSocketFrame`, `connectWebSocket`, `sendWebSocketMessage`) | complete minimal WS client — **dead code**, never referenced |
| `WriteMemoryCallback` | libcurl body sink (realloc-append) |
| `GetDarkDeviceUUID(env, seed)` | `UUID.nameUUIDFromBytes(seed)` hex string |
| `GetDarkBuildString(env, field)` | reads `android.os.Build.MODEL/BRAND` |
| `GetKeyRemainingTime()` | pretty "N Days HH MM SS" from `g_ServerExpiry` |
| **`Login(androidID, key)`** | (1) VPN guard; (2) build `serial = UUID(key + androidID + MODEL + BRAND)`; (3) `curl_easy_init` → URL = runtime-decoded `kPanelUrl` (XOR `0x5A`) = `https://axlmods.myvippanel.shop/useradmin/connect`; (4) POST `game=PUBG&user_key=<key>&serial=<serial>`; (5) parse JSON → `data.token`, `data.rng`, `data.EXP`; (6) accept iff `token` non-empty, `rng+30 > now`, expiry valid & future → sets `g_Token=g_Auth="1"`, `bValid=logged_in=true`; else server `reason` becomes `ERROR_MESSAGE`. **TLS peer/host verification disabled** (`CURLOPT_SSL_VERIFYPEER 0`, `VERIFYHOST 0`), but `CURLOPT_DEFAULT_PROTOCOL "https"` set. Follows redirects; timeouts 15 s/30 s |

Device fingerprinting + key check → the panel operator can bind keys to a device fingerprint and revoke them server-side.

### 6.5 Game object model — `jni/game/`

A hand-maintained C++ reflection layer over the game's Objective-C++ objects (8 Ball Pool is an ObjC/cocos2d-x-hybrid codebase — the source even retains the ObjC type-encoding comment `^{Ruleset=Ii{…}}` in `GameManager.h`).

| Type | File | Key contents |
|---|---|---|
| `Instance` / `Class` | `Types.h` | `instance` pointer; `isInstanceOf(name)` reads **class-name pointer at `+0x0`→`+0x10`** (vtable-adjacent ObjC++ layout); `Class::className()` at `+0x10` |
| `Field<offset,T>` | `Types.h` | templated typed field accessor (read/write at fixed offsets) |
| `NSArray/PNSArray/Array` | `Foundation.h` | `{Class,Count,Max,Data}` container mirrors with negative-index support |
| `GameManager` | `GameManager.h` | fields `_rules 0x3e0, mTable 0x3e8, mVisualCue 0x4b8, mVisualEnglishControl 0x4c8, mStateManager 0x508, mGameMode 0x5c0`; methods `getShotSpin()`, `getPlayerClassification()` (reads rules+0xC8 vector, offsets 0/4 by turn), `is9BallGame()`, `getPocketNominationMode()` (+0x68), `getNominatedPocket()`/`nominatePocket()` (+0x118) |
| `Ball` | `Ball.h` | physics at `position 0x20, velocity 0x30, radius 0x40, spin 0x48, mass 0x60, volume 0x68`; gameplay `classification 0xa0, state 0xa4`; enums `Classification{ANY=-1,CUE_BALL,SOLID,STRIPE,NINE_BALL_RULE,EIGHT_BALL}` and `State{DEFAULT=1,IN_POCKET,UNKNOWN,POTTED}` |
| `Table` / `TableProperties` | `Table.h` | `mTableProperties 0x3b0, _frictionProperties 0x3c0, mBalls 0x450, mTableCollisionBounds 0x588`; pockets pointer at properties+0x68; hard-coded dims 254×127, pocket radius 8.0 |
| `FrictionProperties` | `FrictionProperties.h` | 7 doubles incl. sliding 0.2, rolling 0.0111, velocity-reduction-sliding **196** (used by AutoPlay power estimation) |
| `VisualCue` / `VisualGuide` | `VisualCue.h` | `mVisualGuide 0x3a8, mPower 0x3b0`; guide `mAimAngle 0x28, mClassification 0xa0`; `getShotAngle()`, `getShotPower(strict)` with sqrt power-curve transform |
| `VisualEnglishControl` | `VisualEnglishControl.h` | `mEnglish 0x3b0` (spin) |
| `StateManager` family | `StateManager.h` | negative-index stack `mStateStack` (top state via `stateStack[-1]`); documented state ids: 3=menu, **4=my turn**, 6–8=rolling/opponent, 10=ended; `GameStateManager::isPlayerTurn()` |
| `MainManager` / `MenuManager` | `MainManager.h`, `MenuManager.h` | `MainManager.mStateManager 0x3b0`; `MenuManager::getMenuStateId()` calls game fn `libmain+0x305114c`; `isInQueue()` = state 12 |
| `UserInfo` | `UserInfo.h` | `coins 0x208, cash 0x210, DisplayName 0xc0, loginCountryCode 0xd0` |
| `Ruleset` | `Ruleset.h` | full offset map of the rules object (nomination mode 0x68, nominated pocket 0x118, classification vectors 0x98/0xB0/0xC8, players 0xE0) |
| `CCNode` / `CCDirector` | `CCNode.h`, `CCDirector.h` | cocos2d-x node (`_contentSize 0x2c8`, `_parent 0x2e0`) + singleton wrappers |

Signature gimmick: the mod validates objects at runtime by comparing the **in-memory class-name string** (`isInstanceOf("GameManager")`, `"Table"`, `"TenByFiveNarrowTableProperties"`…), which only works because 8 Ball Pool's ObjC++ objects carry readable type info — a robust anti-refactor trick.

### 6.6 Physics predictor — `game/inc/Prediction*.h`

* `Prediction` (in `Prediction.h`, 655 lines) is a full **event-driven simulator** cloned from the game's own functions (comments cite original addresses like `sub_1C29FA0`, `sub_1BF9ADC`): ball-ball collision time solver, ball-cushion line/point collisions, spin decay, friction model, pocket capture, 16-ball array `guiData.balls[MAX_BALLS_COUNT=16]`, static `float shotResult[50000]`.
* `Prediction.fast.h` (669 lines, actually used) adds the `Candidate` fast path: given a candidate target ball + pocket, it skips full simulation when the first hit isn't the target (`firstHitIsTarget`), and exposes `getPockets()`.
* `Prediction.update.h` / `Prediction.update.offsets.h` are **stale working copies** (decompiler outputs `FUN_02b1b2d0`, address notes `0x2b1b158`, `0x3606c80`, `0x3606d2c`, `0x1bfb66c`) kept for reference — not compiled.
* Trajectories feed both the visual ESP and AutoPlay's shot evaluation.

### 6.7 Bots — `AutoPlay.h`, `AutoQueue.h`, `AutoAim.h`, `ButtonClicker.h`, `PowerSlider.h`, `RangeSelector.h`

| Component | Behavior |
|---|---|
| `AutoPlay::Update()` | state machine `IDLE→SCANNING→NOMINATING→EXECUTING`. Skips while cue-power animation active (game fn `libmain+0x2de6f30`). Only fires on own turn (`isPlayerTurn()`) and only while `bAutoPlaying` (floating play button toggles it + `ClearState()`) |
| `AutoPlay::ScanFast()` | geometric ghost-ball solver: for every legal target ball × pocket, computes ghost-ball position, shot line, angle (`atan2`), distance-scored candidates sorted ascending; power from `sqrt(2·196·distance)` capped 666; validates with the predictor (first hit, pocket match, cue-ball-not-potted, 8-ball foul rules, 9-ball lowest-ball rule); on success → `Shoot()` |
| `AutoPlay::ScanSlow(step)` | brute-force angular sweep in 10-step-per-frame slices at 4 power levels {666,466,266,100}; used when FAST finds nothing; shows "CALCULATING…" overlay (`g_autoPlayCalculating`) |
| `Shoot()/takeShot()` | writes aim angle (`VisualGuide.mAimAngle`), power (`VisualCue.mPower` via `ShotPowerToPower` curve), predetermines result, then **invokes the game's shot function at `libmain+0x2dc0c58` with the cue object from `GameManager+0x3b0`**; pocket nomination handled by UI tap (`ButtonClicker`) at pocket screen position when rules require calling a pocket (modes 1/2) |
| `IsShotValid()` | re-validates the pending candidate against nomination, first-hit class, cue-ball scratch, and 8-ball legality |
| `AutoAim` (disabled) | older angular-sweep aim helper + 3-button overlay window; excluded from build (`game.h` comment) |
| `StartAutoQueue()` | maps 17 fixed table stakes to game modes `M1…M17` (50 → 100 000 000) and directly invokes the hooked **`StartMatch`** with magic args `(manager, 0, mode, 0,0,0,0,0,0, 0x7100000001, 0xffffffff)`; `PopMenuState(stateId)` helper calls `libmain+0x3051f00` |
| `ButtonClicker` | synthetic single tap: hooked `nativeTouchesBegin/End` with touch index 11, 0.15 s hold; used for pocket nomination taps |
| `PowerSlider` | scripted drag (start/end pos, IDLE→STARTING→MOVING→ENDING→RETURNING) with touch index 10, basing distance on desired power — implements power control by simulating the pull-back gesture |
| `RangeSelector` | full marquee/resizable overlay with undo/redo stacks (≤50) — developer calibration helper for screen regions (debug) |

### 6.8 Anti-analysis & stealth — `kill.h`, `manual_dlsym.h`, `random_names.h`, `obfuscate*.h`

* `mod/kill.h` (invoked in `JNI_OnLoad` through `CALL(0)` → hashed lookup of export `N_0`→`__KILL__`):
  * resolves `libdl.so`→`dlsym` and **inline-hooks the game's own `dlsym`** (`_dlsym`, exported under scrambled name). When it sees the game resolve `"ashmem_create_region"` it learns the loader's base (`libloader`) and rewrites pointer at `libloader+0x5c43e0` (a malloc cell) to point at its own `_malloc`. The replacement `_malloc`, when called with size `0x1598`, restores the original pointer and then **parks the calling thread in a `futex(FUTEX_WAIT)` forever** — i.e., it selectively hangs a specific allocation site inside the game (almost certainly an integrity/anti-cheat initialization, consistent with the `libwolf.so` checks in the SEGV handler). All syscalls are inline `svc #0` (no libc symbols), strings and numbers are compile-time obfuscated, and the whole file is wrapped in `OBF_BEGIN/END` bogus-flow macros plus 40 decoy exported functions (`DEFINE_DECOYS_*`).
* `include/manual_dlsym.h` — own ELF parser walking `dl_iterate_phdr`→`PT_DYNAMIC`→`DT_SYMTAB/STRTAB/GNU_HASH`, implementing GNU-hash bucket/chain lookup by hand (bypasses `dlsym` hooks), plus a **compile-time FNV-1a** (`consteval`) so export names can be hashed at build time and never stored.
* `include/random_names.h` (1,516 lines) — generated map `N_0→eUDXnpGl …` renaming every sensitive export (e.g. `A64HookFunction`→`N_6`, `__KILL__`→`N_0`).
* String obfuscation layers: `oxorany` (`O(...)` numbers/strings), custom `_o53` XOR-string class (`obfuscate.h`, `__TIME__`-seeded splitmix key, zero-on-destruct), `obfy` bogus-control-flow (`OBF_BEGIN`, fake `IF/WHILE/FOR` variants, `MAX_BOGUS_IMPLEMENTATIONS=3`), and `obfusheader.h` (present, config-only, not actually included anywhere).
* `pointers.h` SEGV handler: on crash logs backtrace, and if the faulting stack contains **`libwolf.so`** (the game's anti-cheat/integrity lib) it `sleep(10000)`s instead of dying — deliberately freezing to avoid crash telemetry; `TRIGGER_SAFEGUARD` exits the process immediately on demand.
* `proguard-rules.pro` — 5 optimization passes, aggressive renaming, strips line numbers/source files, and **`assumenosideeffects` deletes all `android.util.Log` calls** from release dex.

### 6.9 Support infrastructure — `include/`

| File | Contents (verified) |
|---|---|
| `includes.h` | type aliases (`ptr`), color macros, `pthread()` fire-and-forget helper, `EditMemory/EditPerm` (mprotect RWX patching), `DEFINED` stub-return generators (`STRUE/SFALSE/VOID/I0…` used for disabled hooks like `isVIPFeatureActive`), `absoluteAddress()` and `get8BPbase()`/`getWolfBase()` (`/proc/self/maps` scanners; the 8BP one groups mappings of the *largest* file under the game's package dir and checks `\x7fELF`), `create_directory_recursive`, `whois()` (dladdr based), string helpers, `CONC/SCONC` concat lambdas, `X/XN` xhook macro aliases, `TRIGGER_SAFEGUARD` |
| `hook.h` | `HOOK/HOOKI` choosing **And64InlineHook `A64HookFunction`** on aarch64 (Substrate `MSHookFunction` fallback on 32-bit), `DEFINES/DEFINE` original-function trampoline pattern |
| `logger.h` | log tag **`angousana`** (deliberately random-looking), LOGI/D/W/E macros (debug-branch disabled with `#ifdef NDEBUG` commented out — logging policy is compile-flag dependent) |
| `java.h` | `GetClassName/GetMethodName`, `FindClass` via app classloader, `__ORIENTATION__` monitor thread, `getClipboard()`, `getAndroidID()` (Settings.Secure), `GetJNIEnv()` — both getters **log their returned values** (privacy-relevant, see §18) |
| `input.h` | hooks `Java_com_miniclip_input_MCInput_nativeTouches{Begin,Move,End}`: feeds ImGui's mouse from pointer 0, tracks "touch started outside UI" to decide pass-through; `NativeTouchesBegin/Move/End` inject synthetic events by *calling the original (hooked) JNI implementations* with fabricated indices (10/11) |
| `stacktrace.h` | `_Unwind_Backtrace` walker, `get_lib_offset` (dladdr), `IsInStack(libname)` |
| `syscall.h` | raw AArch64 `svc #0` wrapper (6 args) |
| `generator.h` | C++20 coroutine `Generator<T>` — unused |
| `obfuscate.h`/`obfuscation.h` | XOR-string class + inline string fns + obfy glue (`INLINE/NOINLINE/EXPORT`, `inline_strcmp` under OBF) |
| `persistence.h` (imgui/inc) | JSON-backed settings store; **contains large unused FPS-template key sets** (`cESP_Box`, `fBulletTrack_Probability`…) proving template reuse |
| `dynamic.h` | `DynamicBool/DynamicString` defaulting maps (`dynamic_bool["DebugTouch"]` used by ButtonClicker) |
| `custom_theme.cpp/h` + `8bp.h` + `helpers.h` | gold/cream ImGui theme (§14), tiny draw helpers, placeholder `DrawEightBallLoading` (animation removed) |

---

## 7. Function Inventory

Measured on the 55 first-party source files (project code excluding `include/external/*` and `icons/*`): **≈204 function/method definitions** were counted by pattern analysis; the tables below enumerate the important ones, grouped by subsystem. (Third-party libraries add thousands more and are documented in §15, not per-function.)

### 7.1 Entry / lifecycle

| Function | File | Lines (approx.) | Purpose | Callers |
|---|---|---|---|---|
| `JNI_OnLoad(JavaVM*,void*)` | main.cpp | ~70–80 | store VM, run hidden `__KILL__` via `CALL(0)`, spawn `__1__` | ART loader |
| `__1__()` | main.cpp | 50–69 | init sequence (paths, base, hooks, signals) | `JNI_OnLoad` (thread) |
| `__HOOKS__()` | main.cpp | 30–47 | install inline hooks + xhook eglSwapBuffers | `__1__` |
| `__IMGUI__()` | menu.h | end | create no_backup dir | `__1__` |
| `__INPUT__()` | include/input.h | end | xhook game's 3 touch JNI functions | `__1__` |
| `setActiveVisualCue(ptr)` | main.cpp | 24–28 | hook: capture `sharedGameManager` | game (hooked) |
| `Draw(EGLDisplay,EGLSurface)` | menu.h | near end | hook of `eglSwapBuffers`; full ImGui render | EGL (hooked) |
| `SetupImgui()` | menu.h | ~1175 | ImGui context/theme/persistence init | `Draw` (once) |
| `onSendConfig/onCanvasDraw/getExpTime/MenuColor` | main.cpp | 84–120 | JNI config/telemetry bridge | external UI (optional) |
| `setup_global_segv_handler/is_segv_handler_active` | include/pointers.h | mid | install/verify SIGSEGV handler | `__1__`, `Draw`, `DrawMenu` |
| `SetupSignalTraceHandler/SignalTraceHandler` | include/pointers.h | early | abort/ill/fpe/bus/sys/trap logging | library + event loop |

### 7.2 Authentication / license

| Function | File | Purpose | Key calls/effects |
|---|---|---|---|
| `Login(std::string androidID, std::string key)` | mod/keylogin.h | full remote license check; sets `logged_in`, `g_Token`, `g_Auth`, `g_ServerExpiry`, `g_ExpTime`, `ERROR_MESSAGE` | VPN guard → UUID serial → curl POST → JSON parse |
| `IsVpnOrTunnelActive(JNIEnv*)` | mod/keylogin.h | fail-closed VPN/tunnel detection (3 API routes + `/proc/net/dev`) | called first by `Login` |
| `GetDarkDeviceUUID(env,seed)` | mod/keylogin.h | deterministic device serial `UUID.nameUUIDFromBytes` | `Login` |
| `GetDarkBuildString(env,field)` | mod/keylogin.h | `Build.MODEL/BRAND` readers | `Login` |
| `WriteMemoryCallback` | mod/keylogin.h | curl body sink (realloc-free mgmt) | curl |
| `ParseExpiryTime` / `GetKeyRemainingTime` / `getDt` | mod/keylogin.h | expiry parsing/format | `Login`, UserInfo tab |
| `xor_encrypt/xor_decrypt`, `base64_encode/decode`, `gToken`, `decryptData` | mod/keylogin.h | reversible string transforms (obfuscation of protocol values) | `Login` chain |
| `generateWebSocketKey/createWebSocketFrame/parseWebSocketFrame/connectWebSocket/sendWebSocketMessage` | mod/keylogin.h | WebSocket client helpers — **unused** | — |
| `DrawLogin(ImGuiIO&)` | menu.h | login window, spinner, clipboard + auto-login from saved key | `Draw` |
| `IsExpired()/DrawExpired()` | menu.h | hard build kill-switch UI | `Draw` |

### 7.3 Overlay UI

| Function | File | Purpose |
|---|---|---|
| `DrawMenu/DrawSidebar/DrawContentArea` | menu.h | main multi-tab window (Draw/AutoPlay/AutoQueue/UserInfo) |
| `DrawFloatingButton` | menu.h | logo button (drag vertically, tap opens menu) |
| `DrawToggleButton(cancelMode)` | menu.h | floating play/stop (AutoPlay) & autoqueue cancel button |
| `DrawAutoQueue` | menu.h | 1.2 s countdown window then `StartAutoQueue()` |
| `DrawCalculating` | menu.h | "CALCULATING…" overlay during slow scan |
| `DrawAutoPlayTarget/DrawGoldPanel/DrawGradientRect/SidebarButton/ToggleSwitch` | menu.h | custom widgets & theme painters |
| `EaseOutBack/EaseOutQuart` | menu.h | animation easing |
| `LoadTextureFromMemory` | (imgui layer) | converts embedded PNG arrays to GL textures |
| `switch_theme/StyleColorsCustom` | imgui theme files | gold/cream style application |
| `svConfig_Save/svConfig_Load` | menu.h | thickness/menu-size persistence |
| `ReadNSString` | menu.h | decodes in-memory ObjC NSString (`+0x10` len, `+0x14` UTF-16 buffer) — dead code path currently |

### 7.4 ESP / prediction

| Function | File | Purpose |
|---|---|---|
| `DrawESP(ImDrawList*)` | menu.h | per-frame overlay: singletons refresh, pockets, trajectories |
| `Prediction::determineShotResult(isAuto,angle,power,spin[,cand])` | Prediction(.fast).h | runs full simulation; caches prev angle/power/spin; returns redraw-needed |
| `Prediction::initBalls/initCueBall` | Prediction(.fast).h | snapshot 16 balls; launch cue ball |
| `Prediction::determineBallsPositions/handleCollision/handleBallBallCollision/determineShotState` | Prediction(.fast).h | event-loop simulation & outcome classification |
| `Ball::findNextCollision/calcVelocity/calcVelocityPostCollision/move/isMovingOrSpinning` | Prediction(.fast).h | per-ball kinematics |
| `Ball::isBallBallCollision/willCollideWithTable/determineBallTableCollision/isBallLineCollision/isBallPointCollision` | Prediction(.fast).h | collision-time solvers (mirrors of game subs) |
| `getPockets()` | Prediction.fast.h | 6 pocket world positions from `TableProperties` |
| `UpdateScreenTable()/WorldToScreen()` | ScreenTable.h | 1280×640 reference-frame → device mapping (height-scale + letterbox center) |
| `NumberUtils::normalizeDoublePrecision/calcAngle` | NumberUtils.h | game's own rounding quirks replicated (string-truncation) |
| `ShotPowerToPower` | NumberUtils.h | distance-power → UI power curve (1−(1−x)²) |
| `get8BPbase()/getWolfBase()/absoluteAddress()` | includes.h | `/proc/self/maps` base finders (8BP largest-file+ELF check) |

### 7.5 Bots

| Function | File | Purpose |
|---|---|---|
| `AutoPlay::Update/ScanFast/ScanSlow/Shoot/takeShot/ClearState/shouldAutoPlay/setAimAngle/isAnimationActive` | AutoPlay.h | complete aim-&-shoot bot (see §6.7) |
| `IsShotValid()` | AutoPlay.h | candidate legality re-check |
| `GetPocketScreenPos` | AutoPlay.h | pocket index → screen coords |
| `StartAutoQueue/StartMatch(hook)/PopMenuState/popMenuState` | AutoQueue.h | table auto-join via `StartMatch` call |
| `AutoAim::AIM/Draw/setAimAngle/shouldAutoAIM` | AutoAim.h | disabled aim helper |
| `ButtonClicker::Click/Update` | ButtonClicker.h | synthetic taps (nomination) |
| `PowerSlider::Start/End/Update` (partially seen) | PowerSlider.h | scripted power drag |
| `RangeSelector::{DrawSelector,PushState,Undo,Redo,…}` | RangeSelector.h | calibration overlay w/ undo-redo |
| `nativeTouchesBegin/Move/End` hook bodies + `NativeTouches*` injectors | input.h | touch capture/synthesis |

### 7.6 Anti-analysis / runtime plumbing

| Function | File | Purpose |
|---|---|---|
| `__KILL__` (export `N_0`) | mod/kill.h | installs loader-level `dlsym` hook inside game |
| `_dlsym` (export `N_9`) / `__get__dlsym`/`__get_dlsym` | mod/kill.h | hook body: detects loader, swaps malloc pointer `@libloader+0x5c43e0` |
| `_malloc` | mod/kill.h | size-0x1598 trap → restore pointer + `futex` hang |
| `ManualLookup::dlsym/_hashed/walk_exported_symbols/fnv1a` | manual_dlsym.h | own ELF GNU-hash resolver (no `dlsym`) |
| `__syscall_arm64` | syscall.h | raw syscall gate |
| `log_stacktrace/IsInStack/get_lib_offset` | stacktrace.h | crash forensics |
| `EditMemory/EditPerm` | includes.h | RWX patching helpers (unused in current flow) |
| ~40 `DEFINE_DECOYS_*` bodies | random_defs.h | junk exported functions |

Count note: the project also *declares but does not run* several legacy hooks visible as comments (`isVIPFeatureActive`, `isPayingUser`, `convertToGL/UI`, `convertTouchToNodeSpace`, `getTotalWinnings`) — preserved as development breadcrumbs.

---

## 8. Important Functions

Deep-dive cards for the eight most consequential routines.

### 8.1 `JNI_OnLoad` — `main.cpp` (~line 74)

* **Inputs:** `JavaVM* vm`. **Outputs:** `jint JNI_VERSION_1_6`.
* **Behavior:** `VM = vm`; `CALL(0)` → `ManualLookup::dlsym_call_hashed(FNV1a(N_0="__KILL__"))` executes the anti-analysis installer synchronously on load; then `pthread(__1__)` detaches the main logic thread.
* **Why important:** single chokepoint that starts every subsystem; demonstrates the mod never needs a Java `loadLibrary` call from *this* app — any loader can inject the `.so` and `JNI_OnLoad` self-starts.

### 8.2 `Draw` (hooked `eglSwapBuffers`) — `menu.h` (~line 1238)

* **Inputs:** `EGLDisplay dpy, EGLSurface surface` (game frame). **Output:** `EGLBoolean` (original call result).
* **Behavior:** measures surface → one-time `SetupImgui()` → per-frame state machine: `expired ? DrawExpired : (authed ? floating buttons + menu + watermark + ESP (+calc overlay) : DrawLogin)` → render → `_Draw(dpy,surface)`.
* **Notable:** the **entire product UI runs inside the game's GL thread**; a global segv guard (`jump_buffer_active/sigsetjmp`) wraps `DrawESP` so game-state races can't crash the game (crash → spoofed stability).

### 8.3 `Login` — `mod/keylogin.h` (~line 989)

* **Inputs:** `androidID`, license `key`. **Output:** `bool bValid`.
* **Data flow:** VPN check → serial = `UUID.nameUUIDFromBytes(key+androidID+MODEL+BRAND)` → POST to decoded panel URL → accept iff `status==true`, `data.rng+30>now`, `EXP` parses to a future `time_t` → unlock.
* **Constants:** URL XOR-byte blob (`kPanelUrl`, key `0x5A`), `Content-Type: application/x-www-form-urlencoded`, fields `game=PUBG&user_key=&serial=`, TLS verify disabled, timeouts 15/30 s.
* **Why important:** monetization + remote kill authority; weaknesses documented in §18.

### 8.4 `DrawESP` — `menu.h` (~line 331)

* **Inputs:** background `ImDrawList*`. **Output:** none (draws).
* **Behavior:** pointer-laundering from the game image (4 singleton addresses re-read every frame), forces `UserInfo+0x340=1`, AutoQueue branch when out of game, predictor invocation on state 4, pocket rings, two-pass trajectory polyline + dot rendering with the 16-color palette, all gated on login == true.

### 8.5 `Prediction::determineShotResult` — `game/inc/Prediction.fast.h` (~line 134)

* **Inputs:** `isAuto`, `shotAngle`, `shotPower`, `shotSpin`, optional `Candidate`.
* **Output:** `bool` (geometry changed / redraw).
* **Behavior:** early-exit when inputs unchanged → snapshot balls → set cue-ball velocity/spin (rounded trig, spin factor `power/3.800475`) → event-driven collision loop with `TIME_PER_TICK 0.005` (fast-mode allows candidate early-abort via `firstHitIsTarget`) → pocket bookkeeping → fills `guiData.positions`.

### 8.6 `AutoPlay::ScanFast/Shoot` — `game/inc/AutoPlay.h`

* Verified behavioral pair: geometric candidate generation + validation; `Shoot()` writes aim/power into game memory and **calls the game's shot routine (`libmain+0x2dc0c58`)**, with a nomination sub-state driving `ButtonClicker` taps on the required pocket when needed. This is a complete input-free bot: no screen gestures except the nomination tap.

### 8.7 `StartMatch` hook + `StartAutoQueue` — `game/inc/AutoQueue.h`, `main.cpp`

* `main.cpp` hooks `libmain+0x3068c94`; the hook body simply chains to `_StartMatch`, existing **solely to capture the trampoline** for `StartAutoQueue()` which then directly enters matches on the configured stake table (17 tiers), bypassing the game's menu flow (hence `isInQueue()` checks and `PopMenuState()` helper).

### 8.8 `__KILL__` / `_dlsym` / `_malloc` — `mod/kill.h`

* Loader-level anti-integrity: hooks game's `dlsym`, waits for `ashmem_create_region` resolution, locates loader base via `dladdr(return_address)`, rewrites `libloader+0x5c43e0` (validated by `0x464c457f` ELF magic + pointer equality) to redirect a malloc-cell, then when the game performs the specific `0x1598`-byte allocation, restores the pointer and parks the thread on `futex`. Purpose: selectively neuter an in-game (anti-cheat) initialization without crashing the game — consistent with the `libwolf.so` freeze-on-crash behavior in the SEGV handler.

---

## 9. Binary Analysis

No final APK or `libMarkXit.so` ships in the archive. Prebuilt binaries are **7 static libraries and 7 relocatable objects** (all AArch64, plus an unused 32-bit set) and **2 Gradle wrapper JARs**.

### 9.1 Prebuilt static libraries (`jni/include/external/`)

| File | Size | Format / Arch | Toolchain fingerprint | Identity markers |
|---|---:|---|---|---|
| `curl/curl-android-arm64-v8a/lib/libcurl.a` | 969,004 B | GNU ar, 124 members, ELF64 **AArch64** REL | `GCC: (GNU) 4.9.x 20150123 (prerelease)` | `libcurl/7.51.0`, `LIBCURL_TIMESTAMP "Wed Nov 2 06:54:46 UTC 2016"` |
| `curl/curl-android-armeabi-v7a/lib/libcurl.a` | 783,268 B | ELF32 **ARM EABI v5** REL (unused by build) | GCC 4.9.x | curl 7.51.0 |
| `curl/openssl-android-arm64-v8a/lib/libssl.a` | 675,792 B | 39 members, AArch64 | GCC 4.9.x, leenjewel toolchain (macOS path strings) | OpenSSL **1.1.0c** (2016-11-10) |
| `curl/openssl-android-arm64-v8a/lib/libcrypto.a` | 4,155,536 B | 602 members, AArch64 | build string: `/Users/leenjewel/workspaces/openssl_for_ios_and_android/… aarch64-linux-android-gcc -DZLIB -DDSO_DLFCN …` platform `android64-aarch64` | OpenSSL 1.1.0c |
| `curl/openssl-android-armeabi-v7a/lib/{libssl,libcrypto}.a` | 508,068 + 3,157,996 B | ARM EABI v5 (unused) | GCC 4.9.x, leenjewel | OpenSSL 1.1.0c |
| `xhook/src/arm64-v8a/lib/libxhook.a` | 187,018 B | 7 members, AArch64 | **Android clang 18.0.3 (r522817c, NDK r26+ toolchain string `12470979, +pgo, +bolt, +lto, +mlgo`)** | `libxhook 1.2.0 (aarch64)` |

`libcurl.pc` confirms the upstream build tree: `prefix=/Users/leenjewel/workspaces/openssl_for_ios_and_android/…/curl-android-arm64-v8a`, `supported_protocols=DICT FILE FTP FTPS GOPHER HTTP HTTPS IMAP …`, `features=SSL IPv6 UnixSockets libz AsynchDNS NTLM NTLM_WB TLS-SRP`.

### 9.2 Relocatable objects with leaked provenance (`xhook/src/arm64-v8a/lib/objs/xhook/`)

Seven `.o` files (+ their `.d` make-dep files) are the per-object build output of libxhook. **Every one embeds the author's build path** in debug info:

```
/home/cici/AppProjects/Bloodstrike-static-v2/jni/include/external/xhook
```

i.e., compiled on a Linux host, user `cici`, inside a project named **Bloodstrike-static-v2** — direct evidence this mod was re-skinned from a *Blood Strike* cheat tree. The `.d` files reference `./obj/local/arm64-v8a/objs/xhook/...` (ndk-build intermediates).

### 9.3 Instruction-level verification (Capstone AArch64)

Because the host `objdump` lacked AArch64 support, disassembly was performed with Capstone 5.0.9 over section bytes carved at their **measured file offsets**. Two verified examples:

**`xhook.o` (from `libxhook.a`; identical to `objs/xhook/xhook.o`)** — ELF64/AArch64/REL, 28 sections, section header table at `file+0x13e0`, `.strtab` at `file+0x11e0`. Public API = 6 one-instruction trampolines (each 4 bytes: `b #0` + `R_AARCH64_JUMP26` relocation):

| Symbol | Section | File offset | Instruction | Relocation target |
|---|---|---:|---|---|
| `xhook_register` | `.text.xhook_register` | `0x40` | `b #0` | `xh_core_register + 0` |
| `xhook_ignore` | `.text.xhook_ignore` | `0x44` | `b #0` | `xh_core_ignore + 0` |
| `xhook_refresh` | `.text.xhook_refresh` | `0x48` | `b #0` | `xh_core_refresh + 0` |
| `xhook_clear` | `.text.xhook_clear` | `0x4c` | `b #0` | `xh_core_clear + 0` |
| `xhook_enable_debug` | `.text.xhook_enable_debug` | `0x50` | `b #0` | `xh_core_enable_debug + 0` |
| `xhook_enable_sigsegv_protection` | `.text.xhook_enable_sigsegv_protection` | `0x54` | `b #0` | `xh_core_enable_sigsegv_protection + 0` |

(Relocation records: `.rela.text.*` at file offsets `0xb68…0xbff`, `R_AARCH64_JUMP26`, addend 0.)

**`xh_core.o`** — ELF64/AArch64/REL, 68 sections, `.strtab` `file+0xff28`, section headers `file+0x10610`. Symbols were cross-verified three ways (`nm -S` sizes, `readelf -sW` section indices, `readelf -SW` section file offsets): `xh_core_register` (0x154) @ `file+0x40`, `xh_core_ignore` (0x138) @ `file+0x194`, `xh_core_refresh` (0x1f0) @ `file+0x2cc`, **`xh_core_clear` (0x200)** @ **`file+0x4bc…0x6bb`**, `xh_core_enable_debug` (0x20) @ `file+0xac8`, `xh_core_enable_sigsegv_protection` (0x14) @ `file+0xae8`, `xh_core_check_elf_header` (0xa0) @ `file+0x10ac`, `xh_core_hook` (0x94) @ `file+0x1420`, `xh_core_hook_impl` (0xc0) @ `file+0x14b4`. First instructions of `xh_core_clear` (file offsets shown):

```
file+0x4bc  stp  x29, x30, [sp, #-0x30]!
file+0x4c0  str  x21, [sp, #0x10]
file+0x4c4  stp  x20, x19, [sp, #0x20]
file+0x4c8  mov  x29, sp
file+0x4cc  adrp x20, #0          ; page of "request_pending" flag
file+0x4d0  ldr  w8, [x20]
file+0x4d4  cbz  w8, #0x5c        ; skip if nothing pending
...
file+0x518  adrp x8, #0           ; second flag group (ignored-list)
...
file+0x6b8  b    #0x1b4           ; loop back → cleanup
```

(`nm` size=0x200 matches Capstone-decodable body; `adrp` addends are 0 pending relocation — expected in REL objects. Sanity check: the frame setup/teardown and flag-store pattern matches upstream xhook 1.2.0's `xh_core_clear()` source at `xhook/src/xh_core.c`, which is also present in the tree.)

### 9.4 Java archives

`gradle/wrapper/gradle-wrapper.jar` and `app/gradle/wrapper/gradle-wrapper.jar` (both 51,xxx B-class ZIPs) contain only stock `org/gradle/wrapper/*` classes (`GradleWrapperMain`, `Download`, `Install`, `ExclusiveFileAccessManager`…) with a bare `META-INF/MANIFEST.MF` (`Implementation-Title: Gradle Wrapper`); actual Gradle version comes from `distributionUrl` = **gradle-8.5-bin.zip**.

### 9.5 Binaries intentionally not present

* No `.dex`, no APK, no compiled `libMarkXit.so` (fresh source drop).
* No keystore/signing material.

---

## 10. Important Offsets & Locations (Addresses)

Two different address families must not be confused:

* **(A) File offsets** — measured inside files in this archive (verified by tooling; 0-based, hex).
* **(B) libmain-relative offsets (RVA)** — constants in the source that index into the **game's** main library at runtime (`libmain = get8BPbase()`). The game binary is *not* in the archive, so these are **source-verified, not binary-verified**. Their accuracy depends on the exact 8 Ball Pool version the author targeted (the tree self-documents "v56.13.0" in `NumberUtils.h` for some constants and "5.8.0" for prediction subs — treat as author's version map).

### (A) Verified file offsets inside archive binaries

| File | Symbol/Section | File offset | VA (in REL obj) | Size | Description / how identified |
|---|---|---:|---:|---:|---|
| `libxhook.a → xhook.o` | `.text.xhook_register` | `0x40` | 0 | 4 | `b` trampoline; readelf -SW + Capstone; reloc→`xh_core_register` |
|  | `.text.xhook_ignore` | `0x44` | 0 | 4 | same method |
|  | `.text.xhook_refresh` | `0x48` | 0 | 4 | same method |
|  | `.text.xhook_clear` | `0x4c` | 0 | 4 | same method |
|  | `.text.xhook_enable_debug` | `0x50` | 0 | 4 | same method |
|  | `.text.xhook_enable_sigsegv_protection` | `0x54` | 0 | 4 | same method |
|  | `.rela.text.xhook_register` | `0xb68` | — | 0x18 | readelf -rW: `R_AARCH64_JUMP26 xh_core_register+0` |
|  | `.strtab` | `0x11e0` | — | 0x1f9 | string table |
|  | section header table | `0x13e0` | — | 28×64 | readelf -S header |
| `libxhook.a → xh_core.o` | `.text.xh_core_clear` | `0x4bc` | 0 | 0x200 | nm size + Capstone full-body decode (entry `stp x29,x30,[sp,#-0x30]!` @0x4bc) |
|  | `.text.xh_core_register` | `0x40` | 0 | 0x154 | readelf -sW shndx 3 ↔ readelf -SW |
|  | `.text.xh_core_ignore` | `0x194` | 0 | 0x138 | readelf -sW shndx 5 ↔ readelf -SW |
|  | `.text.xh_core_refresh` | `0x2cc` | 0 | 0x1f0 | readelf -sW shndx 7 ↔ readelf -SW |
|  | `.text.xh_core_enable_debug` | `0xac8` | 0 | 0x20 | readelf -sW shndx 13 ↔ readelf -SW |
|  | `.text.xh_core_enable_sigsegv_protection` | `0xae8` | 0 | 0x14 | readelf -sW shndx 15 ↔ readelf -SW |
|  | `.text.xh_core_check_elf_header` | `0x10ac` | 0 | 0xa0 | readelf -sW shndx 23 ↔ readelf -SW |
|  | `.text.xh_core_hook` | `0x1420` | 0 | 0x94 | readelf -sW shndx 29 ↔ readelf -SW |
|  | `.strtab` | `0xff28` | — | 0x6e3 | string table |
|  | section header table | `0x10610` | — | 68×64 | readelf -S header |

### (B) libmain-relative runtime offsets used by the mod (source constants)

| RVA (hex) | Kind | Meaning | Evidence location |
|---|---|---|---|
| `0x2d911e0` | inline hook target | game fn capturing `GameManager*` (labelled `setActiveVisualCue`) | main.cpp `__HOOKS__` |
| `0x3068c94` | inline hook target | `MenuManager::StartMatch` (trampoline captured for AutoQueue) | main.cpp `__HOOKS__`, AutoQueue.h |
| `0x4f06288` | global pointer | CCDirector singleton | menu.h `DrawESP` |
| `0x4e9feb8` | global pointer | UserInfo singleton | menu.h `DrawESP` |
| `0x4dde3e0` | global pointer | MainManager singleton | menu.h `DrawESP` |
| `0x4dfe838` | global pointer | MenuManager singleton | menu.h `DrawESP` |
| `0x4e49418` | global double | `CUE_PROPERTIES_SPIN` | Types.h |
| `0x4e49410` | global double | `CUE_PROPERTIES_MAX_POWER` | Types.h |
| `0x2dc0c58` | call target | shot-execution function (arg = object at `GameManager+0x3b0`) | AutoPlay.h `takeShot` |
| `0x2de6f30` | call target | power-bar active-action getter | AutoPlay.h `isAnimationActive` |
| `0x3051f00` | call target | `MenuManager::popMenuState:withScene:` | AutoQueue.h |
| `0x305114c` | call target | `MenuManager::getMenuStateId` | MenuManager.h |
| `0x5c43e0` | data pointer cell | inside **libloader** (game's loader lib); malloc-pointer swap site | mod/kill.h |
| `0x340` | struct offset | UserInfo boolean forced `true` per frame | menu.h `DrawESP` |

Disabled/legacy offsets (comments only, **not active**): `isVIPFeatureActive 0x368b390`, `isPayingUser 0x358fdc4`, `convertToGL 0x390cd80`, `convertToUI 0x390cfd4`, `convertTouchToNodeSpace 0x392376c`, `FUN_02b1bb3c`, `FUN_02b1bfc0 0x2b1bfc0`, `getTotalWinnings 0x3406610`, prediction reference subs `0x2b1b158 / 0x3606c80 / 0x3606d2c / 0x1bfb66c` (stale `Prediction.update.offsets.h`), physics constant snapshots `0x4c8b9f8/0x4c8ba00/0x4c8b998/0x4c8bc98` ("v56.13.0" annotations in `NumberUtils.h`).

### (C) In-object struct offsets (class model)

`instance+0x10` class-name pointer · GameManager `0x3e0/0x3e8/0x4b8/0x4c8/0x508/0x5c0` · Ball `0x20/0x30/0x40/0x48/0x60/0x68/0xa0/0xa4` · Table `0x3b0/0x3c0/0x450/0x588` · TableProperties pockets `0x68` · VisualCue `0x3a8/0x3b0`, power-bar `+0x510` · VisualGuide `0x28/0xa0` · VisualEnglishControl `0x3b0` · MainManager `0x3b0` · UserInfo `coins 0x208, cash 0x210, DisplayName 0xc0, loginCountryCode 0xd0` · Ruleset `nominationMode 0x68, nominatedPocket 0x118, classifications 0x98/0xB0/0xC8` · State `mStateId 0x18`, StateManager stack `0x8` · ObjC NSString `len 0x10, chars 0x14`.

---

## 11. Strings & Constants

Extracted in full to **`8bp_analysis/INTERESTING_STRINGS.txt`** (with per-file locations). Highest-value items summary:

| Category | String / constant | Location |
|---|---|---|
| Endpoint | `https://axlmods.myvippanel.shop/useradmin/connect` (XOR-0x5A blob `kPanelUrl[49]`) | mod/keylogin.h |
| Deep links | `https://t.me/DARK_OWNER_VIP` | MainActivity.java:116, menu.h:873, notices |
| POST template | `game=PUBG&user_key=%s&serial=%s` | keylogin.h |
| Kill-switch | `EXPIRY_TS = 1788805800` → 2026-09-07 18:30 UTC | menu.h |
| UI titles | "ADMIN SERVER LOGIN", "8 BALL POOL", "MOD EXPIRED", "DRAW SETTINGS", "AUTO PLAY", "AUTO QUEUE", "USER INFO", "CALCULATING..." | menu.h |
| Auth errors | "Server Off", "Key expired", "Invalid or expired server response", "Could not get Android ID", "Key Is Empty or Failed to get Key" | keylogin.h |
| Log tag | `angousana` | include/logger.h |
| Tunnel interface names | `tun0-5, wg0-2, ppp0-1, ipsec0-1` | keylogin.h |
| Config keys | `ESP::LINES / ESP::POCKETS / ESP::STATES / AUTO::PLAY / AUTO::QUEUE` | main.cpp |
| Persistence | `bESP_DrawPredictionLine, bESP_DrawPocketsShotState, bAutoPlay, bAutoQueue, iLineThickness, iMenuSizeOffset, iAutoQueue_FixTable, key` | persistence.h, menu.h |
| Paths | `/data/user_de/0/<pkg>/no_backup/{.ini,persistence.json,style.json}`, `/data/user/0/<pkg>/files/svConfig.txt`, `/proc/self/maps`, `/proc/net/dev` | menu.h, keylogin.h, includes.h |
| xhook regex | `.*/com.miniclip.eightballpool/.*` + `eglSwapBuffers` | main.cpp; `Java_com_miniclip_input_MCInput_nativeTouches*` | input.h |
| Build-path leaks | `/home/cici/AppProjects/Bloodstrike-static-v2/...` (all 7 xhook .o), `/Users/leenjewel/workspaces/openssl_for_ios_and_android/...` (openssl .comment) | binaries |
| Version strings | `libxhook 1.2.0 (aarch64)`, `ImGui 1.85 WIP`, `nlohmann/json 3.11.2`, `libzip 1.0.1`, `7.51.0`, `OpenSSL 1.1.0c` | sources/binaries |
| Branding | "DARK OWNER ADMIN SERVER", "PROUDLY MADE FOR INDIA", "DARK OWNER EDITION" | throughout |
| Multilingual comments | Romanian (MainActivity/menu), Hindi (keylogin/notice), Bengali (makefiles/helpers), Arabic (AutoPlay) | sources |

No hard-coded credentials, API keys, private keys, or signing material were found (§18).

---

## 12. Features

### 12.1 Confirmed features (direct code evidence)

| # | Feature | Description | Evidence (files/functions) | Confidence |
|---|---|---|---|---|
| F1 | **Aim-trajectory ESP ("Draw Lines")** | full physics prediction of all 16 balls drawn as colored polylines with start/end markers over the live table | menu.h `DrawESP`, Prediction.fast.h, ScreenTable.h, `colors[16]`; toggle `bESP_DrawPredictionLine`, `iLineThickness` | Confirmed |
| F2 | **Pocket-state indicators ("Draw Pockets")** | green ring on pockets the current aim would pot into | menu.h, `Prediction::pocketStatus[6]`, toggle `bESP_DrawPocketsShotState` | Confirmed |
| F3 | **AutoPlay bot** | geometric + brute-force shot search; writes aim angle/power; executes the shot via game function; handles 8-ball/9-ball/pocket-call rules; floating play/pause button; "CALCULATING…" state | AutoPlay.h, PowerSlider/ButtonClicker, menu.h toggle button | Confirmed |
| F4 | **AutoQueue bot** | auto-joins one of 17 stake tiers by calling the game's `StartMatch` with `M1..M17`; 1.2 s countdown UI; floating cancel button | AutoQueue.h, menu.h `DrawAutoQueue` | Confirmed |
| F5 | **License-key login (remote auth)** | clipboard→key HTTPS license check w/ device-bound serial; auto-login on saved key; spinner + error surfaces; unlock gates **all** cheat UI | keylogin.h, menu.h `DrawLogin` | Confirmed |
| F6 | **Hard build expiry** | whole mod shows "MOD EXPIRED" after `EXPIRY_TS` | menu.h | Confirmed |
| F7 | **VPN fail-closed check** | refuses login while any VPN/tunnel is up; shows "Server Off" | keylogin.h `IsVpnOrTunnelActive` | Confirmed |
| F8 | **In-game overlay menu** | 4 tabs (Draw settings incl. line thickness 1–10 + menu size −10..+10, AutoPlay, AutoQueue w/ 17 stake buttons, User Info card w/ license status + expiry + Telegram JOIN), draggable floating launcher, animated open | menu.h | Confirmed |
| F9 | **Persistent settings** | JSON in `no_backup` dir + svConfig.txt | persistence.h, menu.h | Confirmed |
| F10 | **Touch capture & synthetic input** | steals/passes real touches to ImGui; injects taps/drags (indices 10/11) | input.h, ButtonClicker.h, PowerSlider.h | Confirmed |
| F11 | **Anti-tamper/anti-debug** | loader `dlsym`/`malloc` interposition + futex hang; crash freeze on `libwolf.so`; obfuscated exports; decoys; raw syscalls; string/number obfuscation; ProGuard log strip | kill.h, manual_dlsym.h, random_names/defs, obfus*, pointers.h, proguard | Confirmed |
| F12 | **Telegram funnel** | multiple one-tap opens to `t.me/DARK_OWNER_VIP`; offline-state heart branding | MainActivity, menu.h, expired screen | Confirmed |
| F13 | **External config bridge** | `Java_android_service_SurfaceView_onSendConfig` accepts `ESP::*`/`AUTO::*` toggles from an external app over JNI | main.cpp | Confirmed |
| F14 | **Server expiry display** | User-tab "EXPIRES • dd MMM yyyy" from `g_ServerExpiry` (or fallback build expiry); `getExpTime` JNI export | menu.h, keylogin.h | Confirmed |

### 12.2 Probable / inferred features (not directly wired in visible code)

| # | Feature | Basis | Confidence |
|---|---|---|---|
| P1 | Root-assisted loader (Zygisk module or external injector) | zygisk.hpp header, ACCESS_SUPERUSER, libsu deps, kill.h design — but **no loader code present** | Medium |
| P2 | WebSocket telemetry/control channel | complete unused WS client in keylogin.h | Low (dormant code) |
| P3 | VIP/premium bypass of game purchases | commented hooks `isVIPFeatureActive/isPayingUser` | Low (disabled) |
| P4 | Range-selector-based touch calibration per device | RangeSelector.h present, integrates via `selector.IsActive()` comments | Medium |
| P5 | Pocket call-pocket automation timing tuning | nomination frame counters (10/20 frames) hard-coded | Confirmed-in-code but empirically tuned (High) |

---

## 13. UI/UX Analysis

The UI exists on two planes:

### 13.1 Java carrier (rarely seen)

* **Single element:** floating pill (rounded rect, radius 60 px) at (50,200), elevation 12, containing `offline_heart` icon (72×72) + bold 14 sp white text **"DARK OWNER ADMIN SERVER"**; background `#B00000` ("login red" per code comment).
* Drag with 10 px dead-zone, <200 ms treated as click → opens Telegram. No launcher activity layout (`activity_main.xml` empty `LinearLayout`; `setContentView` commented out). Theme `Theme.Material.Light.NoActionBar`.

### 13.2 Native ImGui overlay (injected into the game)

**Login gate (pre-auth):** full-screen dim (`rgba(10,10,15,0.96)`) + centered 600×450 card with gold gradient header (dark→gold), typography-scaled title "ADMIN SERVER LOGIN" + subtitle "8 BALL POOL", gold double borders, red-brown input panel labeled "LICENSE KEY", full-width gold "ENTER KEY" button (65 px), footnote "Secure login • Server verified license key", amber error text area, 12-dot animated spinner + "Authenticating..." during requests. Auto-login triggers when a saved key exists.

**Main menu (post-auth):** center-screen, ~650×(520/540/620) px scaled by `iMenuSizeOffset`, 16–20 px corner radii, dark-gold frame `#4A2B0A` with double gold keylines, alpha fade-in (`EaseOutBack/Quart` helpers). Top strip = 4 image-tab sidebar buttons (Draw/Play/Queue/User) + close "X". Content panels are cream cards with dark-brown text in gold frames. Tab bodies:

* **DRAW SETTINGS:** two custom animated toggle switches ("Draw Lines", "Draw Pockets"), "Line Thickness" slider 1–10, "Fix Menu Size" slider −10..+10 (0="Normal"), "Save Config" button (55 px).
* **AUTO PLAY:** single toggle + cream info card (112 px) with crosshair/target glyph drawn via draw-list primitives, 3-line explainer text.
* **AUTO QUEUE:** toggle + three coin cards ("100", "100M", "200M" images) + 4-column grid of **17 stake buttons** (100 → 200M), selected state in bright gold.
* **USER INFO:** fixed card (300 px) listing icon rows: India flag "PROUDLY MADE FOR INDIA", lightning "DARK OWNER EDITION", Telegram pill with horizontally scrollable `@DARK_OWNER_VIP`, lock "LICENSE STATUS • ACTIVE" (green), calendar "EXPIRES • dd MMM yyyy" (red), and full-width gold "JOIN NOW" button.

**Ambient widgets:** draggable logo button (right edge, 58 px radius), floating play/stop button (left edge, 96 px) that doubles as queue-cancel in cancel mode, red-bordered "CALCULATING…" pill during slow scans, 1.2 s large red countdown digits before auto-queuing, bottom watermark "DARK OWNER ADMIN SERVER" (gold, always on), and a hard "MOD EXPIRED" gate screen post-expiry.

**Typography/spacing:** ImGui default font at 40 px (touch-scaled), generous 8–12 px item spacing, `TouchExtraPadding 10`, all custom widgets re-implemented on `ImDrawList` (rounded rects, multi-color rects, image + text rows). Visual hierarchy: dark gold frame → cream content cards → brown serif-ish text with gold controls; consistent radius language (9–24 px) across widgets.

**States:** hidden (button only), open (animated), authenticating (spinner), calculating (overlay), countdown (queue), expired (gate), error inline (login), disabled (everything pre-login). Accessibility/localization: English-only UI strings; no dynamic color/blind modes.

---

## 14. Colors & Visual Resources

All values extracted **directly from source** (no guesses). ImGui `ImVec4` floats converted to 8-bit hex where the source uses floats.

### 14.1 Native overlay palette (menu.h constants)

| Token | RGBA / HEX | Usage |
|---|---|---|
| `UI_GOLD` | (173,112,28,255) `#AD701C` | selected sidebar button, primary accent, join/enter buttons (`0.678,0.439,0.110`) |
| `UI_GOLD_DARK` | (116,70,14,255) `#74460E` | gradient start, unselected AQ buttons, login field frame |
| `UI_GOLD_LIGHT` | (232,194,112,255) `#E8C270` | borders/highlights, login border, "LICENSE KEY" label |
| `UI_CREAM` | (247,232,198,255) `#F7E8C6` | info/user cards |
| `UI_PANEL` | (236,214,169,255) `#ECD6A9` | content panels |
| `UI_PANEL_DARK` | (92,57,20,255) `#5C3914` | license input well |
| `UI_TEXT` | (55,34,15,255) `#37220F` | primary text |
| `UI_MUTED` | (112,83,45,255) `#70532D` | secondary text (defined) |
| Sidebar bg | (105,66,18,245) `#694212` | tab strip |
| Menu frame | (74,43,10,245) `#4A2B0A` | outer window |
| Toggle ON | (0.82,0.55,0.10) ≈ `#D18C1A` | switch track enabled |
| Toggle OFF | (0.39,0.27,0.11) ≈ `#63451C` | switch track disabled |
| Toggle knob | (255,250,232) `#FFFAE8` (+shadow (70,40,10,55)) | switch knob |
| Slider grab | (0.83,0.55,0.10) ≈ `#D48C1A` | thickness/size sliders |
| Frame bg (inputs) | (0.55,0.38,0.16) ≈ `#8C6129` | slider troughs |
| Buttons hover | (0.910,0.761,0.439) ≈ `#E8C270` / (0.82,0.62,0.28) `#D19E47` | hover states |
| Buttons active | (0.455,0.275,0.055) ≈ `#74460E` | pressed |
| Login overlay | (0.04,0.04,0.06,0.96) ≈ `#0A0A0F` | dim backdrop |
| Login card | (0.055,0.055,0.07,0.98) ≈ `#0E0E12` | card bg |
| Error text | (0.95,0.72,0.32) ≈ `#F2B852` | login errors |
| Expired bg | (21,21,21) `#151515`; text red (1.0,0.1,0.1) | kill screen |
| Calculating border | (220,30,30) `#DC1E1E`; bg `#151515` | scan overlay |
| License ACTIVE | (34,151,55) `#229737` | user tab |
| Expiry value | (196,45,38) `#C42D26` | user tab |
| Watermark | (173,112,28) `#AD701C` | bottom credit |

### 14.2 Java layer

* Floating pill background `#B00000` (ARGB `0xFFB00000`), text `#FFFFFF`, fallback icon bg `#FFFFFFFF`.

### 14.3 ImGui "Custom" theme (`custom_theme.cpp`)

Text `#38210D`; TextDisabled `#7A592E`; WindowBg `#E0C794` (0.88,0.78,0.58); ChildBg `#EBD6AB`; PopupBg `#DBBA7A`; Border `#8C5910`@75%; TitleBg `#78470D`/`#8C571A`; ScrollbarGrab `#BF852E`; CheckMark `#FFF5D1`; Button `#91611F` (hov `#B8802E`, act `#73450D`); Tab family `#85520D`→`#B3 7306` — same warm gold system as §14.1.

### 14.4 Ball trajectory palette (`game/inc/game.h`)

16 entries indexed by ball: white `#FFFFFF`, yellow `#FFFF00`, blue `#0000FF`, red `#FF0000`, purple `#800080` (0.50196…), orange `#FFA500` (1,0.647,0), green `#008000`, maroon `#800000`, black `#000000`, then repeat (stripes). Pocket-state ring: `GREEN (0,255,0)` radius 30 px, thickness 5 px.

---

## 15. Dependencies

### 15.1 Native third-party (bundled under `jni/include/external/`)

| Library | Version (verified) | How used | Compiled? |
|---|---|---|---|
| **Dear ImGui** | 1.85 WIP (`IMGUI_VERSION_NUM 18416`) | entire overlay UI (`imgui.cpp` amalgam incl. Android + GLES3 backends) | Yes (amalgam TU) |
| **And64InlineHook** | n/a (source bundled; API `A64HookFunction(V)`, `A64RestoreHook(s)`) | inline hooks on game functions | Yes |
| **Cydia Substrate (port)** | n/a | `MSHookFunction` (32-bit fallback path only) | Yes |
| **xhook** | 1.2.0 (string + `xhook.h`) | PLT/GOT hooks: `eglSwapBuffers`, MCInput JNI fns | Yes (`xhook.c`; prebuilt .a/.o also shipped unused) |
| **dlfcn shim (`fdl*`)** | n/a | safe dlopen/dlsym wrapper used by `HOOKS` macro | Yes |
| **libcurl** | 7.51.0 (2016) | license HTTPS POST | Prebuilt static (arm64; armv7 unused) |
| **OpenSSL** | 1.1.0c (2016-11-10) | TLS backend for curl | Prebuilt static (×2 ABIs) |
| **nlohmann/json** | 3.11.2 | auth JSON + config persistence | Header-only |
| **Vector math headers** | n/a | `Point2D/Vec2d/Vec3d/Vec4d` physics | Header-only |
| **oxorany / obfy / obfusheader** | n/a | compile-time obfuscation (`O()`, `OBF_*`) | Header-only (obfusheader unused) |
| **libzip** | 1.0.1 (zipconf.h) | **unused** — not in Android.mk | No (dead sources) |
| **zlib** | system `-lz` | curl dependency flag | Link flag only |

### 15.2 Java/Android dependencies (`app/build.gradle`)

`androidx.multidex 2.0.1` (multiDexEnabled), **libsu `core` + `service` 5.0.5** (`com.github.topjohnwu.libsu` — root shell/IPC, unused by visible code), `androidx.core-ktx 1.3.2`, `appcompat 1.2.0`, `material 1.3.0`, `constraintlayout 2.0.4`, **Glide 4.12.0** (image loading, unused by visible code). Repos: google, jcenter (dead repo), jitpack.

### 15.3 Version catalog (`gradle/libs.versions.toml`) — declared but mostly unused

AGP 8.2.1; appcompat 1.7.0; libsu catalog 6.0.0; **Firebase BOM 33.8.0 + firebase-auth 23.1.0 + firebase-database 21.0.0 + firebase-analytics**; **jbcrypt 0.4**; OkHttp 4.9.3 + logging-interceptor; Retrofit 2.9.0 + converter-gson; material 1.12.0; constraintlayout 2.2.0; lifecycle 2.8.7; navigation 2.8.6; recyclerview 1.4.0. → strongly suggests a sibling/companion app (login client or admin front-end) sharing the template, or planned features; **none linked by the app module's own dependencies block** except via the buildscript AGP.

### 15.4 Build toolchain

Gradle 8.5 wrapper; Android Gradle Plugin 8.2.1; `compileSdk 34`, `minSdk 21`, `targetSdk 34`; **NDK 24.0.8215888** (pinned), `ndk-build` (Android.mk), C++20 + `c++_static`, `APP_PLATFORM android-21`, PIE, release; Java 8 source/target; linked libs `-landroid -lGLESv3 -lEGL -ldl -llog -lz`.

---

## 16. Networking/Communication Analysis

### 16.1 Outbound channels (all inside the game process)

| Channel | Peer | Purpose | Verification |
|---|---|---|---|
| HTTPS POST | `axlmods.myvippanel.shop:443` path `/useradmin/connect` | license login (`user_key`, `serial`) | **TLS verification completely disabled** (`VERIFYPEER 0`, `VERIFYHOST 0`) — MITM-trivial; `DEFAULT_PROTOCOL=https` only guards scheme-less URLs; redirects followed |
| HTTPS GET (intent) | `t.me/DARK_OWNER_VIP` | support/sales funnel | OS-handled (browser/Telegram) |
| WebSocket (dormant) | any IP/port (helpers exist) | none observed | unused code |

### 16.2 Login protocol reconstruction (from code)

```
Client → POST /useradmin/connect  (application/x-www-form-urlencoded)
        game=PUBG & user_key=<license> & serial=<UUIDv3(key+androidId+model+brand)>
Server → {"status": true|false,
          "data":  {"token": "...", "rng": <unix ts>, "EXP": "<ts|datetime>"},
          "reason": "<error string>"   (when status=false)}
Client accepts iff: status==true ∧ token≠"" ∧ rng+30 > now ∧ EXP parse > now
```

* Clock-skew hardening: the 30-second `rng` freshness window prevents replay of old responses.
* Device binding: deterministic UUIDv3 seed = key+device ids → server can lock keys to one device fingerprint.
* The `game=PUBG` literal is panel-system boilerplate (the same hosted panels serve many games), again evidencing template reuse.
* No certificate pinning, no HMAC request signing, payload values URL-encoded plain; secrecy relies solely on TLS (which is then deliberately unauthenticated) — see §18.
* **Local IPC:** none (no sockets/binders); external control flows via the JNI `onSendConfig` bridge (§F13) and via files (persistence JSON).

### 16.3 Protocols supported by the bundled curl (but unused)

DICT/FILE/FTP(S)/GOPHER/IMAP(S)/POP3(S)/RTSP/SMB(S)/SMTP(S)/TELNET/TFTP (from `libcurl.pc`).

---

## 17. Configuration Analysis

| Config item | Location | Contents |
|---|---|---|
| Manifest | `AndroidManifest.xml` | perms: `INTERNET`, `SYSTEM_ALERT_WINDOW`, `ACCESS_NETWORK_STATE`, `ACCESS_SUPERUSER`, `FOREGROUND_SERVICE`; `allowBackup=true`, `requestLegacyExternalStorage=true` (targets API 31 note), single exported launcher `MainActivity`; label `@string/app_name` = "DARK OWNER ADMIN SERVER" |
| Gradle | §20 | — |
| ProGuard | `app/proguard-rules.pro` | 5 passes; metadata strip; **kill all android.util.Log**; keep `com.bnmodz.external..MainActivity` (**stale package name from the template — a provenance leak**), keep native methods, keep libsu |
| OpenSSL | `openssl.cnf`(+`.dist`) ×2 | stock 1.1.0 config incl. example `tsa_policy` OIDs; `ssl/private`, `ssl/certs` dirs empty (`.rartemp` placeholders) |
| Persistence (runtime) | `/data/user_de/0/<game pkg>/no_backup/persistence.json`, `style.json`, `.ini` | toggles, ints/floats/colors incl. saved license **`"key"`** (plaintext), ImGui theme index & style sizes |
| svConfig (runtime) | `/data/user/0/<game pkg>/files/svConfig.txt` | `iLineThickness`, `iMenuSizeOffset` |
| IDE | `.androidide/editor/openedFiles.json.bak` | empty — developed with **AndroidIDE** (phone-based IDE) |

Sensitive-angle: license key is stored **unencrypted** in `persistence.json` inside the *game's* private dir (root/other-mod readable), and clipboard is used as the key-entry mechanism (any clipboard-snooping app/mod in the same ecosystem can steal keys).

---

## 18. Security-Relevant Observations

Educational assessment of the mod's own engineering (not exploitation guidance). Severity is judged from the *mod user's* perspective unless noted.

| # | Observation | Where | Why it matters | Severity | Evidence | Defensive remediation |
|---|---|---|---|---|---|---|
| S1 | **TLS certificate validation disabled** on the license client (`SSL_VERIFYPEER 0`, `VERIFYHOST 0`) | keylogin.h Login() | any on-path attacker (hostile Wi-Fi — common for this user base) can forge the panel, capture `user_key`+device serial, and issue unlimited fake "valid" licenses | **High** | quoted curl_setopt calls | enable verification; pin CA/leaf; fail on `CURLE_SSL_*` |
| S2 | **License key stored in plaintext** on disk (`persistence.json` → `"key"`) inside the game package dir | persistence.h | key theft by co-resident mods/root; violates panel ToS | Medium | save/load functions | encrypt with Android Keystore-bound key or don't persist |
| S3 | **Clipboard used as credential channel** + key logged via `LOGI("GetClipboard: %s")` and `LOGI("GetAndroidID: %s")` | java.h, menu.h | logcat exposure of license key and device ID (read-logs on old Android/root) | Medium | log statements | strip logs in release (ProGuard does this for Java but **not for native `__android_log_print`**) |
| S4 | **Device fingerprint upload** (`ANDROID_ID`, `Build.MODEL/BRAND`, UUIDv3) without user notice | keylogin.h | privacy exposure to third-party panel operator | Medium | serial construction | disclose/anonymize; hash-only binding |
| S5 | **XOR "encryption" of secrets in binary** (URL blob key `0x5A`; `xor_encrypt` with repeating key) | keylogin.h | obfuscation ≠ secrecy; trivially reversible (this report decoded it) | Low (design weakness) | decoded §11 | don't rely on client-side obfuscation for trust decisions |
| S6 | **Client-side license gating is bypassable by design** (`g_Token==g_Auth` compare of two literals set to `"1"`; `DEBUG_BYPASS_LOGIN` flag present) | keylogin.h, menu.h | any re-packer can flip the flag / the byte — no server-side feature delivery | High (for author's business model) | code constants | move real assets/server-gated content online; integrity checks |
| S7 | **Anti-analysis that manipulates another process's allocator/linker state** (`_malloc` futex hang, pointer swap at `libloader+0x5c43e0`) | mod/kill.h | fragile by design: version mismatch → game instability/crashes on user devices; ethically hostile code path | Medium (stability) | kill.h | n/a (remove; don't ship tamper code with user products) |
| S8 | **Stale/liberal permissions & flags**: `requestLegacyExternalStorage`, `allowBackup=true`, unused `SYSTEM_ALERT_WINDOW`, `ACCESS_SUPERUSER` request | manifest | widens attack surface of the carrier app unnecessarily | Low-Medium | manifest lines | drop unused perms, `allowBackup=false` |
| S9 | **Ancient vendored crypto/network libs**: curl 7.51.0 & OpenSSL **1.1.0c** (Nov 2016; both EOL with many public CVEs) | prebuilt `.a` | any TLS use inherits ~9 years of published vulnerabilities (esp. with S1) | High | version strings/headers | rebuild with current curl/OpenSSL 3.x |
| S10 | **Hard kill-switch ** `EXPIRY_TS` — the author can/does time-bomb builds; users outside the channel get stranded | menu.h | supply-chain trust issue for users | Low | constant | n/a |
| S11 | **ProGuard keep rule leaks template package** `com.bnmodz.external..MainActivity` | proguard-rules.pro | attribution/heritage leak; also implies this exact class may not exist (rule harmless) | Informational | file content | clean rules per project |
| S12 | **Unsafe memory patterns**: unchecked `realloc` loop in `WriteMemoryCallback`, malloc(1) then free on early errors, `snprintf` into 4096 body w/o escaping of key (form injection if key contains `&`), `char buffer[65536]` WS recv, `fgets` parsers | keylogin.h | robustness/logic issues; form-field injection possible via crafted clipboard key | Low-Medium | code review | bounds-checking, URL-encode fields, bounded reads |
| S13 | **Logging of internal state** (`LOGI` everywhere incl. `libmain` base, hook addresses) under tag `angousana` | logger.h + all | leaks addresses into logcat (ASLR-relevant info for local attackers) | Low | macros | compile-time NDEBUG for release |
| S14 | **JNI bridge lacks caller verification** (`Java_android_service_SurfaceView_onSendConfig`) | main.cpp | any in-process code can flip features; expected inside game, but worth noting | Informational | export list | n/a |
| S15 | **No integrity verification of the panel build** — a single hard-coded panel URL; redirect-following (`FOLLOWLOCATION 1`) to wherever DNS/attackers point (S1 compounds) | keylogin.h | panel operator swaps/re-keys = silent migration; MITM can serve any JSON | Medium | curl flags | pin host + verify TLS; sign responses (HMAC) |

**Secrets sweep result:** no passwords, API tokens, private keys, keystores, or OAuth material exist in the archive. Nothing had to be redacted; the only protected items are (a) the panel URL (documented as infrastructure), (b) end-user license keys (not present).

---

## 19. Resource Analysis

| Resource | Type/Dims | Observed purpose |
|---|---|---|
| `res/drawable/ic_launcher.png` | PNG 736×736 RGBA (163 KB) | launcher icon — circular badge: black disk, silver rim, "Offline" + heart glyph (matches the "offline" theme of floating button) |
| `res/drawable/offline_heart.png` | PNG 736×736 RGBA (198 KB) | floating-pill icon — visually identical artwork to launcher (different bytes: re-exported) |
| `res/drawable/twotone_vpn_key.xml` | vector 24dp | stock Material "vpn key" icon (unused in visible code) |
| `res/values/strings.xml` | 1 string | `app_name` |
| `res/values/styles.xml` + `values-v21/styles.xml` | 1 style | `AppTheme` (Material.Light.NoActionBar) |
| `res/values/colors.xml` | empty resources tag | no colors defined (everything is programmatic) |
| `res/layout/activity_main.xml` | empty LinearLayout | unused |
| `jni/icons/*.png` (7) | coin_100 44×44; mod_calendar/india/lightning/lock/telegram 64×64 RGBA | source images for the embedded arrays below |
| `jni/icons/*.h` (16 headers, ~2.0 MB total) | C arrays of PNG bytes: `logo_png`, `draw_icon_png`, `play_icon_png`, `q_icon_png`, `user_icon_png`, `play_on/play_off`, `coin_100/100m/200m`, `mod_india/lightning/telegram/lock/calendar` | GL textures for sidebar tabs, floating buttons, stake cards, info rows (loaded via `LoadTextureFromMemory` + stb_image); spot-check: `coin_100.h` contains the PNG byte stream (4,691 B png vs 4,711 B embedded incl. padding/formatting delta — content matches format) |
| Fonts | none bundled | ImGui default bitmap font @40 px |
| Audio/video | none | n/a |
| OpenSSL misc | `CA.pl`, `tsget`, `openssl.cnf(.dist)` | dead weight from the vendored OpenSSL distribution |
| Notices | `Importent_Notice.txt` ×8 (identical, 1,634 B) | branding/advert ("Private source ke liye direct contact karein… TELEGRAM @DARK_OWNER_VIP… PROUDLY MADE FOR INDIA") |
| `LICENSE` | GPLv3 full text | stock license file |

---

## 20. Build System

| Item | Value (verified) |
|---|---|
| Build system | Gradle 8.5 (wrapper) + AGP 8.2.1 + **ndk-build** (`externalNativeBuild.ndkBuild path 'src/main/jni/Android.mk'`) |
| Module | `:app`, namespace/appId `com.eightballpool.bp`, versionCode 1, versionName 1.0 |
| SDK/ABI | compileSdk/target 34, minSdk 21, `abiFilters 'arm64-v8a'` |
| NDK | pinned `24.0.8215888`; `Application.mk`: `APP_ABI arm64-v8a`, `APP_STL c++_static`, `APP_OPTIM release`, `APP_PIE true`, `APP_PLATFORM android-21`, `-std=c++20 -frtti -fexceptions` |
| Native target | `LOCAL_MODULE := MarkXit` → `libMarkXit.so` (`BUILD_SHARED_LIBRARY`), sources: `main.cpp` + dlfcn + imgui amalgam + Substrate(3) + And64InlineHook + xhook.c + oxorany.cpp; flags: `-fvisibility=hidden -ffunction-sections -fdata-sections -w -fno-exceptions/-fexceptions (contradictory) -rdynamic -funwind-tables`, LDFLAGS `--strip-all`, links `-landroid -lGLESv3 -lEGL -ldl -llog -lz` + static curl/ssl/crypto |
| Release hardening | `minifyEnabled`, `shrinkResources`, default ProGuard + custom rules |
| Repos | google, **jcenter** (defunct), jitpack |
| IDE provenance | **AndroidIDE** (`.androidide` dir, wrapper duplicated inside `app/`, mobile-built project); gradle.properties forced UTF-8/en-US with a 2026-03-15 timestamp comment |
| Build verification | **Not performed** (per §24: no SDK/NDK/game in sandbox; static analysis only). "Builds successfully" is therefore **not claimed**. Static inspection found: unused armeabi-v7a prebuilts would still be declared as `PREBUILT_STATIC_LIBRARY` per-ABI at build time (harmless), `jni/include/features.h` etc. unused, and `AutoQueue.h`'s `DEFINE(...popMenuState...)`/duplicate symbols are managed by macro naming; one latent issue — `keylogin.h` references `ManualLookup`/`O()` macros requiring `include/obfuscate.h` include-order provided transitively by `menu.h` (plausible but unverified by compiler). |

---

## 21. Timeline / Execution Flow

**Verified (code-traced) host-process flow** when `libMarkXit.so` is loaded inside `com.miniclip.eightballpool`:

1. **Load & `JNI_OnLoad`** — store `JavaVM*`; synchronously execute hidden anti-analysis installer (`__KILL__` via hashed lookup); spawn detached thread `__1__`. *(Verified)*
2. **+2 s** — read `/proc/self/cmdline` → `PACKAGE_NAME`; create `/data/user_de/0/<pkg>/no_backup`. *(Verified)*
3. **+12 s (approx.)** — `get8BPbase()` polls `/proc/self/maps` (1 s intervals) until the game's main library appears (largest mapping under the game package with ELF magic) → `libmain`. *(Verified)*
4. `__HOOKS__()` — inline-hook `libmain+0x2d911e0` (GameManager capture) and `libmain+0x3068c94` (StartMatch); xhook-PLT-hook `eglSwapBuffers`. *(Verified)*
5. `__INPUT__()` — xhook the three `MCInput` touch JNI exports. *(Verified)*
6. Signal handlers installed (trace on ABRT/ILL/FPE/BUS/SYS/TRAP/XCPU/XFSZ; custom SIGSEGV with libwolf freeze). *(Verified)*
7. **First frame** — hooked `Draw()` measures the EGL surface, builds ImGui (theme, fonts 40 px, loads persistence JSON + svConfig), sets ini path. *(Verified)*
8. **Gate evaluation (every frame):** `now ≥ 1788805800` → expired screen → **stop**. *(Verified)*
9. **Pre-login:** login card → user copies key → "ENTER KEY" → detached thread `Login(androidID, clipboard)` → VPN guard → fingerprint+POST → success sets `logged_in` (+`g_Token/g_Auth="1"`) and stores server expiry → subsequent frames render the mod. Failure writes `ERROR_MESSAGE` (server `reason` or curl error). *(Verified)*
10. **In menus (not in game):** floating logo button always visible; if AutoQueue armed and not already queuing → 1.2 s countdown → `StartAutoQueue()` → hooked `StartMatch` invoked with selected `M*` mode; cancel button available. *(Verified)*
11. **In game (state 4/own turn):** per-frame `determineShotResult` → trajectory/pocket ESP; if AutoPlay enabled → floating play button toggles `bAutoPlaying` → scan (fast geometric, else slow sweep +CALCULATING overlay) → nominate pocket (tap injection) → `takeShot` writes angle/power and calls the game's shot fn. States 6–8 (roll/opponent) pause ESP work. *(Verified)*
12. **Any config change** → `save_persistence()` (and manual "Save Config" → svConfig). *(Verified)*
13. **Shutdown:** no native cleanup exists (process-lifetime hooks; items like `removeFloatingBtn` exist only in the carrier app's `onDestroy`); the mod never unhooks. *(Verified — absence of cleanup paths)*

**Inferred (not in source):** step 0 — the loader that injects the `.so` into the game (root/Zygisk/injector app) — **Not determined from available evidence.**

---

## 22. Important Files

Ranked by analytical importance:

| File | Why it matters |
|---|---|
| `jni/main.cpp` | entry point, hook installation, JNI bridge |
| `jni/menu.h` | the whole product UI + ESP + gating logic |
| `jni/mod/keylogin.h` | monetization/auth, network, VPN guard, crypto-ish helpers |
| `jni/mod/kill.h` | most sensitive anti-tamper code |
| `jni/game/inc/AutoPlay.h` | the aim-bot decision engine |
| `jni/game/inc/Prediction.fast.h` / `Prediction.h` | physics predictor (cloned game logic) |
| `jni/game/inc/AutoQueue.h` | match auto-join |
| `jni/game/*.h` | offset model of the game's objects |
| `jni/include/includes.h` + `hook.h` + `pointers.h` | runtime plumbing, maps-parsing, macros, crash handling |
| `jni/include/input.h` | touch capture/injection |
| `jni/include/manual_dlsym.h` + `random_names.h` | symbol stealth machinery |
| `jni/Android.mk` / `Application.mk` | native build definition |
| `app/build.gradle` / `proguard-rules.pro` | packaging + hardening (incl. template leak) |
| `MainActivity.java` | carrier UI + Telegram funnel |
| `jni/icons/*` | visual identity of the menu |
| `8bp_analysis/FILE_INVENTORY.csv` | full 505-file manifest (artifact of this analysis) |

---

## 23. Key Findings

1. **It is a commercialized 8 Ball Pool cheat**: trajectory-ESP, a rules-aware AutoPlay bot executing shots through the game's own functions, and a table AutoQueue bot, behind a remote license gate with monthly-key economics and a hard build expiry. *(Confirmed)*
2. **In-process overlay architecture**: everything runs inside the game via `eglSwapBuffers` hooking + ImGui over the game's GL context, with game-state mirrored through a precise hand-built offset model of the game's ObjC++ objects. *(Confirmed)*
3. **Heavy stealth/anti-analysis investment**: three obfuscation layers, hashed-symbol invocation, ~40 decoy exports, raw syscalls, loader-level `dlsym`/`malloc` tampering that parks a specific game allocation in `futex` sleep, crash-freeze on the game's `libwolf.so`, and log-stripped release dex. *(Confirmed)*
4. **Weak secrets engineering**: TLS verification disabled, plaintext key storage, clipboard-as-credential-channel with native logging of key+Android ID — the license system protects the author's revenue only against casual users, not targeted extraction. *(Confirmed)*
5. **Template provenance is fully traceable**: Roman-commented floating-button tutorial code, PUBG-oriented panel field (`game=PUBG`), leftover FPS-cheat setting keys (aimbot/BulletTrack/Xray), a stale ProGuard keep for `com.bnmodz.external..MainActivity`, and prebuilt xhook objects stamped `/home/cici/AppProjects/Bloodstrike-static-v2/…` — a Blood Strike cheat tree. The "8BP" skin is the latest re-target of a multi-game mod framework. *(Confirmed from literals)*
6. **Stale/ancient dependencies**: curl 7.51.0 + OpenSSL 1.1.0c (2016), jcenter repo, unused Firebase/Retrofit/jbcrypt catalog entries with no wiring. *(Confirmed)*
7. **Distribution funnel**: Telegram-centric sales (@DARK_OWNER_VIP), "PROUDLY MADE FOR INDIA" market positioning, "Importent_Notice" advert duplicated eight times. *(Confirmed)*
8. **The visible sources lack the injector**: how `libMarkXit.so` gets into the game (Zygisk module, virtual-app hook framework, or manual root injection) is not part of this drop — only breadcrumbs (zygisk.hpp, libsu, ACCESS_SUPERUSER) exist. *(Confirmed absence; mechanism inferred)*

---

## 24. Limitations

* **No dynamic analysis was performed** — nothing was compiled, executed, emulated, or injected. All behavioral statements are code-trace conclusions; runtime confirmation (e.g., which allocation site `kill.h` actually freezes, exact game version compatibility of the `libmain` offsets) was not possible.
* **The target game's binary is not in the archive.** Every `libmain+0x…` offset is a source constant applicable to the author's version of 8 Ball Pool (self-annotated "v56.13.0"/"5.8.0" in places) and is not independently verifiable here; offsets rot with each game update.
* AArch64 disassembly coverage was targeted (xhook objects) rather than exhaustive across the ~772 members of the five OpenSSL/curl archives; those are stock builds, and their identity was pinned via version strings/build metadata instead of full audit.
* The `imgui.cpp` amalgam and the third-party trees were **reviewed for version/provenance, not line-audited** (≈780k LOC bundled). Custom modifications inside `imgui/inc/*`, `custom_theme.cpp`, and `persistence.h` were read fully.
* UI screenshots of the running menu could not be produced (needs a device + game); the UI description is reconstructed from layout code and constants.
* Anything labeled "Not determined from available evidence" in-line is exactly that — reported unknown rather than guessed.

---

## 25. Final Assessment

`DARK_ADMIN` is a **mature, production-grade game-cheat source drop**, professionally organized for resale: a thin Java carrier, a single-translation-unit native payload, remote licensing with device binding and time-bombs, aggressive stealth, and a polished gold-on-cream menu. Technically it demonstrates competent low-level Android work — clean event-driven physics cloning, careful frame-scheduled bots, resilient pointer-laundering with runtime class-name validation, and layered anti-analysis.

From a security standpoint the same codebase is a double liability: **for users** it weakens TLS guarantees, persists credentials in plaintext, uploads device fingerprints to a third-party panel, and ships end-of-life crypto; **for the ecosystem** it is cheat-ware whose use violates the game's terms of service and whose distribution channel (Telegram, time-bombed beta builds) offers buyers no recourse. For defensive work, the report furnishes concrete detection anchors: the `angousana` log tag, hard modal constants (`EXPIRY_TS`, `0x7100000001`, `0x1598` futex trap), the `axlmods.myvippanel.shop` endpoint, the `M1–M17` direct `StartMatch` invocation pattern, hooks at `eglSwapBuffers` + `MCInput` JNI symbols, and the xhook/And64InlineHook signatures bundled in the tree.

*Report generated from static analysis only; all technical statements reference the evidence sections above.*

---
### Appendix A — Companion artifacts in this repository

| Path | Contents |
|---|---|
| `8bp_analysis/FILE_INVENTORY.csv` | all 505 files: path, size, classification, SHA-256 |
| `8bp_analysis/HASHES_SHA256.txt` | flat SHA-256 manifest |
| `8bp_analysis/INTERESTING_STRINGS.txt` | categorized strings/constants with locations |

*(The original archive `8BP_SRC_PUBLIC_BY_DARK_OWNER (3).zip` and the extraction workspace are intentionally not committed to version control.)*
