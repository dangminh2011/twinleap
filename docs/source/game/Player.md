# Player.cpp — hình chữ nhật va chạm của nhân vật

[Mã nguồn](../../../src/game/Player.cpp) · [Header](../../../include/twinleap/game/Player.hpp) · [Mục lục](../README.md)

## Tra cứu hàm bounds

```cpp
Rect Player::bounds() const;
```

| Thành phần | Giải thích |
|---|---|
| Tham số | Không có; đọc pos của chính đối tượng gọi hàm |
| Trả về | Một bản Rect có x=pos.x, y=pos.y, w=12, h=14 |
| Dữ liệu thay đổi | Không có; const ở cuối cho biết hàm không sửa Player |
| Nơi gọi | Physics, PlayState, Entities dùng hình này để kiểm tra chạm |
| Hàm được gọi bên trong | Không gọi SDL/STL hoặc hàm project khác; chỉ tạo giá trị Rect |

```cpp
twinleap::Player p;
p.pos = {50, 100};
twinleap::Rect box = p.bounds(); // box = {50,100,12,14}
box.x = 0;                     // p.pos.x vẫn là 50
```

`p.bounds()` là gọi phương thức trên p; `Player::bounds` là cách ghi tên đầy đủ
của phương thức khi định nghĩa. `return {...}` tạo giá trị trả về, không trả
con trỏ hoặc tham chiếu tới một Rect tạm.

## Vai trò và dữ liệu

Player.hpp khai báo dữ liệu nhân vật; Player.cpp chỉ cài đặt bounds().
Di chuyển nằm trong [Physics](Physics.md), chết/hồi sinh ở [PlayState](PlayState.md).

| Thành viên | Ý nghĩa |
|---|---|
| pos | Góc trên trái hitbox, pixel logic |
| previous | Vị trí trước bước vật lý, dùng carry/va chạm |
| vel | Vận tốc pixel/giây |
| onGround | Có điểm tựa |
| coyoteTimer | Thời gian còn được nhảy sau khi rời mép |
| jumpBufferTimer | Thời gian giữ yêu cầu nhảy sớm |
| respawnTimer | Thời gian chờ hồi sinh |
| facing | 1 nhìn phải, -1 nhìn trái |
| status | Alive, Dead hoặc AtExit |

## Player::bounds() const

Hàm trả Rect `{pos.x, pos.y, 12, 14}` theo giá trị. `const` phía cuối nghĩa là
hàm không sửa Player. Bản Rect trả về độc lập với Player.

Ví dụ pos=(50,100) tạo hitbox từ (50,100) tới (62,114).
Sửa tọa độ trên Rect nhận về không tự thay đổi pos.
Physics dùng hitbox kiểm tra nền; PlayState dùng kiểm tra xu, gai, checkpoint, exit.

## PlayerInput trong header

moveX nằm trong [-1,1]. jumpPressed là vừa bấm, jumpHeld đang giữ,
jumpReleased vừa nhả. Dữ liệu này không chứa kiểu SDL, nên test Physics
có thể tự tạo input mà không đọc bàn phím.

`Player p{}` tạo vận tốc/timer bằng 0, status Alive, facing=1.
PlayState dùng `p = Player{}` khi hồi sinh để bỏ vận tốc và timer cũ.

## Điều dễ nhầm và bài tập

Sprite 24 × 24 không phải hitbox 12 × 14. Nếu đổi bounds, cần rà số 12/14
ở Physics, PlayState, Entities và phần căn sprite trong Renderer.
Giữ Player.cpp dù ngắn để cấu trúc header/nguồn nhất quán.

Đặt breakpoint ở bounds và quan sát Rect khi nhân vật di chuyển.
Đọc [test_physics](../tests/test_physics.md) để xem cách tạo Player cho một tình huống cụ thể.
