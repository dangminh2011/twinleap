# Entities.cpp — trạng thái nút và cửa

[Mã nguồn](../../../src/game/Entities.cpp) · [Header](../../../include/twinleap/game/Entities.hpp) · [Mục lục](../README.md)

## Tra cứu chữ ký và cách gọi

```cpp
bool updateDoors(Level &level,
                 const std::array<Player, 2> &players, float dt);
```

| Thành phần | Giải thích |
|---|---|
| level | Tham chiếu có thể sửa; hàm cập nhật objects loại nút/cửa |
| players | Mảng đúng hai người, chỉ đọc trạng thái/chân/hitbox |
| dt | Giây đã trôi trong bước; game dùng 1/120 |
| Trả về | true khi ít nhất một cửa đổi active, không phải chỉ khi cửa đang mở |
| Nơi gọi | PlayState::update, test_buttons_doors |

Hàm con được dùng: `p.bounds()` lấy Rect của người; `overlaps(a,b)` trả bool
hai Rect giao nhau; `std::max(0.f,o.timer-dt)` giảm timer nhưng không cho âm.
Không gọi SDL, không tự phát âm thanh.

```cpp
if (twinleap::updateDoors(level, players, 1.f / 120))
    sfx.play(twinleap::Sound::Door);
```

Ví dụ cho thấy trách nhiệm tách đôi: updateDoors thay logic,
người gọi quyết định phát tiếng khi kết quả true.

## Hàm updateDoors(level, players, dt)

Đầu vào là Level có thể sửa, hai Player chỉ đọc và thời gian bước tính bằng giây.
Hàm thay active của nút/cửa, timer cửa; trả true nếu ít nhất một cửa đổi mở/đóng.
PlayState dùng giá trị trả về để yêu cầu Sound::Door. Hàm không tự phát tiếng.

## Lượt thứ nhất: nút a/b

Xóa active của nút để tính lại mỗi bước. Vùng nhận chân bắt đầu ở
`o.box.y+12`, cao 5 pixel. Chân người là Rect tại pos.y+13, cao 2 pixel.
Chỉ người chưa Dead và onGround mới giữ nút; bay ngang qua không giữ.

`held[o.kind-'a'] |= o.active` gộp trạng thái theo nhóm:
'a'-'a'=0, 'b'-'a'=1. Nhiều nút cùng nhóm dùng OR.

## Lượt thứ hai: cửa A/B

Lưu active cũ để so sánh. Nhóm có nút giữ thì timer=0,2;
không có thì giảm timer và chặn dưới ở 0.
occupied bật nếu có người chưa Dead giao vùng cửa, không cần onGround.

```cpp
o.active = o.timer > 0 || (o.active && occupied);
```

Nghĩa là cửa mở khi timer còn hoặc cửa vốn đang mở và vẫn có người trong.
Người đứng trong một cửa vốn đóng không tự mở nó. active=true của cửa nghĩa
là mở; Level::isSolidAt dùng thông tin này cho va chạm ở các bước kế tiếp.

## Ví dụ theo thời gian

Giữ a → A mở, timer=0,2. Rời nút 0,1 giây → timer còn 0,1, vẫn mở.
Rời thêm 0,11 giây → timer về 0; đóng nếu không occupied.
Nếu người đang trong cửa, cửa giữ mở tới khi người ra ngoài.

## Debug và thử sửa

Đặt breakpoint ở held, timer và phép gán active. Chú ý Object::active mang
ý nghĩa khác ở xu/checkpoint, nhưng file này chỉ sửa a/b/A/B.
Muốn thêm nhóm c cần cập nhật mảng held, parser và render; C hiện dành cho checkpoint.
Muốn đổi độ trễ thì sửa 0,2 và đọc [test_buttons_doors](../tests/test_buttons_doors.md).
