# Camera.cpp — góc nhìn theo trung điểm

[Mã nguồn](../../../src/game/Camera.cpp) · [Header](../../../include/twinleap/game/Camera.hpp) · [Mục lục](../README.md)

## Tra cứu hàm và cách gọi

```cpp
void Camera::update(const std::array<Player, 2> &players,
                    int width, int height, float dt);
```

| Thành phần | Ý nghĩa |
|---|---|
| players | Hai người chỉ đọc; Dead bị bỏ qua, AtExit vẫn tính |
| width, height | Kích thước map theo tile; bên trong nhân 16 ra pixel |
| dt | Giây của bước, quyết định độ mượt |
| Trả về | void; kết quả được ghi vào this->position |
| Thay đổi | Chỉ position của Camera, không thay vị trí Player |
| Nơi gọi | PlayState::update qua một Camera tạm |

Lambda `bound(value,size,screen)` trả float: map nhỏ thì căn giữa,
map lớn thì dùng `std::clamp(value,0.f,size-screen)`.
`std::exp(-8*dt)` trả e lũy thừa -8dt; `1-exp(...)` là hệ số tiến về mục tiêu.

```cpp
twinleap::Camera view;
view.position = camera;
view.update(players, level.width, level.height, 1.f / 120);
camera = view.position;
```

Nếu không chép camera cũ vào position trước mỗi lần tạo view,
hiệu ứng mượt sẽ bắt đầu lại từ (0,0) thay vì nối tiếp bước trước.

## Hàm update(players, width, height, dt)

Nhận hai Player, kích thước map **theo tile** và dt giây. Kết quả được ghi
vào Camera::position, là tọa độ thế giới tại góc trên trái vùng nhìn chung.
Hàm không đổi vị trí nhân vật, không tự vẽ và không quyết định chia màn.

## Tính mục tiêu

Duyệt người có status khác Dead; cộng tâm hitbox `(pos.x+6,pos.y+7)`.
AtExit vẫn tính. Không có người sống thì return, giữ nguyên position.
Chia tổng cho số người rồi trừ `(240,135)` là nửa màn 480 × 270.

Ví dụ hai tâm ở X=200 và X=400 thì trung điểm là 300;
camera mục tiêu X=60 trước khi giới hạn theo map.

## Lambda bound và làm mượt

bound nhận tọa độ góc nhìn mong muốn, kích thước map pixel và kích thước vùng nhìn.
Map nhỏ: trả `(size-screen)/2`, có thể âm để căn giữa.
Map lớn: clamp trong `[0,size-screen]` để không nhìn quá biên.
width/height được nhân 16 trước khi truyền vào.

Sau đó `smooth=1-exp(-8*dt)` và `position += (target-position)*smooth` theo mỗi trục.
Với dt=1/120, smooth khoảng 0,0645: mỗi bước tiến khoảng 6,45% phần còn thiếu.
Mục tiêu đổi thì camera tiếp tục từ vị trí hiện tại, không nhảy ngay tới đó.

## Camera chung và split view khác nhau

PlayState tạo Camera tạm mỗi update, nạp position từ camera đang lưu rồi chép
kết quả về. Sau đó PlayState còn chỉnh camera chung để bắt kịp người hồi sinh.
Vì vậy vị trí cuối cùng có thể khác riêng kết quả làm mượt của file này.

Ngưỡng chia/gộp và hai camera viewport nằm ở [PlayState.cpp](PlayState.md).
Khi chia, mỗi bên dùng vùng 240 × 240 và tính góc nhìn riêng khi render.

## Tự học

Đặt breakpoint trước/sau phép làm mượt, xem target và position khi chạy sang phải.
Tăng hệ số 8 khiến bám nhanh hơn, giảm thì trễ hơn. Thay đổi này chỉ tác động
camera chung; kiểm tra cả lúc hồi sinh và chia/gộp để hiểu phần PlayState chỉnh thêm.
