# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 專案概觀

VEX V5 機器人 C++ 專案，目標平台為 `vexv5`，使用 ARM bare-metal toolchain（clang 編譯 + `arm-none-eabi-ld` 連結），最終產生上傳到 V5 Brain 的 `.bin` 檔。`.vscode/vex_project_settings.json` 紀錄此專案綁定 SDK 版本 `V5_20240802_15_00_00`、slot 1，並啟用 `printf_float`。

整合了 [JAR-Template](https://github.com/JacksonAreaRobotics/JAR-Template)（MIT 授權）作為底盤函式庫，提供 `Drive` / `Odometry` / `PID` / `util` 等模組，支援 Tank / Holonomic 與 8 種 odometry 配置。

## 常用指令

```bash
# 預設 build：產出 build/<PROJECT>.bin（PROJECT 預設取自當前目錄名稱）
make

# 顯示完整編譯指令（debug toolchain 用）
make VERBOSE=1

# 覆寫專案名稱或 SDK 路徑（在容器/CI 中常需要）
make P=vax_ai_car5 T=$HOME/sdk

# 清除 build/ 目錄
make clean
```

注意事項：
- `PROJECT` 名稱**不能含空白**（`vex/mkenv.mk:25` 會直接報錯），所以不要在路徑含空白的目錄下執行 `make` 而不指定 `P=`。當前路徑包含 `OneDrive-NTHU` 的空白與底線屬正常，但 `CURDIR` 若含空白會觸發此檢查 —— 透過 `P=` 明確指定可避免。
- `VEX_SDK_PATH` 預設為 `$HOME/sdk`；本機未安裝 SDK 時，編譯會在第一個 `-I"$(VEX_SDK_PATH)/..."` 失敗。VS Code 的 `c_cpp_properties.json` 走的是 VEXcode 擴充套件設定（`${config:vexrobotics.vexcode.Cpp.Sdk.Home}`），與命令列 `make` 的路徑來源不同 —— 兩邊要分別設定。
- 此專案無單元測試框架；驗證行為的方式是上傳到 V5 Brain 後在實機觀察（VEXcode 擴充套件提供 Build/Download/Run 按鈕）。

## 架構重點

VEXcode 標準三層 makefile 結構，未來新增程式碼時不需動到 `vex/` 內任何檔案：

- `makefile`（root）：只負責列出 `SRC_C`（遞迴 glob `src/*.cpp`、`src/*/*.cpp` 等）與 `INC_F`，再 include `vex/mkenv.mk` + `vex/mkrules.mk`。
- `vex/mkenv.mk`：定義 toolchain（`clang` for compile、`arm-none-eabi-ld` for link）、ARM v7-A + NEON + softfp 的目標旗標、`-DVexV5` macro、以及連結時用的 `lscript.ld` 與 `stdlib_0.lib`。
- `vex/mkrules.mk`：`%.cpp → build/%.o → build/$(PROJECT).elf → build/$(PROJECT).bin` 的 pipeline。

新增原始檔只需放到 `src/` 或其子目錄即可被 glob 捕捉到；新增 header 放 `include/`（會被 `INC_F` 加進 `-I`）。**不要**改動 `vex/` 內檔案，那是 VEXcode 擴充套件管理的範本。

`include/vex.h` 是專案唯一的入口 header，依序包裝：`v5.h` / `v5_vcs.h`、`robot-config.h`、JAR-Template 全部四個模組 header、`autons.h`。**所有 `.cpp` 只需 `#include "vex.h"` 就會拿到全部 API**。同時定義兩個常用 macro：
- `waitUntil(cond)`：每 5ms 檢查一次條件，會 block 當前 task。
- `repeat(n)`：簡化的固定次數迴圈。

### 三個專案層檔案的職責分工

- **`include/robot-config.h` + `src/robot-config.cpp`** — 宣告所有硬體實例（`motor`、`inertial`、`rotation`、`encoder` 等）。`robot-config.cpp` 用 `motor LeftFront = motor(PORT1, ratio6_1, false);` 風格定義並用 `extern` 在 header 露出，再由 `Brain` 與其他全域物件共用。`vexcodeInit()` 用於開機初始化（目前是空殼）。**換車體時主要改這裡。**
- **`src/main.cpp`** — 三件事：(1) 建立全域 `Drive chassis(...)`，把 motor group、感測器、輪徑、傳動比、odometry tracker 距離全部塞進 constructor；(2) 註冊 `Competition.autonomous(autonomous)` 與 `Competition.drivercontrol(usercontrol)` callback；(3) `pre_auton()` 在 Brain 螢幕跑 auton 選單（點螢幕切換 `current_auton_selection`），`autonomous()` 用 `switch` 對應到 `autons.cpp` 的具名函式，`usercontrol()` 預設呼叫 `chassis.control_arcade()`。
- **`include/autons.h` + `src/autons.cpp`** — 自動程式的函式集合（`drive_test()` / `turn_test()` / `odom_test()` 等）。`default_constants()` 也在這裡，呼叫 `chassis.set_drive_constants(...)` 與 `chassis.set_*_pid_constants(...)` 設定 PID 參數。**調整自動程式行為與 PID 常數時改這裡。**

### JAR-Template 的關鍵 API（位於 `include/JAR-Template/drive.h`）

`Drive` class 是入口；常用方法：
- 自動程式：`drive_distance(inches)` / `turn_to_angle(deg)` / `swing_to_angle(...)` / `holonomic_drive_to_point(x, y, deg)` / `drive_to_point(x, y)`（後兩者需 odometry）。
- 遙控：`control_tank()` / `control_arcade()` / `control_holonomic()`。
- PID 參數：`set_drive_constants(...)`、`set_heading_constants(...)`、`set_turn_constants(...)`、`set_swing_constants(...)`、`set_drive_exit_conditions(...)` 等 — 不要把 PID 常數寫死在自動程式裡，集中放在 `default_constants()`。
- 狀態查詢：`get_absolute_heading()`、`get_X_position()`、`get_Y_position()`。

修改 motor port、輪徑、傳動比、tracker 配置等物理常數時**只能改 `src/main.cpp` 的 `Drive chassis(...)` constructor**，不要動 `src/JAR-Template/drive.cpp`。

## 與使用者溝通

回覆與生成檔案請使用**繁體中文**。
