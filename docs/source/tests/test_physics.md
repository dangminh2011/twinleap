# test_physics.cpp — mô phỏng vật lý

[Mã nguồn](../../../tests/test_physics.cpp) · [Physics](../game/Physics.md) · [Mục lục](../README.md)

## Tra cứu hàm và macro

| Chữ ký / macro | Tham số | Trả về / dữ liệu thay đổi |
|---|---|---|
| `static Level arena()` | Không có | Bản Level 30×20 có nền; không đọc file |
| `static void step(Player &p,const Level &l,int n,PlayerInput in={})` | p được cập nhật; l chỉ đọc; n số bước; in dùng lặp lại | void; gọi Physics::step n lần và sửa p |
| `static float jumpHeight(bool cut)` | true nhả sớm, false nhảy đủ | Độ cao pixel = Y ban đầu - Y nhỏ nhất; dùng Player/Level cục bộ |
| `int main()` | Không có | 0 tất cả đạt, 1 khi CHECK thất bại |
| `CHECK(x)` | Biểu thức bool | Macro, không phải hàm: false thì in lỗi và return 1 từ main |

main gọi arena, helper step, jumpHeight và Physics::step trực tiếp khi cần đồng đội.
jumpHeight gọi arena/Physics::step và `std::min(peak,p.pos.y)` để lấy đỉnh.
`tiles.assign(20,string(30,'.'))` tạo 20 dòng; `std::abs` so khoảng sai số float;
std::cout/cerr ghi kết quả/lỗi. Không có hàm SDL nào được gọi trong file test này.

Ví dụ `step(p,l,6)` mô phỏng 6/120=0,05 giây. `jumpHeight(true)` trả số đo,
không trực tiếp kết luận đạt; main mới so số đó bằng CHECK.

## Mục đích và các helper

Kiểm tra vật lý không mở cửa sổ hoặc đọc bàn phím. CHECK(x) in số dòng,
biểu thức và return 1 từ main khi sai; không bị tắt trong Release như assert.

`dt=1/120` khớp game. `arena()` tạo map 30 × 20 với nền ở hàng 15.
Tạo dữ liệu trực tiếp tránh để lỗi parser ảnh hưởng test vật lý.
`step(p,l,n,in)` gọi Physics::step n lần với other=nullptr.
`jumpHeight(cut)` nhảy từ Y=226, lấy Y nhỏ nhất trong 120 bước rồi trả
226-peak. cut=true tạo cạnh nhả tại lượt i=5 của vòng theo dõi.

## Các tình huống trong main

| Thiết lập | Kỳ vọng |
|---|---|
| Rơi xuống nền | onGround=true, vel.y=0 |
| Nhảy giữ/nhả sớm | Cao 48–56px; nhả sớm thấp hơn trên 12px |
| Rời mép 6/24 bước | Khoảng 0,05s vẫn nhảy; 0,2s thì không |
| Bấm trước lúc đáp | Buffer làm nhân vật nhảy sau tiếp đất |
| Đi vào tường khi rơi nhanh | Hitbox không vượt tường |
| One-way | Lên xuyên, xuống đứng ở Y≈146 |
| Rơi lên đồng đội, người dưới dịch X=2 | Đứng đầu và được carry |
| Tường bốn tile | Không lên một mình; nhảy từ đầu bạn lên được |

PlayerInput khởi tạo theo moveX, jumpPressed, jumpHeld, jumpReleased.
Sai số 0,1/0,3 tránh đòi float chính xác tuyệt đối với bước xử lý va chạm 0,25px.

## Chạy và tự học

```powershell
ctest --test-dir build-windows -R '^physics$' --output-on-failure
```

Đặt breakpoint tại CHECK lỗi, xem p rồi theo vào Physics.
Thêm test theo mẫu: tạo arena → đặt trạng thái → mô phỏng số bước cố định → CHECK.
Không đo bằng đồng hồ máy vì khó lặp lại. Đổi hằng số nhảy có thể làm kỳ vọng
độ cao thất bại; chỉ sửa kỳ vọng nếu bạn chủ động đổi yêu cầu gameplay.
