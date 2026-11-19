# App.cpp — khởi tạo và vòng lặp chính

[Mã nguồn](../../../src/core/App.cpp) · [Header](../../../include/twinleap/core/App.hpp) · [Mục lục](../README.md)

## Tra cứu các hàm của file

```cpp
void showStartupError(const char *context, SDL_Window *window = nullptr);
int App::run(int argc, char **argv);
```

| Hàm | Tham số | Kết quả / tác động / nơi gọi |
|---|---|---|
| showStartupError | context là chuỗi ngữ cảnh; window là cửa sổ cha, có thể nullptr | void; ghi log và mở hộp thoại; chỉ run gọi khi startup/render lỗi |
| App::run | argc/argv là tham số từ main | int 0 thành công, 1 lỗi; tạo tài nguyên, chạy game, giải phóng; main gọi một lần |
| Lambda load trong run | Không tham số; bắt files/current/game/error/requested/input bằng tham chiếu | void; nạp màn đang chọn, đặt requested=Play/Error và consume input; các nhánh Start/Restart/Next gọi |

### Các hàm SDL theo vòng đời

| Hàm được dùng | Giá trị trả về và việc code sử dụng |
|---|---|
| `SDL_Init(flags)` | bool; true mới tiếp tục tạo cửa sổ |
| `SDL_CreateWindow(title,w,h,flags)` | SDL_Window* hoặc nullptr; tạo cửa sổ resizable |
| `SDL_CreateRenderer(window,nullptr)` | SDL_Renderer* hoặc nullptr; lấy renderer mặc định |
| `SDL_SetRenderVSync(renderer,1)` | bool; bật vsync, lỗi chuyển vào r.check |
| `SDL_SetRenderLogicalPresentation(...)` | bool; đặt vùng logic 480×270 letterbox |
| `SDL_GetBasePath()` | const char* do SDL giữ, có thể nullptr; dùng tìm assets/maps cạnh exe, không free |
| `SDL_PollEvent(&event)` | bool; true đồng thời ghi sự kiện vào event |
| `SDL_SetWindowFullscreen(window,flag)` | bool; đổi fullscreen, lỗi chuyển vào r.check |
| `SDL_GetTicksNS()` | Số nanosecond; lấy chênh lệch hai lần gọi để tính elapsed giây |
| `SDL_GetWindowFlags(window)` | Bitmask; dùng & SDL_WINDOW_MINIMIZED để nhận biết minimize |
| `SDL_Delay(16)` | void; nghỉ ngắn khi minimize, tránh vòng lặp chạy hết CPU |
| `SDL_RenderPresent(renderer)` | bool; đưa kết quả vẽ ra màn hình, lỗi qua r.check |
| `SDL_ShowSimpleMessageBox(...)` | bool; hiển thị lỗi, nếu false thì log thêm |
| `SDL_GetError()` / `SDL_Log(...)` | Chuỗi lỗi hiện tại / ghi log |
| `SDL_DestroyRenderer`, `SDL_DestroyWindow`, `SDL_Quit` | void; giải phóng đúng thứ tự sau khi các wrapper hết scope |

### Hàm project và STL gọi từ run

| Nhóm | Các lời gọi và ý nghĩa |
|---|---|
| Input | event(e) nhận từng sự kiện; poll() đọc thiết bị; consume() xóa cạnh đã dùng |
| Gameplay | game.load(path,error) trả bool; update(inputs,dt,sfx) đổi logic; render(r,debug) vẽ |
| Time | add(elapsed) cộng accumulator; ready() trả bool; consume() trừ một bước; reset() về 0 |
| Audio | toggle() mute; silence() xóa SFX/queue; setPaused(bool) dừng thiết bị; update() cấp queue |
| UI/render | r.check(bool) giữ lỗi; background/centered/text vẽ; MenuState::render chọn UI |
| Filesystem | is_directory(path,ec) trả bool; directory_iterator duyệt; increment(ec) bước tiếp mà báo lỗi qua error_code |
| Path | extension() lấy .txt; stem() lấy tên không đuôi; string() đổi path thành string |
| Vector | emplace_back/push_back thêm phần tử; empty/size kiểm tra số phần tử |
| Thuật toán | sort(begin,end) sắp tên; min giới hạn elapsed; copy_n chép từng đoạn thông báo lỗi |
| Chuỗi | c_str() lấy chuỗi C cho SDL; snprintf ghi văn bản có giới hạn vào buffer |

### Ví dụ lần theo một lệnh

Nhấn Enter ở Start → input.confirm=true → lambda load gọi game.load
→ requested=Play → screen=requested → game.render. Từ frame gameplay sau,
clock.add/ready điều khiển số lần game.update. Muốn sửa nhảy phải đi tiếp
vào PlayState/Physics, không sửa SDL_PollEvent trong App.

## Vai trò

App nối SDL với các module, quản lý màn hình hiện tại và vòng đời tài nguyên.
main gọi run; run giữ Input, Renderer, Sfx và PlayState trong suốt phiên chạy.

## showStartupError(context, window)

Hàm nội bộ ghép context với SDL_GetError, ghi log và mở hộp thoại lỗi.
window có thể nullptr khi chưa tạo được cửa sổ. Hộp thoại lỗi thất bại
thì ghi thêm log. Anonymous namespace giới hạn helper trong file này.

## run(argc, argv): khởi tạo

Kiểm tra --smoke, khởi tạo video/gamepad, tạo cửa sổ 1280 × 720 và renderer.
Nếu không tạo được renderer thì hủy window nếu có, SDL_Quit và trả 1.
Trong scope tài nguyên, nạp ảnh, bật vsync và đặt logical presentation 480 × 270.
Renderer::check giữ cờ failed nếu các bước render thất bại.

Map được tìm tại assets/maps ở working directory, cạnh executable, rồi ở
thư mục cha executable. Dùng thư mục đầu tiên có TXT; sắp xếp tên file.
displayNames lấy tên file không phần mở rộng để vẽ Select Level.
Không có map thì screen=Error và lưu thông báo.

## Các biến điều khiển cần hiểu

| Biến | Ý nghĩa |
|---|---|
| screen | Màn hình đang xử lý |
| requested | Màn hình yêu cầu chuyển tới ở cuối phần update |
| choice | Dòng menu chọn, tính từ 0 |
| current | Chỉ số màn trong files |
| running | Còn chạy vòng lặp |
| clock | Time accumulator cho fixed timestep |
| previous/now | Mốc nanosecond để tính elapsed |
| fps | FPS được làm mượt cho overlay |

Lambda load bắt biến ngoài bằng tham chiếu `[&]`: nạp files[current], đặt
requested=Play/Error và consume input. Đó là hàm cục bộ dùng lại cho Start,
Select, Restart và Next, không phải lớp hay thread riêng.

## Một frame chạy thế nào?

1. Poll SDL_Event: chuyển cho Input, xử lý quit, F11/F3/M/R.
2. Input::poll đọc trạng thái hiện tại. Tính elapsed và giới hạn 0,25 giây.
3. Minimize: reset clock, xóa SFX chờ, pause audio, delay 16ms rồi bỏ frame.
4. Xử lý pause/back. Các yêu cầu màn hình lưu vào requested.
5. Ngoài Play: điều hướng choice, xử lý confirm, consume input và reset clock.
6. Trong Play: cộng elapsed, lặp update 1/120 giây khi clock.ready.
   Sau mỗi update consume input và trừ một bước clock. Complete thì yêu cầu đổi màn.
7. Gán screen=requested, cập nhật pause audio.
8. Vẽ game hoặc nền menu, phủ UI; vẽ thông báo lỗi/FPS nếu cần.
9. RenderPresent, cập nhật queue audio khi không pause; đếm frame cho smoke.

Việc dùng requested gom chuyển trạng thái về một chỗ. Tuy nhiên các nhánh
trong frame vẫn đọc screen cũ trước phép gán; khi debug cần phân biệt hai biến.

## Luồng menu

Start nạp màn 0; Select dùng choice chọn file; Controls chỉ vẽ hướng dẫn.
Pause có Resume/Restart/Menu. Complete tăng current và nạp màn kế;
hết file thì Congrats, xác nhận để về Menu. Error xác nhận để thoát.
Esc đang Play yêu cầu Pause, đang Pause yêu cầu Play, màn phụ yêu cầu Menu.
Chữ và bố cục ở [MenuState](../game/MenuState.md), hành động ở App.

## Kết thúc và giải phóng

Ra khỏi scope bên trong run thì Sfx/Input/Renderer được hủy; texture thuộc
Assets bên trong Renderer được giải phóng trước SDL renderer.
Sau đó hủy SDL renderer, window, SDL_Quit rồi trả result.
SDL renderer thô do App sở hữu; wrapper Renderer không tự hủy con trỏ đó.

Smoke bỏ menu, nạp màn đầu và thoát sau 31 frame. Nó kiểm tra startup/render,
không kiểm tra hoàn thành toàn bộ gameplay. F3 hiển thị FPS; cập nhật vật lý
120 lần/giây không có nghĩa render cũng 120 FPS.

## Tự học

Đặt breakpoint ở load, `while(clock.ready())` và `screen=requested`.
Theo dõi một lần Start → Play → Pause → Resume. Nếu sửa menu, kiểm tra
cả chuột đóng cửa sổ, Esc và gamepad Start; build game xong mới kiểm tra --smoke.
