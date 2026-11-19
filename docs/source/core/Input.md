# Input.cpp — bàn phím, gamepad và cạnh nhấn

[Mã nguồn](../../../src/core/Input.cpp) · [Header](../../../include/twinleap/core/Input.hpp) · [Mục lục](../README.md)

## Tra cứu từng hàm

| Chữ ký | Tham số và trả về | Thay đổi và nơi gọi |
|---|---|---|
| `void Input::event(const SDL_Event &e)` | e là sự kiện chỉ đọc; void không trả kết quả | Đổi cờ nhấn/menu hoặc mở/đóng pads; App gọi cho từng sự kiện |
| `void Input::poll()` | Không tham số; void | Ghi players/held theo trạng thái thiết bị; App gọi sau vòng lấy sự kiện mỗi frame |
| `void Input::consume()` | Không tham số; void | Xóa các cạnh nhấn/nhả và cờ menu đã dùng; App gọi sau xử lý menu/update |
| `Input::~Input()` | Destructor không có kiểu trả về | Đóng pad còn mở; C++ tự gọi khi Input hết vòng đời |

### Các hàm SDL/STL được sử dụng

| Lời gọi | Đầu vào → kết quả | Dùng để làm gì ở đây? |
|---|---|---|
| `SDL_GetKeyboardState(nullptr)` | Không lấy số lượng phím → con trỏ mảng bool do SDL quản lý | Đọc k[scancode]; không tự giải phóng mảng |
| `SDL_OpenGamepad(id)` | ID thiết bị → SDL_Gamepad* hoặc nullptr | Giữ pad trong slot trống; nullptr thì log |
| `SDL_GetGamepadID(pad)` | Pad đang mở → ID | So sánh với ID trong sự kiện để chọn đúng người |
| `SDL_CloseGamepad(pad)` | Pad đang mở → không trả dữ liệu | Giải phóng handle, sau đó code đặt slot nullptr |
| `SDL_GetGamepadAxis(pad, axis)` | Pad và trục LEFTX → số nguyên có dấu | Chia 32767.f để thành độ nghiêng float, xét deadzone |
| `SDL_GetGamepadButton(pad, button)` | Pad và nút → bool | Đọc D-pad/SOUTH đang giữ |
| `std::abs(axis)` | Số float → độ lớn không âm | So sánh deadzone cả trái lẫn phải |
| `std::clamp(x,-1.f,1.f)` | Giá trị và hai biên → giá trị trong biên | Giới hạn tổng keyboard+pad |
| `SDL_GetError()` / `SDL_Log(...)` | Lấy lỗi chuỗi / ghi log | Báo không mở được tay cầm |

### Ví dụ thứ tự gọi

```cpp
SDL_Event e;
while (SDL_PollEvent(&e))
    input.event(e);
input.poll();
// Khi một bước gameplay đã sử dụng input.players:
input.consume();
```

`SDL_PollEvent(&e)` trả bool: true khi lấy được một sự kiện và ghi vào e.
Không gọi consume ngay sau poll nếu bước logic chưa sử dụng cạnh nhấn.

## Vai trò và dữ liệu

App đưa SDL_Event vào event rồi gọi poll mỗi frame. Kết quả là players[2]
và các cờ confirm/pause/up/down, để App/Physics dùng mà không cần tự đọc phím.
pads giữ hai con trỏ gamepad; held giữ trạng thái nhảy ở lần poll trước.

## event(e)

Nút SOUTH của pad được đối chiếu ID với slot để đặt jumpPressed/Released.
Key down không repeat hoặc key up của W/Space thuộc P1, Up/RightCtrl thuộc P2.
Đọc sự kiện giúp giữ được cú bấm/nhả nhanh giữa các lần lấy trạng thái.

GAMEPAD_ADDED mở thiết bị vào slot trống đầu tiên; mở lỗi thì log.
GAMEPAD_REMOVED tìm ID, đóng và gán nullptr để không dùng con trỏ cũ.
Hai slot đầy thì không nhận thêm pad. Gamepad được gán theo slot trống,
không gắn cố định với một tay cầm sau khi rút/cắm lại.

Key down/nút down còn đặt các cờ menu. SOUTH vừa là nhảy trong gameplay
vừa confirm trong menu; START pause, D-pad up/down điều hướng.
Phím Enter/Esc/W/S/Up/Down tương ứng các hành động menu.
Các phép `|=` giữ cờ true nếu trước đó đã có sự kiện trong frame.

## poll()

Đọc bool keyboard state. P1 lấy D-A, P2 lấy Right-Left; ép bool sang float
biến bấm phải thành +1, trái thành -1, bấm cả hai thành 0.
Nếu có pad, cộng stick X khi trị tuyệt đối >0,25 và cộng D-pad.
Cuối cùng clamp [-1,1]. Bàn phím và pad cộng chung nên hai hướng ngược
có thể triệt tiêu nhau.

jump là OR các phím nhảy/nút SOUTH. jumpHeld nhận giá trị hiện tại.
jumpPressed được giữ bằng OR khi jump mới bật. jumpReleased chỉ giữ khi
hiện không còn jump và đã có sự kiện nhả hoặc held trước đó là true.
Cuối lượt, held được cập nhật.

## consume() và destructor

consume xóa jumpPressed/Released và các cờ menu, giữ moveX/jumpHeld.
App gọi sau update đầu tiên dùng input, không reset ngay khi frame kết thúc.
Destructor đóng từng pad khác nullptr; App phải giữ SDL sống tới khi hủy Input.

## Ví dụ fixed timestep

| Thời điểm | Kết quả |
|---|---|
| Frame A nhận bấm W nhưng chưa đủ thời gian update | jumpPressed vẫn true |
| Frame B đủ một bước | Physics nhận jumpPressed, App consume |
| Frame B còn chạy update thứ hai | jumpPressed=false, không bấm nhảy thêm |

Lưu cạnh bấm như vậy tránh mất input ở frame không update. Giữ W nhiều frame
không đồng nghĩa jumpPressed luôn true.

## Debug và thay đổi

Xem moveX/jumpHeld/Pressed/Released ở cuối poll, rồi Step Into Physics.
Đổi phím nhảy cần sửa cả nhánh event và poll; cập nhật chữ Controls/README.
Test hiện có mô phỏng PlayerInput, chưa thay thế thử gamepad thật và hot-plug.
