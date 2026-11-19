# AGENTS.md — Game Platformer 2D Co-op 2 Người Chơi (SDL3 + CMake)

> Cập nhật theo yêu cầu người dùng: chỉ giữ asset đang dùng trong `assets/characters/{raccoon,bird}`, `assets/backgrounds`, `assets/terrain`, `assets/traps`, `assets/maps`; âm thanh tương lai chia `assets/audio/{sfx,music}`. P1 gấu mèo, P2 chim xanh; background/terrain từ Pixel Adventure 1. Đã bỏ ZIP, PNG dư và script chuyển đổi. Giữ SDL3 duy nhất; các quy định procedural/không asset và đường dẫn levels bên dưới là đặc tả gốc, ưu tiên cấu trúc mới này. Phần âm thanh giữ nguyên để người dùng tự xử lý theo `docs/AUDIO.md`.

> Tài liệu này là **đặc tả đầy đủ** để Codex CLI tự xây dựng game từ đầu đến cuối.
> Hãy đọc toàn bộ trước khi viết code. Làm theo thứ tự các **Milestone** ở mục 12; sau mỗi milestone phải build được và chạy được.

---

## 1. Tổng quan dự án

- **Tên mã dự án:** `twinleap`
- **Thể loại:** Platformer 2D, co-op **2 người chơi trên cùng một máy** (local), chia sẻ bàn phím và/hoặc dùng gamepad.
- **Ngôn ngữ:** C++17 (không dùng exception cho luồng game, không RTTI nặng).
- **Thư viện:** **SDL3 duy nhất** (không SDL_image, không SDL_ttf, không SDL_mixer). Mọi đồ họa vẽ bằng hình chữ nhật/đường thẳng/pixel procedural; chữ dùng `SDL_RenderDebugText`.
- **Build system:** CMake ≥ 3.21.
- **Nền tảng mục tiêu:** Linux, Windows (MSVC/MinGW), macOS. Ưu tiên chạy được trên Linux trước.
- **Không cần asset file ngoài** ngoài các file level `.txt`. Game phải chạy được chỉ với thư mục `assets/maps/`.

### Ý tưởng gameplay

Hai nhân vật (P1 màu đỏ, P2 màu xanh) phải **cùng nhau** vượt qua từng màn:

- Cả hai phải **cùng chạm cửa thoát (Exit)** để qua màn.
- Một người có thể **đứng lên đầu** người kia để với tới chỗ cao.
- **Nút bấm (button)** mở **cửa (door)** — một người phải đứng giữ nút để người kia đi qua.
- Thu thập **đồng xu (coin)** để lấy điểm chung.
- Chạm **gai (spike)** hoặc rơi khỏi màn hình → người đó hồi sinh tại **checkpoint** gần nhất (có thời gian chờ ngắn), game không thua hẳn.

---

## 2. Quy tắc làm việc cho Codex

1. **Luôn giữ project build được.** Sau mỗi thay đổi lớn, chạy lệnh build ở mục 4.
2. Không thêm dependency ngoài SDL3. Không dùng Boost, STL ngoài chuẩn, không dùng code sinh tự động.
3. Không để lại `TODO` mơ hồ. Nếu hoãn việc gì, ghi vào mục "Known Issues" trong `README.md`.
4. **Tách riêng header và nguồn:** `.hpp` chỉ nằm trong `include/twinleap/...`, `.cpp` chỉ nằm trong `src/...` với cùng cấu trúc thư mục con (xem mục 3). Mỗi `.cpp` có `.hpp` tương ứng (trừ các file header-only đã liệt kê), dùng `#pragma once`.
5. Không dùng biến global thay đổi được (trừ hằng số `constexpr`). Truyền state qua struct/class.
6. Kiểm tra giá trị trả về của mọi hàm SDL có thể thất bại; khi lỗi dùng `SDL_Log("... %s", SDL_GetError())` rồi thoát gọn gàng.
7. Giải phóng tài nguyên đúng thứ tự (RAII wrapper hoặc hàm shutdown rõ ràng).
8. Tách **logic** (không phụ thuộc SDL render) khỏi **render** để dễ test.
9. Commit nhỏ, thông điệp rõ (nếu repo có git): `feat:`, `fix:`, `refactor:`, `docs:`.
10. Khi gặp mơ hồ trong đặc tả, chọn phương án đơn giản nhất và ghi quyết định vào `docs/DECISIONS.md`.

---

## 3. Cấu trúc thư mục

**Nguyên tắc:** file header (`.hpp`) và file nguồn (`.cpp`) nằm ở **hai cây thư mục riêng**, cấu trúc con bên trong phản chiếu nhau 1-1.

- `include/twinleap/<module>/*.hpp` — toàn bộ header (giao diện công khai của từng module).
- `src/<module>/*.cpp` — toàn bộ file cài đặt; `src/main.cpp` là điểm vào.
- Header chỉ có khai báo + `constexpr`/`inline` ngắn. Các file **header-only** (không có `.cpp` tương ứng) được liệt kê rõ bên dưới.

```
twinleap/
├── AGENTS.md                      # file này
├── README.md                      # hướng dẫn build/chơi, Known Issues
├── CMakeLists.txt
├── cmake/
│   └── FetchSDL3.cmake            # (tuỳ chọn) logic find/fetch SDL3
├── docs/
│   └── DECISIONS.md
├── levels/
│   ├── 01_first_steps.txt
│   ├── 02_buttons.txt
│   ├── 03_tower.txt
│   └── 04_gauntlet.txt
│
├── include/                       # ===== TẤT CẢ HEADER (.hpp) =====
│   └── twinleap/
│       ├── core/
│       │   ├── App.hpp            # vòng lặp chính, khởi tạo/tắt SDL
│       │   ├── Time.hpp           # (header-only) fixed timestep helper
│       │   ├── Input.hpp          # input abstraction cho 2 người chơi
│       │   └── Math.hpp           # (header-only) Vec2, Rect, clamp, approach
│       ├── game/
│       │   ├── GameState.hpp      # (header-only) interface State + enum
│       │   ├── MenuState.hpp
│       │   ├── PlayState.hpp      # gồm cả pause / màn hoàn thành
│       │   ├── Level.hpp          # tilemap, parse + validate file level
│       │   ├── Player.hpp         # trạng thái nhân vật
│       │   ├── Physics.hpp        # hằng số vật lý + hàm va chạm AABB
│       │   ├── Entities.hpp       # Coin, Button, Door, Checkpoint, Exit
│       │   ├── Camera.hpp
│       │   └── Particles.hpp
│       ├── render/
│       │   ├── Renderer.hpp       # wrapper vẽ: rect, text, background
│       │   └── Palette.hpp        # (header-only) bảng màu constexpr
│       └── audio/
│           └── Sfx.hpp            # (tuỳ chọn, milestone cuối)
│
├── src/                           # ===== TẤT CẢ FILE NGUỒN (.cpp) =====
│   ├── main.cpp                   # điểm vào, chỉ khởi tạo App và chạy
│   ├── core/
│   │   ├── App.cpp
│   │   └── Input.cpp
│   ├── game/
│   │   ├── MenuState.cpp
│   │   ├── PlayState.cpp
│   │   ├── Level.cpp
│   │   ├── Player.cpp
│   │   ├── Physics.cpp
│   │   ├── Entities.cpp
│   │   ├── Camera.cpp
│   │   └── Particles.cpp
│   ├── render/
│   │   └── Renderer.cpp
│   └── audio/
│       └── Sfx.cpp
│
└── tests/
    ├── CMakeLists.txt
    ├── test_physics.cpp           # test logic thuần, không cần cửa sổ
    ├── test_level_parser.cpp
    └── test_buttons_doors.cpp
```

### Quy ước include

- Trong mọi file, include header của project bằng đường dẫn đầy đủ tính từ `include/`, dùng dấu ngoặc kép:
  `#include "twinleap/game/Physics.hpp"` (**không** dùng `../` hay đường dẫn tương đối).
- File `.cpp` include header tương ứng của nó **đầu tiên** (để phát hiện header thiếu include), sau đó header project khác, sau đó SDL, sau đó thư viện chuẩn.
- Header của project **không** được include `<SDL3/SDL.h>` nếu không cần thiết; ưu tiên forward-declare (`struct SDL_Renderer;`) hoặc chỉ include khi header thực sự dùng kiểu SDL (ví dụ `SDL_FRect`, `SDL_Event`). Header thuần logic (`Math.hpp`, `Physics.hpp`, `Level.hpp`, `Player.hpp`) **không phụ thuộc SDL** để test được mà không cần SDL.
- Không đặt file `.cpp` trong `include/` và không đặt file `.hpp` trong `src/` (ngoại lệ duy nhất: header nội bộ không công khai thì đặt cạnh `.cpp` với hậu tố `_internal.hpp`, và phải ghi lý do trong `docs/DECISIONS.md`).

---

## 4. CMake & Build

### 4.1. `CMakeLists.txt` gốc (yêu cầu)

- `cmake_minimum_required(VERSION 3.21)`, `project(twinleap LANGUAGES CXX)`.
- `set(CMAKE_CXX_STANDARD 17)`, `CMAKE_CXX_STANDARD_REQUIRED ON`, `CMAKE_CXX_EXTENSIONS OFF`.
- Mặc định `CMAKE_BUILD_TYPE=Release` nếu không chỉ định (với generator đơn cấu hình).
- Xuất `compile_commands.json` (`CMAKE_EXPORT_COMPILE_COMMANDS ON`).
- **Tìm SDL3 theo thứ tự:**
  1. `find_package(SDL3 3.2 CONFIG QUIET)` — dùng bản cài sẵn trên hệ thống.
  2. Nếu không thấy → `FetchContent` tải từ GitHub `https://github.com/libsdl-org/SDL.git` với `GIT_TAG` là một tag ổn định `release-3.2.x` (chọn bản 3.2.x mới nhất mà bạn biết tồn tại; tối thiểu `release-3.2.0`), `GIT_SHALLOW TRUE`. Trước khi `FetchContent_MakeAvailable`, đặt `SDL_SHARED OFF`, `SDL_STATIC ON`, `SDL_TEST_LIBRARY OFF`, `SDL_TESTS OFF`, `SDL_INSTALL OFF` (dạng `set(... CACHE BOOL "" FORCE)`).
- Thêm option `TWINLEAP_FETCH_SDL3` (mặc định `ON`) cho phép tắt fetch tự động.
- Link target: `SDL3::SDL3` (nếu dùng static qua FetchContent thì dùng `SDL3::SDL3-static` hoặc alias `SDL3::SDL3` tuỳ cái nào có; viết code CMake kiểm tra `if(TARGET SDL3::SDL3)` rồi mới fallback).
- Tạo thư viện `twinleap_core` (OBJECT/STATIC) chứa toàn bộ logic ngoài `main.cpp`, và executable `twinleap` link với nó. Test link với `twinleap_core` luôn.
- **Khai báo nguồn và include rõ ràng (không dùng `GLOB` mù quáng):**
  - Liệt kê tường minh từng file `.cpp` trong `src/` (qua `target_sources` hoặc biến `TWINLEAP_SOURCES`); khi thêm file mới phải cập nhật danh sách này.
  - `target_include_directories(twinleap_core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)` để mọi target (exe, tests) có thể `#include "twinleap/..."`.
  - `main.cpp` thuộc target `twinleap`; mọi `.cpp` còn lại thuộc `twinleap_core`.
  - Tuỳ chọn: dùng `target_sources(... FILE_SET HEADERS ...)` (CMake ≥ 3.23) hoặc liệt kê header để IDE hiển thị đầy đủ.
- Bật cảnh báo:
  - GCC/Clang: `-Wall -Wextra -Wpedantic -Wshadow -Wconversion` (cho phép tắt `-Wconversion` nếu quá ồn, ghi vào DECISIONS.md).
  - MSVC: `/W4`.
- Sau khi build, **copy thư mục `assets/maps/`** cạnh executable bằng `add_custom_command(TARGET twinleap POST_BUILD ...)` hoặc `file(COPY)`; ngoài ra game phải tìm level theo thứ tự: `./levels`, `<thư mục exe>/levels`, `<thư mục exe>/../levels` (dùng `SDL_GetBasePath()`).
- Trên Windows, nếu link động thì copy `SDL3.dll` cạnh exe (`add_custom_command` với `$<TARGET_FILE:SDL3::SDL3>`); ưu tiên static để khỏi cần.
- Option `TWINLEAP_BUILD_TESTS` (mặc định `ON`) → `enable_testing()` + `add_subdirectory(tests)`; test dùng `add_test`, không cần framework (dùng `assert` hoặc macro CHECK tự viết, thoát mã ≠ 0 khi fail).

### 4.2. Lệnh build chuẩn (ghi vào README)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/twinleap
ctest --test-dir build --output-on-failure
```

### 4.3. Yêu cầu hệ thống Linux (ghi vào README)

Nếu build SDL3 từ source, cần các gói dev (X11/Wayland, ALSA/Pulse, …). Ghi lệnh gợi ý cho Debian/Ubuntu, ví dụ:
`sudo apt install build-essential cmake pkg-config libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxss-dev libwayland-dev libxkbcommon-dev libasound2-dev libpulse-dev libudev-dev libdrm-dev libgbm-dev libgl1-mesa-dev`.

---

## 5. Lưu ý API SDL3 (RẤT QUAN TRỌNG — khác SDL2)

Codex phải dùng đúng API SDL3, **không** dùng cú pháp SDL2:

| Việc | SDL3 đúng |
|---|---|
| Include | `#include <SDL3/SDL.h>` |
| Init | `SDL_Init(SDL_INIT_VIDEO \| SDL_INIT_GAMEPAD)` — trả về `bool` (`true` = thành công) |
| Cửa sổ | `SDL_CreateWindow(title, w, h, flags)` (không có x, y) |
| Renderer | `SDL_CreateRenderer(window, nullptr)` |
| VSync | `SDL_SetRenderVSync(renderer, 1)` |
| Độ phân giải logic | `SDL_SetRenderLogicalPresentation(r, 480, 270, SDL_LOGICAL_PRESENTATION_LETTERBOX)` |
| Vẽ hình chữ nhật | `SDL_RenderFillRect(r, const SDL_FRect*)` — **float**, dùng `SDL_FRect` |
| Màu | `SDL_SetRenderDrawColor(r, R, G, B, A)` |
| Trạng thái phím | `const bool* keys = SDL_GetKeyboardState(nullptr);` rồi `keys[SDL_SCANCODE_A]` |
| Sự kiện | `SDL_EVENT_QUIT`, `SDL_EVENT_KEY_DOWN`, `SDL_EVENT_KEY_UP`, `event.key.scancode`, `event.key.repeat` |
| Gamepad | `SDL_GetGamepads(&count)`, `SDL_OpenGamepad(id)`, `SDL_CloseGamepad`, `SDL_GetGamepadButton(pad, SDL_GAMEPAD_BUTTON_SOUTH)`, `SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX)`, sự kiện `SDL_EVENT_GAMEPAD_ADDED/REMOVED` |
| Thời gian | `SDL_GetTicksNS()` hoặc `SDL_GetPerformanceCounter()` / `SDL_GetPerformanceFrequency()` |
| Chữ debug | `SDL_RenderDebugText(r, x, y, "text")` (font 8×8 tích hợp; scale bằng `SDL_SetRenderScale` nếu cần) |
| Đường dẫn exe | `SDL_GetBasePath()` (trả về `const char*`, **không** cần free trong SDL3) |
| Đọc file | Dùng `std::ifstream` hoặc `SDL_LoadFile` (nhớ `SDL_free`) |
| Hủy | `SDL_DestroyRenderer`, `SDL_DestroyWindow`, `SDL_Quit` |

Ngoài ra:
- Nếu không chắc một hàm SDL3 có tồn tại, **tra header** trong `build/_deps/sdl3-src/include/SDL3/` hoặc trong thư mục include của hệ thống trước khi dùng. Không đoán tên hàm.
- Dùng vòng lặp `main` thông thường (`int main(int, char**)`). **Không** bắt buộc dùng mô hình `SDL_MAIN_USE_CALLBACKS`.
- Nếu dùng `SDL_main`, include `<SDL3/SDL_main.h>` đúng chỗ (chỉ trong file chứa `main`).

---

## 6. Thiết kế game chi tiết

### 6.1. Màn hình & đơn vị

- **Độ phân giải logic:** `480 × 270` (letterbox, giữ tỉ lệ 16:9). Cửa sổ mặc định `1280 × 720`, cho phép resize; phím `F11` bật/tắt fullscreen.
- **Tile size:** `16 × 16` px logic. Màn hình hiển thị tối đa 30 × ~17 tile.
- Đơn vị vị trí/vận tốc: **pixel logic**, `float`. Vận tốc: px/giây. Gia tốc: px/giây².
- Gốc tọa độ ở góc trên trái, trục Y hướng xuống.

### 6.2. Vòng lặp & thời gian

- **Fixed timestep:** `dt = 1/120` giây cho update logic; render theo vsync.
- Tích lũy thời gian (accumulator), giới hạn tối đa 0.25s mỗi khung để tránh "spiral of death".
- Render có thể nội suy (interpolation) vị trí — tuỳ chọn; nếu không làm, cứ vẽ vị trí logic mới nhất.

### 6.3. Điều khiển

Mỗi người chơi có một `PlayerInput` (đã xử lý) gồm: `moveX` (-1..1), `jumpPressed` (cạnh lên), `jumpHeld`, `jumpReleased` (cạnh xuống).

**Bàn phím (cùng một bàn phím):**

| Hành động | Player 1 | Player 2 |
|---|---|---|
| Trái | `A` | `←` |
| Phải | `D` | `→` |
| Nhảy | `W` hoặc `Space` | `↑` hoặc `Right Ctrl` |

**Chung:** `Esc` = Pause/Menu, `R` = restart màn, `F11` = fullscreen, `F3` = bật/tắt overlay debug (hitbox, FPS), `Enter` = xác nhận menu.

**Gamepad:**
- Gamepad đầu tiên được cắm → gán cho P1 *thêm vào* bàn phím; gamepad thứ hai → P2. (Bàn phím vẫn luôn hoạt động.)
- Trái/phải: D-pad hoặc stick trái (deadzone `0.25`).
- Nhảy: nút `SOUTH` (A/Cross). `START` = Pause.
- Xử lý hot-plug: `SDL_EVENT_GAMEPAD_ADDED/REMOVED` mở/đóng gamepad an toàn, không crash.

### 6.4. Nhân vật (Player)

- Hitbox: `12 × 14` px (hơi nhỏ hơn 1 tile để dễ lách). Vẽ bằng hình chữ nhật bo góc giả (rect + vài pixel), có "mắt" nhìn theo hướng chạy.
- Màu: P1 đỏ `#E64A4A`, P2 xanh `#4A90E6`. Khi đứng trên đầu nhau vẫn phân biệt được.
- **Hằng số vật lý (đặt trong `Physics.hpp` dạng `constexpr`, dễ chỉnh):**

| Hằng số | Giá trị |
|---|---|
| `GRAVITY` | 1100 px/s² |
| `MAX_FALL_SPEED` | 420 px/s |
| `RUN_SPEED` | 110 px/s |
| `GROUND_ACCEL` | 900 px/s² |
| `GROUND_FRICTION` | 1000 px/s² (khi không bấm hướng) |
| `AIR_ACCEL` | 600 px/s² |
| `AIR_FRICTION` | 200 px/s² |
| `JUMP_VELOCITY` | −340 px/s (nhảy cao ≈ 3.3 tile) |
| `JUMP_CUT_MULTIPLIER` | 0.45 (nhả phím sớm → vận tốc dọc nhân hệ số này nếu đang bay lên) |
| `COYOTE_TIME` | 0.09 s |
| `JUMP_BUFFER_TIME` | 0.10 s |
| `RESPAWN_DELAY` | 1.0 s |

- **Cảm giác điều khiển (bắt buộc):**
  - *Coyote time:* vẫn nhảy được trong `COYOTE_TIME` sau khi rời mép.
  - *Jump buffering:* bấm nhảy sớm `JUMP_BUFFER_TIME` trước khi chạm đất vẫn tính.
  - *Variable jump height:* nhả phím nhảy khi đang bay lên → cắt vận tốc.
  - *Apex nhẹ:* khi `|vy| < 40` và đang giữ nhảy, giảm trọng lực còn 60% (tuỳ chọn, cho cảm giác mượt).
- **Trạng thái:** `Alive`, `Dead` (chờ hồi sinh), `AtExit` (đã vào cửa thoát, đứng yên và mờ đi một chút).

### 6.5. Va chạm (Physics)

**Tile vs player — AABB tách trục (axis-separated):**
1. Di chuyển theo X: `pos.x += vel.x * dt`; kiểm tra các tile solid giao với hitbox; nếu đè → đẩy ra về cạnh tile gần nhất, `vel.x = 0`.
2. Di chuyển theo Y tương tự; nếu chạm khi đang rơi → `onGround = true`, `vel.y = 0`; nếu đụng trần → `vel.y = 0`.
3. Dùng **sub-step** hoặc giới hạn dịch chuyển mỗi bước < 1 tile để không xuyên tường (với `dt=1/120` và `MAX_FALL_SPEED=420` → 3.5px/bước, an toàn).
4. `onGround` xác định bằng "ground probe": kiểm tra 1px dưới chân có solid / one-way / người chơi khác không.

**One-way platform (`=`):** chỉ chặn khi người chơi đang rơi (`vel.y >= 0`) và chân ở phía trên mặt platform ở bước trước. Cho phép nhảy xuyên từ dưới lên. (Tuỳ chọn: nhấn xuống + nhảy để rơi xuống — **không** cần vì không có phím xuống.)

**Player vs Player (cốt lõi co-op):**
- Hai người chơi **va chạm cứng với nhau theo chiều dọc**: người này có thể **đứng trên đầu** người kia và được mang theo (nếu người dưới di chuyển ngang, người trên được cộng thêm `dx` của người dưới — "carry").
- Theo chiều ngang: họ **đẩy nhẹ nhau** hoặc xuyên qua nhau? → **Chọn: không chặn nhau theo chiều ngang** (đi xuyên qua nhau) để tránh kẹt, chỉ chặn theo chiều dọc khi một người rơi xuống đầu người kia.
- Khi người dưới nhảy, người trên được truyền thêm một phần vận tốc dọc (đạp lên đầu để lên cao hơn): nếu người trên đang đứng trên người dưới và người dưới bắt đầu nhảy cùng lúc, người trên nhận `vel.y = min(vel.y, JUMP_VELOCITY * 0.9)` — đơn giản hóa: coi người dưới như nền, nhảy bình thường từ nền đang bay lên.
- Xử lý thứ tự cập nhật: cập nhật người dưới (y lớn hơn) trước, sau đó người trên.

### 6.6. Đối tượng trong màn

| Đối tượng | Ký hiệu | Hành vi |
|---|---|---|
| Solid tile | `#` | Chặn hoàn toàn. Vẽ gạch tối với viền sáng trên cùng. |
| One-way platform | `=` | Chặn từ trên xuống (mô tả ở 6.5). |
| Spike | `^` | Chạm hitbox thu nhỏ (cao 8px phía dưới tile) → người đó chết. |
| Coin | `o` | Nhặt khi hitbox chạm; +100 điểm chung; hiệu ứng particle; biến mất. |
| Checkpoint | `C` | Cờ; khi **bất kỳ** người nào chạm, cả hai dùng điểm này khi hồi sinh; cờ đổi màu. |
| Exit | `E` | Cửa thoát (rộng 1 tile). Người chạm → trạng thái `AtExit`. Cả hai `AtExit` → thắng màn. Nếu một người rời cửa (không đứng trong vùng) → trở lại `Alive`. |
| Button | `a`,`b`,`c` (chữ thường) | Nút sàn; **đang được giữ** khi có người đứng lên nó. Nhả ra → cửa đóng lại (có thể delay 0.2s). |
| Door | `A`,`B`,`C`… (chữ hoa) | Cửa chặn như solid; **mở** khi nút cùng chữ cái (`A` ↔ `a`) đang được giữ. Khi mở: không chặn, vẽ mờ. Nếu cửa đóng lại lúc có người đứng trong → đẩy người đó ra hướng gần nhất hoặc cho phép (chọn đơn giản: không đóng khi có người bên trong). |
| Spawn P1 / P2 | `P` / `Q` | Vị trí xuất phát (tile trống). |

> Lưu ý xung đột ký hiệu: chữ `C` (checkpoint) ≠ cửa `C`. **Quy ước:** ký hiệu cửa/nút chỉ dùng `A–B` / `a–b` (tối đa 2 cặp mỗi màn). Checkpoint dùng `C`, coin dùng `o`, exit dùng `E`, spawn dùng `P`, `Q`.

### 6.7. Hồi sinh & chết

- Chết do: chạm gai, rơi xuống dưới đáy map (`y > mapHeight + 64`), bị "ép" (đè giữa solid và người kia — tuỳ chọn, bỏ qua nếu phức tạp).
- Khi chết: particle nổ, `state = Dead`, đếm `RESPAWN_DELAY`, sau đó xuất hiện tại checkpoint đã kích hoạt gần nhất (mặc định: vị trí spawn của chính người đó). Nếu spawn trùng chỗ người kia, dịch ngang 12px.
- Có bộ đếm "Deaths" chung hiển thị ở HUD.

### 6.8. Camera

- Camera nhìn theo **trung điểm** của các người chơi còn sống (nếu cả hai chết → giữ nguyên).
- Làm mượt: `cam += (target - cam) * (1 - exp(-8 * dt))`.
- Giới hạn camera trong biên map. Nếu map nhỏ hơn màn hình → căn giữa.
- **Giữ cả hai trong tầm nhìn:** nếu một người sắp ra khỏi màn hình (cách mép < 16px), chặn người đó không đi tiếp ra ngoài về phía sau (kẹp vị trí vào vùng camera theo trục X). Không kẹp theo trục Y (cho phép rơi để chết).
- Map thiết kế sao cho khoảng cách tối đa giữa hai người không vượt chiều rộng màn hình khi cần phối hợp.

### 6.9. HUD & UI

- Góc trên trái: `P1`, góc trên phải: `P2` (màu tương ứng), kèm biểu tượng trạng thái (sống/chết/đã vào cửa).
- Giữa trên: tên màn, điểm (`Score`), số lần chết (`Deaths`), thời gian màn (`mm:ss.t`).
- Menu chính: `Start`, `Select Level`, `Controls`, `Quit`. Điều hướng bằng cả hai bàn phím + gamepad.
- Màn hình **Pause:** `Resume`, `Restart`, `Menu`.
- Màn hình **Level Complete:** hiển thị thời gian, điểm, số lần chết; bấm `Enter`/`A` để sang màn kế; sau màn cuối hiển thị "Chúc mừng" và về menu.
- Chữ dùng `SDL_RenderDebugText`; scale 1× hoặc 2× tuỳ vị trí. Có hàm helper `drawTextCentered`.
- Overlay debug (`F3`): FPS, số tile solid xung quanh, hitbox, vector vận tốc.

### 6.10. Đồ họa procedural & hiệu ứng

- **Palette** (đặt trong `Palette.hpp`):
  - Nền: gradient dọc `#1B1F3B` → `#3A2E5C` (vẽ bằng nhiều dải rect).
  - Parallax: 2 lớp đồi/ngôi sao vẽ bằng rect/ điểm ngẫu nhiên cố định (seed cố định), cuộn chậm hơn camera (hệ số 0.2, 0.5).
  - Solid: `#2E3350`, viền sáng `#5B6396`.
  - Gai: `#D9D9E3`; coin: `#FFD54A`; checkpoint: xám → xanh lá khi kích hoạt; exit: tím `#B07CFF`; cửa/nút: cặp màu theo chữ (A = cam `#FF9F43`, B = lục lam `#2ED3B7`).
- **Particles:** nổ khi chết, lấp lánh khi nhặt coin, bụi khi đáp xuống. Pool cố định (ví dụ 512 hạt), mỗi hạt: vị trí, vận tốc, tuổi, màu, kích thước.
- **Squash & stretch nhẹ** khi nhảy/đáp (tuỳ chọn, chỉ ảnh hưởng render, không đổi hitbox).
- **Screen shake** ngắn khi có người chết (biên độ 2px, 0.15s).

### 6.11. Âm thanh (tuỳ chọn — làm ở milestone cuối)

- Dùng `SDL_OpenAudioDeviceStream` với `SDL_AudioSpec` mono/float32/44100 (hoặc tương đương SDL3 hiện hành).
- Sinh âm procedural: `jump` (sóng vuông đi lên), `coin` (hai nốt), `death` (nhiễu giảm dần), `door` (trầm), `win` (arpeggio).
- Nếu khởi tạo audio thất bại → game vẫn chạy, chỉ log cảnh báo.
- Phím `M` bật/tắt tiếng.

---

## 7. Định dạng file level

File văn bản UTF-8 trong `assets/maps/`. Cấu trúc:

```
name: Tên màn hiển thị
par: 45          # (tuỳ chọn) thời gian tham khảo, giây
--
<các dòng bản đồ, mỗi ký tự = 1 tile 16x16>
```

- Phần trước dòng `--` là metadata `key: value`. Dòng bắt đầu bằng `#` **trong phần metadata** là chú thích (khác với ký tự `#` trong bản đồ vì bản đồ nằm sau `--`).
- Các dòng bản đồ có thể có độ dài khác nhau → pad bằng `.` (trống) bên phải đến chiều rộng lớn nhất.
- **Ngoài biên map = solid** ở trái/phải/trên; **đáy = hố chết** (rơi xuống là chết).
- Validator khi load phải kiểm tra và báo lỗi rõ (kèm số dòng) nếu:
  - thiếu `P` hoặc `Q` (hoặc trùng lặp),
  - thiếu `E`,
  - có cửa không có nút cùng cặp (và ngược lại),
  - ký tự không hợp lệ.
- Danh sách màn lấy từ việc quét thư mục `assets/maps/` và **sắp xếp theo tên file** (tiền tố số `01_`, `02_`…). Dùng `SDL_EnumerateDirectory` hoặc `std::filesystem`.

### Ví dụ `levels/01_first_steps.txt`

```
name: First Steps
par: 30
--
..............................
..............................
..............................
..............................
..............................
.......o.........o............
......###.......###...........
..............................
..................o...........
...P..Q...........=====....E..
##############.....##########.
##############^^^^^##########.
```

> Codex phải tạo **ít nhất 4 màn** hoàn chỉnh, mỗi màn phải **giải được**:
> 1. `01_first_steps` — chạy, nhảy, nhặt coin, một hố nhỏ, thoát cùng nhau.
> 2. `02_buttons` — một cặp nút/cửa `a`/`A`: một người giữ nút, người kia qua cửa; sau đó đổi vai trò (cửa thứ hai ở phía ngược lại).
> 3. `03_tower` — bức tường cao hơn 1 lần nhảy: bắt buộc đứng lên đầu nhau; có one-way platform và checkpoint.
> 4. `04_gauntlet` — kết hợp gai, hai cặp nút/cửa `a/A`, `b/B`, platform one-way, coin bonus ở chỗ khó.
>
> Sau khi tạo, viết một test (hoặc script) **kiểm tra tính hợp lệ** (validator pass) cho mọi file trong `assets/maps/`.

---

## 8. Kiến trúc code

### 8.1. State machine

```cpp
struct State {
    virtual ~State() = default;
    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void handleEvent(const SDL_Event&) {}
    virtual void update(float dt) = 0;     // fixed dt
    virtual void render(Renderer&) = 0;
};
```

`App` giữ một stack hoặc con trỏ state hiện tại; chuyển state bằng yêu cầu hoãn (`requestState(...)`) để không hủy state đang chạy giữa vòng lặp.

### 8.2. Input

- Lớp `Input` mỗi frame: đọc `SDL_GetKeyboardState` + gamepad → điền `PlayerInput inputs[2]`.
- Tính cạnh (`pressed`/`released`) bằng cách so với frame trước.
- Vì update chạy ở fixed timestep (có thể nhiều lần/khung hoặc 0 lần), **cạnh nhấn phải được "giữ" cho tới khi được tiêu thụ** bởi một lần update (tránh mất phím nhảy). Gợi ý: cờ `jumpPressedLatched`, reset sau khi `update` xử lý.

### 8.3. Dữ liệu cốt lõi (gợi ý, được phép điều chỉnh)

```cpp
struct Vec2 { float x{}, y{}; };
struct Rect { float x{}, y{}, w{}, h{}; };      // AABB

enum class Tile : uint8_t { Empty, Solid, OneWay, Spike };

struct Level {
    int width = 0, height = 0;                  // theo tile
    std::vector<Tile> tiles;
    std::vector<Coin> coins;
    std::vector<Button> buttons;
    std::vector<Door> doors;
    std::vector<Checkpoint> checkpoints;
    Rect exit;
    Vec2 spawn[2];
    std::string name;
    float parTime = 0;
};

struct Player {
    Vec2 pos, vel;                              // pos = góc trên-trái hitbox
    bool onGround = false;
    float coyoteTimer = 0, jumpBufferTimer = 0, respawnTimer = 0;
    int facing = 1;
    enum class Status { Alive, Dead, AtExit } status = Status::Alive;
};
```

- `Physics::step(Player&, const PlayerInput&, const Level&, const Player& other, float dt)` là **hàm thuần** (không gọi SDL) để test được.
- Giao diện truy vấn tile: `bool Level::isSolidAt(int tx, int ty)` (đã xét cửa đóng), `Tile Level::tileAt(...)`.

### 8.4. Quy ước code

- Tổ chức file: một class/module chính mỗi cặp `include/twinleap/<module>/X.hpp` + `src/<module>/X.cpp`; namespace `twinleap` (có thể lồng theo module, ví dụ `twinleap::game`).
- Tên: `PascalCase` cho type, `camelCase` cho hàm/biến, `kConstantName` hoặc `UPPER_SNAKE` cho hằng (chọn một và nhất quán).
- Không dùng `using namespace std;` trong header.
- Dùng `std::array`/`std::vector`, tránh `new/delete` trần; dùng `std::unique_ptr` khi cần sở hữu.
- Hàm dài > 60 dòng nên tách.
- Bình luận ngắn gọn, giải thích *tại sao*, không giải thích *cái gì*.

---

## 9. Kiểm thử

Trong `tests/` (include header qua `#include "twinleap/..."`, link với `twinleap_core`):

1. **test_physics** (không cửa sổ, không SDL video):
   - Rơi xuống nền → `onGround == true`, `vel.y == 0`.
   - Nhảy đạt độ cao trong khoảng `[48, 56]` px khi giữ phím đủ lâu; nhả sớm → thấp hơn rõ rệt.
   - Coyote time: rời mép 0.05s vẫn nhảy được; 0.2s thì không.
   - Jump buffer: bấm nhảy 0.05s trước khi chạm đất → nhảy ngay khi chạm.
   - Không xuyên tường ở tốc độ rơi tối đa.
   - One-way: nhảy từ dưới lên xuyên qua; rơi từ trên xuống thì đứng được.
   - Đứng trên đầu người kia: `onGround == true`, được "carry" khi người dưới di chuyển.
2. **test_level_parser:**
   - Parse level hợp lệ → số tile/đối tượng đúng.
   - Level thiếu `P`/`Q`/`E` → lỗi có thông điệp rõ.
   - Cửa không có nút → lỗi.
   - Tất cả file trong `assets/maps/` đều parse thành công.
3. **test_buttons_doors:** đứng lên nút → cửa mở; rời → cửa đóng.

Kiểm thử thủ công (ghi checklist trong README): chạy được 4 màn, qua màn bằng cách phối hợp, chết/hồi sinh, pause/resume, fullscreen, hot-plug gamepad.

---

## 10. Hiệu năng & độ bền

- Mục tiêu 60 FPS ổn định (vsync) trên máy yếu; không cấp phát động trong vòng lặp update/render (dùng pool, `reserve`).
- Không crash khi: thiếu thư mục `assets/maps/`, file level hỏng, không có gamepad, không có thiết bị âm thanh, cửa sổ bị minimize (khi minimize, tạm dừng accumulator hoặc clamp dt).
- Thoát sạch: không rò rỉ tài nguyên (kiểm tra bằng `valgrind`/ASan nếu có).
- Cung cấp preset CMake sanitizer (tuỳ chọn): `-DTWINLEAP_SANITIZE=ON` bật `-fsanitize=address,undefined` trên GCC/Clang.

---

## 11. Tiêu chí hoàn thành (Definition of Done)

- [ ] `cmake -S . -B build && cmake --build build` thành công trên Linux, không có warning nghiêm trọng.
- [ ] `ctest` pass toàn bộ.
- [ ] Chạy `./build/twinleap` mở cửa sổ, hiển thị menu, chọn màn và chơi được.
- [ ] 2 người chơi điều khiển độc lập trên cùng bàn phím; gamepad hoạt động nếu có.
- [ ] Cảm giác nhảy có coyote time, jump buffer, variable jump.
- [ ] Đứng lên đầu nhau hoạt động; nút/cửa hoạt động; checkpoint & hồi sinh hoạt động.
- [ ] Cả hai vào Exit → hiện màn hình hoàn thành → sang màn tiếp theo.
- [ ] 4 màn chơi được và giải được.
- [ ] Pause, restart, fullscreen, debug overlay hoạt động.
- [ ] `README.md` có: mô tả, ảnh/mô tả điều khiển, hướng dẫn build 3 hệ điều hành, cách thêm level mới, Known Issues.

---

## 12. Kế hoạch thực hiện theo Milestone

Thực hiện **tuần tự**; hoàn tất và build được từng milestone trước khi sang milestone kế tiếp.

**M0 — Khung dự án**
- Tạo cấu trúc thư mục, `CMakeLists.txt` (tìm/fetch SDL3), `main.cpp` mở cửa sổ + renderer, vòng lặp sự kiện, tô nền, thoát bằng `Esc`/đóng cửa sổ.
- Fixed timestep + hiển thị FPS bằng `SDL_RenderDebugText`.
- ✅ *Kiểm tra:* chạy được, cửa sổ hiện, thoát sạch.

**M1 — Tilemap & render cơ bản**
- `Level` parser + validator, vẽ tile solid/one-way/spike từ file `01_first_steps.txt`.
- `Camera` cơ bản (cố định hoặc theo điểm). Logical presentation 480×270 letterbox.
- ✅ *Kiểm tra:* thấy map đúng với file; test parser pass.

**M2 — Một người chơi + vật lý**
- `Player`, `Physics::step`, nhập liệu bàn phím P1, va chạm tile, coyote/buffer/variable jump.
- ✅ *Kiểm tra:* test_physics pass; chạy/nhảy cảm giác tốt trên màn 1.

**M3 — Hai người chơi**
- Thêm P2, input P2, va chạm dọc giữa 2 người (đứng lên đầu, carry), camera theo trung điểm, kẹp người chơi trong khung nhìn.
- ✅ *Kiểm tra:* hai người điều khiển độc lập, đứng chồng lên nhau được.

**M4 — Đối tượng màn chơi**
- Coin, spike + chết/hồi sinh, checkpoint, exit + điều kiện thắng, nút/cửa.
- HUD cơ bản (điểm, chết, thời gian).
- ✅ *Kiểm tra:* màn 1 và 2 chơi được từ đầu tới cuối.

**M5 — State machine & UI**
- Menu chính, chọn màn, pause, màn hình hoàn thành, chuyển màn, danh sách level tự quét.
- ✅ *Kiểm tra:* đi từ menu → chơi → thắng → màn kế → menu.

**M6 — Nội dung & hoàn thiện cảm giác**
- Hoàn thiện màn 3 và 4; chỉnh hằng số vật lý cho dễ chơi; particles, parallax nền, screen shake, squash & stretch.
- Gamepad + hot-plug.
- ✅ *Kiểm tra:* chơi hết 4 màn.

**M7 — Âm thanh (tuỳ chọn), kiểm thử, tài liệu**
- Sfx procedural, test bổ sung, README đầy đủ, dọn warning, thử sanitizer.
- ✅ *Kiểm tra:* toàn bộ Definition of Done (mục 11).

---

## 13. Ý tưởng mở rộng (chỉ làm sau khi xong M7)

- Platform di chuyển (moving platform) chở người chơi.
- Khối đẩy được (pushable crate) cần cả hai người đẩy.
- Chế độ 1 người chơi điều khiển luân phiên 2 nhân vật (phím `Tab` chuyển).
- Lưu thời gian tốt nhất mỗi màn vào file `save.txt` (dùng `SDL_GetPrefPath`).
- Trình soạn level đơn giản bằng chuột.
- Đua thời gian / chế độ đối kháng (ai nhặt nhiều coin hơn).

---

## 14. Gợi ý prompt để chạy Codex CLI

```text
Đọc AGENTS.md ở thư mục gốc. Thực hiện tuần tự từ M0 đến M7.
Sau mỗi milestone: build, chạy ctest, tóm tắt những gì đã làm và những gì còn lại,
rồi tiếp tục milestone kế tiếp mà không cần hỏi lại, trừ khi bị chặn bởi lỗi không tự sửa được.
Tuân thủ mục 5 (API SDL3) và tra header SDL3 nếu không chắc về tên hàm.
```
