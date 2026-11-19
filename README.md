# Twinleap

Game platformer co-op hai người trên cùng máy, viết bằng C++17 và SDL3. Cùng vượt hố, đứng lên đầu nhau, giữ nút mở cửa và đến cửa tím để hoàn thành màn. **P1 Gấu mèo** từ `RACCOONSPRITESHEET.png`, **P2 Chim xanh** từ `BIRDSPRITESHEET_Blue.gif`; nền và địa hình pixel art từ Pixel Adventure 1. Âm thanh dùng WAV (SFX và nhạc nền); xem [hướng dẫn thay âm thanh](docs/AUDIO.md).

Windows x64 là nền tảng chính. Linux/macOS được giữ làm nền tảng hỗ trợ bổ sung.

## Chạy trên Windows

Nếu đã tạo gói bằng `build-windows.cmd -Package`, giải nén `build-windows/package/Twinleap-Windows-x64.zip`, mở thư mục bên trong và nhấp đúp `twinleap.exe`. Gói có SDL3, runtime MSVC, hình ảnh và năm màn chơi; giữ các DLL và thư mục `assets` đi kèm. ZIP là sản phẩm build, không lưu sẵn trong mã nguồn.

Trong thư mục mã nguồn, bản mới nhất nằm ở `build-windows/twinleap.exe`:

```powershell
.\build-windows\twinleap.exe
```

Game mở trực tiếp cửa sổ, không bật console. Có thể bật console khi phát triển bằng `-DTWINLEAP_WINDOWS_GUI=OFF`.

## Build Windows tự động (khuyên dùng)

Cài Visual Studio hoặc Build Tools với **Desktop development with C++**, CMake >=3.21 và Ninja (có trong bộ công cụ CMake của Visual Studio). Nhấp đúp `build-windows.cmd` để build và chạy test. Script tự tìm MSVC/Ninja; không cần mở Developer Command Prompt hay sửa PATH hệ thống.

Trong PowerShell tại thư mục dự án:

```powershell
.\build-windows.cmd
.\build-windows.cmd -Run
.\build-windows.cmd -Package
```

`-Run` mở game sau khi test đạt. `-Package` tạo `build-windows/package/Twinleap-Windows-x64.zip`; có thể dùng cả hai tùy chọn cùng lúc.

Script tìm SDL3 trong `.deps/SDL3-*/cmake`. Nếu cài SDL3 ở chỗ khác:

```powershell
.\build-windows.cmd -SDL3Dir "C:\libs\SDL3\cmake" -Package
```

Nếu không có SDL3 cục bộ, CMake tìm bản cài trên máy rồi tải tự động khi cần; bước tải cần Git và kết nối GitHub.

## Chạy trong Visual Studio

1. Chọn **File > Open > Folder**, mở thư mục chứa `CMakeLists.txt`.
2. Chọn preset **Twinleap - Visual Studio x64 Debug** (`vs-x64-debug`) và đợi CMake cấu hình xong. Nếu chưa thấy preset, đóng/mở lại thư mục; kiểm tra **Tools > Options > CMake > General** đã bật dùng CMakePresets.json.
3. Trong **Startup Item**, chọn **twinleap.exe**, rồi bấm **Ctrl+F5** để chạy hoặc **F5** để debug. Không chọn các chương trình `test_*` làm startup.

Preset dùng SDL3 có sẵn tại `.deps/SDL3-3.4.18/cmake`, không tải lại từ mạng,
và tạo build riêng trong `out/build/vs-x64-debug`. Nếu dùng máy khác, chuẩn bị
SDL3 tại vị trí đó hoặc sửa `SDL3_DIR` trong preset cho phù hợp.
Asset và DLL SDL3 được copy cạnh executable khi build target `twinleap`.
Thư mục build của preset được tạo lại khi cấu hình. Cần workload **Desktop development with C++**
và thành phần **C++ CMake tools for Windows**.

## Điều khiển

| Hành động | P1 | P2 |
|---|---|---|
| Đi trái/phải | A / D | Trái / Phải |
| Nhảy | W hoặc Space | Lên hoặc Right Ctrl |

Gamepad đầu tiên điều khiển P1, thứ hai điều khiển P2: stick trái/D-pad di chuyển, A/Cross nhảy, Start pause. Bàn phím vẫn hoạt động cùng gamepad. Trong menu: W/S hoặc Lên/Xuống, Enter/A xác nhận. Esc pause/trở lại, R chơi lại màn, F11 fullscreen, F3 debug, M bật/tắt âm thanh.

Điểm, thời gian và deaths tính riêng mỗi màn; restart xóa các thống kê này. Checkpoint được chia sẻ. Người chết hồi sinh sau một giây. Cả hai phải vào cửa thoát. Nhả nút nhảy sớm để nhảy thấp hơn. Khi hai người cách xa nhau (kể cả sau hồi sinh), màn hình tự chia đôi: P1 bên trái, P2 bên phải, mỗi bên có camera riêng. Khi lại gần, game tự trở về camera chung.

## Build thủ công và nền tảng bổ sung

Cần CMake >=3.21, trình biên dịch C++17 và SDL3 >=3.2. CMake tìm SDL3 đã cài, nếu thiếu sẽ tải tag `release-3.2.0` từ GitHub. Chỉ có SDL3 là dependency ngoài thư viện chuẩn.

Linux/macOS:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/twinleap
ctest --test-dir build --output-on-failure
```

Debian/Ubuntu khi build SDL3 từ source:

```sh
sudo apt install build-essential cmake pkg-config libx11-dev libxext-dev libxrandr-dev libxcursor-dev libxi-dev libxss-dev libwayland-dev libxkbcommon-dev libasound2-dev libpulse-dev libudev-dev libdrm-dev libgbm-dev libgl1-mesa-dev
```

macOS: cài Xcode Command Line Tools (`xcode-select --install`) và CMake, sau đó dùng lệnh trên.

Windows: dùng Developer Command Prompt của Visual Studio với workload Desktop development with C++:

```bat
cmake -S . -B build
cmake --build build --config Release -j
build\Release\twinleap.exe
ctest --test-dir build -C Release --output-on-failure
```

Với Ninja, executable nằm ngay trong thư mục build. Script Windows phía trên tự tìm MSVC và Ninja; bản build chính nằm ở `build-windows/`. `.deps/SDL3-3.4.18` là SDK SDL3 cục bộ để không phải tải lại. Không đóng gói `.deps` vào source; có thể truyền `-DSDL3_DIR=<đường dẫn SDL3/cmake>` cho bản cài khác. Tắt download bằng `-DTWINLEAP_FETCH_SDL3=OFF`, tắt test bằng `-DTWINLEAP_BUILD_TESTS=OFF`. GCC/Clang hỗ trợ `-DTWINLEAP_SANITIZE=ON`.

## Năm màn

1. Meadow Steps: nền xanh lam, đất cỏ xanh; làm quen chạy/nhảy, hố ba tile và đường platform lấy xu thưởng.
2. Amber Passage: nền xanh lá, đất cam; người thứ nhất giữ nút cam, người thứ hai qua cửa rồi giữ nút bên kia để đổi vai. Có platform và xu thưởng sau cửa.
3. Lavender Summit: nền tím, đất cỏ hồng; dùng đầu đồng đội để lên bức tường bốn tile. Người trên giữ nút mở lối dưới cho người còn lại, sau đó cùng xuống bên phải. Có checkpoint trên đỉnh và xu phía trên platform.
4. Ironworks Relay: nền nâu và địa hình kim loại; hai cặp cửa cam/xanh, gai, one-way, checkpoint và đường lấy xu thưởng sau cửa cuối.

5. Last Ascent: màn dài nhất (192 × 18 tile, dài gấp 3,84 lần màn 4), thời gian tham khảo 300 giây. Ba cửa tiếp sức với nút ở hai phía, hai chuỗi bệ hẹp trên gai, hố rộng có bệ giữa và tháp bốn tile cần đứng lên đầu đồng đội. Bốn checkpoint chia sẻ giúp luyện từng chặng; xu thưởng nằm trên đường nhảy khó. Chọn dòng thứ năm trong Select Level.

## Hình ảnh và asset

P1/P2 có animation đứng (4 frame), chạy (8 frame), tư thế nhảy và rơi lấy từ các frame có sẵn; tự quay theo hướng đi khi chạy/ở trên không. Số 1/2 phía trên đầu giúp phân biệt khi đứng chồng nhau. Hitbox vẫn là 12×14 pixel; khung hình sprite có vùng trong suốt và không thay đổi va chạm.

Game nạp BMP trong `assets/` một lần lúc khởi động, dùng color key hồng tím cho nền trong suốt và nearest filtering để giữ nét pixel. Không thêm SDL_image. Chỉ giữ các file đang dùng, chia theo chức năng:

```text
assets/
├── characters/
│   ├── raccoon/     # P1: idle, run, jump, fall (.bmp)
│   └── bird/        # P2: idle, run, jump, fall (.bmp)
├── backgrounds/     # 4 nền (.bmp)
├── terrain/         # atlas địa hình (.bmp)
├── traps/           # gai (.bmp)
├── maps/            # 5 màn (.txt)
└── audio/
    ├── sfx/         # 4 WAV Kenney + coin do người dùng cung cấp
    └── music/       # background.wav phát lặp
```

Đã bỏ ZIP, toàn bộ PNG nguồn dư và script chuyển đổi không còn dùng. BMP là asset chính để chỉnh sửa/thay thế. Xem [quy ước asset](assets/README.md) và [nguồn hình ảnh](assets/CREDITS.md). Build tự copy cả `assets/` kể cả khi chỉ sửa dữ liệu; ZIP có cùng cấu trúc. Thiếu/hỏng asset sẽ báo lỗi và thoát sạch.

## Tự thêm âm thanh

Đã chọn năm hiệu ứng Kenney cho nhảy, xu, chết, cửa và hoàn thành màn trong `assets/audio/sfx/`, chuyển sang WAV PCM mono 44100 Hz, 16-bit và giữ giấy phép CC0. ZIP nguồn cùng các âm dư đã bỏ. Xem [danh sách âm](assets/audio/sfx/README.md). Game nạp WAV khi khởi động bằng SDL3. Coin thay bằng tiếng đầu tiên từ getcoin.mp3; nhạc nền từ bgsong.mp3 phát lặp ở menu và khi chơi. M bật/tắt cả nhạc và SFX; pause/minimize tạm dừng âm thanh. Xem [docs/AUDIO.md](docs/AUDIO.md) để thay âm thanh; không cần sửa code nếu giữ tên file.

## Tài liệu kiến trúc

Xem [tài liệu riêng cho từng file .cpp](docs/source/README.md): mỗi file mã game và test có một trang giải thích hàm, dữ liệu, ví dụ và cách debug.

Đọc [Học mã nguồn Twinleap](docs/PROJECT_OVERVIEW.md): lộ trình đọc code, giải thích cả 14 file `.cpp`, dữ liệu đầu vào/đầu ra, luồng nhảy và bài tập nhỏ. Bắt đầu với Player → Level → Physics → PlayState, sau đó đọc vòng lặp App và các phần hiển thị/âm thanh.

`src/`, `include/`, `tests/`, `assets/` là các phần cần học và sửa. `build-windows/` là kết quả build chính; `out/build/` được Visual Studio tạo khi dùng preset; `.vs/` là trạng thái/cache IDE. Xem [DECISIONS.md](docs/DECISIONS.md) để biết cấu trúc hiện tại và những phần đã dọn.

## Thêm màn

Tạo file `.txt` trong `assets/maps/`, đặt tiền tố số để sắp thứ tự:

```text
name: My Level
par: 45
--
....................
..P.Q............E..
####################
```

`#` solid, `=` one-way, `^` gai, `o` xu, `C` checkpoint, `E` exit, `P/Q` spawn, `a/A` và `b/B` nút/cửa. Có thể có nhiều nút/cửa cùng nhóm; bất kỳ nút nào được giữ đều mở cả nhóm. Dòng ngắn được pad bằng dấu chấm. Không dùng C làm cửa. Đặt nút ngay trên nền. Parser kiểm tra spawn, exit, cặp nút/cửa và ký tự; tính giải được của màn cần kiểm tra riêng. Chạy lại build để copy màn cạnh executable.

## Kiểm thử

CTest gồm 5 test: parser với dữ liệu sai và cả năm màn, vật lý (nhảy, coyote, buffer, one-way, carry), nút/cửa, gameplay (exit, chết/hồi sinh, checkpoint, audio dummy), và render asset bằng software renderer không cần cửa sổ. Test render kiểm tra bốn theme, trạng thái nhân vật, lật hướng và độ mờ, đồng thời xuất `render-preview-1.bmp` đến `render-preview-4.bmp` trong thư mục build. `twinleap --smoke` tự mở màn đầu rồi thoát sau 31 frame.

Checklist kiểm tra thủ công:

- [ ] Hai người chơi hết cả năm màn và đổi vai giữ nút.
- [ ] Kiểm tra menu, chọn màn, pause/resume, restart và chuyển màn cuối.
- [ ] Nghe các âm jump/coin/death/door/win; thử M và pause.
- [ ] Cắm/rút hai gamepad, kiểm tra bàn phím đồng thời.
- [ ] Resize, fullscreen, debug, minimize và đóng cửa sổ.

## Known Issues

- Build/CTest và smoke test đã chạy trên Windows; Linux/macOS, sanitizer và phần cứng gamepad chưa kiểm chứng.
- Kiểm tra audio bằng dummy driver không chứng minh chất lượng nghe trên loa thực tế.
- Chưa hoàn thành checklist chơi thử hai người qua cả năm màn; validator chỉ xác nhận định dạng màn hợp lệ.
- Font debug chỉ hiển thị ASCII; dùng tên màn tiếng Anh.
- Không có lưu tiến độ hoặc đổi phím. Bàn phím có giới hạn ghosting có thể cần gamepad.
