# MenuState.cpp — vẽ menu và màn thông báo

[Mã nguồn](../../../src/game/MenuState.cpp) · [Header](../../../include/twinleap/game/MenuState.hpp) · [Mục lục](../README.md)

## Tra cứu hàm và tham số

```cpp
void MenuState::render(Renderer &r, Screen screen, int choice,
                       const std::vector<std::string> &files,
                       const PlayState &game);
```

| Tham số | Dùng thế nào? |
|---|---|
| r | Nhận lệnh vẽ, có thể đổi trạng thái render/failed |
| screen | Chọn nhánh UI; truyền theo giá trị, không bị hàm đổi màn |
| choice | Dòng chọn từ 0; hàm chỉ vẽ dấu >, không điều hướng |
| files | Các tên màn đã chuẩn bị; chỉ đọc khi vẽ Select |
| game | Chỉ đọc score/deaths/time để vẽ Complete |

Trả void; App hoặc test render gọi. Trong header, phương thức được khai báo
`static` bên trong lớp; phần định nghĩa bên ngoài không ghi lại từ khóa này.
`static` của phương thức nghĩa không cần
tạo đối tượng MenuState để gọi, khác `static` helper nội bộ trong Physics.cpp.

Lambda `menu(labels,y)` nhận danh sách nhãn và Y đầu, trả void; dùng r/choice
bắt bằng tham chiếu để vẽ mỗi mục. `std::strlen(label)` trả số byte chuỗi C,
dùng tính vị trí dấu chọn; `std::snprintf` ghi thống kê có giới hạn;
`SDL_GetTicksNS()` trả thời gian nano để animation nhân vật ngoài gameplay.
r.rect/character/text/centered đều là hàm wrapper vẽ trả void.

```cpp
twinleap::MenuState::render(renderer, twinleap::Screen::Pause,
                           0, displayNames, game);
```

Lệnh này vẽ Resume được chọn; nó không tự pause mô phỏng.
App phải ngừng game.update và chọn Screen::Pause riêng.

## render(r, screen, choice, files, game)

Nhận renderer, enum màn hình, chỉ số lựa chọn, danh sách tên file và game chỉ đọc.
Hàm vẽ UI; không đọc bàn phím, nạp map hoặc tự chuyển screen.
[App](../core/App.md) là nơi xử lý hành động confirm/back.

## Lambda menu

Nhận initializer_list các nhãn và Y bắt đầu, vẽ từng dòng cách nhau 18 pixel.
Nếu chỉ số dòng bằng choice thì vẽ dấu > cạnh nhãn.
strlen(label)*4 hỗ trợ đặt dấu chọn dựa trên nửa bề rộng chữ font 8px.
Lambda bắt r và choice từ phạm vi ngoài bằng tham chiếu.

## Các nhánh màn hình

| Screen | Nội dung |
|---|---|
| Menu | Start, Select Level, Controls, Quit; nhân vật trang trí |
| Select | Các tên files, dấu > theo choice |
| Controls | Phím của P1/P2, pad và các phím chung |
| Pause | Khung phủ game, Resume/Restart/Menu |
| Complete | Điểm, deaths, thời gian; nhắc sang màn |
| Congrats | Chúc mừng, nhắc về menu |
| Error | Thông báo không nạp được; chi tiết lỗi do App vẽ thêm |

Không có nhánh Play, vì frame gameplay đã do PlayState vẽ.
Các hình khung gọi rect với world=false để tọa độ không chạy theo camera.
Nhân vật trang trí dùng SDL_GetTicksNS để animation còn chạy ngoài gameplay.

## Chuỗi và dữ liệu

Complete dùng snprintf vào buffer 100 ký tự rồi centered.
game được truyền const nên vẽ thống kê không làm thay đổi điểm/thời gian.
Tên Select hiện là tên file không đuôi do App chuẩn bị, không phải metadata name.

## Tự học và giới hạn

Đổi nhãn Resume, build rồi Esc để quan sát. Thêm mục menu phải sửa cả nhãn,
số lựa chọn và hành động trong App; đổi chữ ở đây không tạo chức năng mới.
Danh sách Select vẽ theo dòng cố định, chưa cuộn nếu thêm quá nhiều màn.
Font debug dùng ASCII; không đưa chuỗi tiếng Việt có dấu vào UI hiện tại.
