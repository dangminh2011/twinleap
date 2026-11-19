# Physics.cpp — chạy, nhảy và va chạm

[Mã nguồn](../../../src/game/Physics.cpp) · [Header và hằng số](../../../include/twinleap/game/Physics.hpp) · [Mục lục](../README.md)

## Tra cứu hàm và tham số

```cpp
static bool solid(Rect r, const Level &l);
void Physics::step(Player &p, const PlayerInput &in,
                   const Level &l, const Player *other, float dt);
```

`solid` nhận bản sao hitbox và level chỉ đọc, trả true nếu hitbox phủ solid.
Chỉ step dùng helper này; bên ngoài Physics.cpp không gọi được nó.

| Tham số step | Ý nghĩa |
|---|---|
| p | Nhân vật cần cập nhật; tham chiếu cho phép sửa đối tượng gốc |
| in | Ý định di chuyển/nhảy đã xử lý; chỉ đọc |
| l | Map và cửa; chỉ đọc |
| other | Con trỏ tới đồng đội để đứng đầu/carry; nullptr khi không xét đồng đội |
| dt | Độ dài bước theo giây; game truyền 1.f/120 |

step trả void. Kết quả nằm trong p.pos, p.previous, p.vel, p.onGround,
p.facing và các timer nhảy. PlayState hoặc test gọi step; nó không sửa level/other.

### Các hàm được gọi bên trong

| Hàm | Kết quả và mục đích |
|---|---|
| `p.bounds()` | Rect hitbox hiện tại; dùng dò solid |
| `l.isSolidAt(x,y)` | bool; xét cả tile # và cửa đóng |
| `l.tileAt(x,y)` | char; nhận diện nền = riêng |
| `approach(current,target,maxDelta)` | Hàm Math.hpp đưa số dần tới target, không vượt target |
| `std::floor(value)` | Làm tròn xuống; đổi biên hitbox sang chỉ số tile trong solid |
| `std::abs(value)` | Độ lớn của vận tốc/độ dịch hoặc khoảng cách |
| `std::min(a,b)` | Chọn nhỏ hơn; giới hạn rơi hoặc lấy vận tốc hướng lên mạnh hơn |
| `std::max(a,b)` | Chọn lớn hơn; không cho timer âm |

### Ví dụ gọi một bước

```cpp
twinleap::PlayerInput input{1.f, true, true, false};
twinleap::Physics::step(player, input, level, &teammate, 1.f / 120);
```

Ví dụ cần player/teammate đã có vị trí hợp lệ, level đã nạp và player có
điểm tựa/coyote nếu muốn nhảy ngay. `&teammate` lấy địa chỉ; trong hàm,
`other->pos` đọc pos qua con trỏ. Truyền nullptr để bỏ xét đồng đội.

## Vai trò và giao diện

PlayState gọi `Physics::step(p, in, level, other, dt)` mỗi bước logic.
Hàm sửa p qua tham chiếu; chỉ đọc input/level/other. other có thể nullptr.
Không gọi SDL, không tự xử lý xu, gai, hồi sinh hoặc thắng màn.
Không gọi đây là hàm thuần toán học vì nó thay đổi p.

## solid(r, level): hàm phụ nội bộ

Chia tọa độ Rect cho 16 và floor để ra vùng chỉ số tile.
Trừ 0,01 ở mép phải/dưới để cạnh vừa chạm không tính thêm ô bên cạnh.
Chỉ cần một ô isSolidAt=true là trả true. `static` ở đây giới hạn hàm
trong file nguồn, không phải một biến global.

## step: đọc theo thứ tự thực hiện

### 1. Trạng thái và carry

Dead/AtExit trả về ngay. Alive lưu previous=pos.
Nếu đang grounded, chân gần đỉnh cũ của đồng đội và chồng nhau theo X,
nhân vật được cộng `other.pos.x-other.previous.x` nếu vị trí mới không chạm solid.
Người dưới có vel.y âm thì người trên nhận tối thiểu JUMP_VELOCITY*0,9.
PlayState cập nhật người dưới trước để độ dịch này thuộc bước hiện tại.

### 2. Timer nhảy

Có nền thì coyoteTimer=0,09; không có thì giảm về 0.
Vừa bấm nhảy thì jumpBufferTimer=0,10; nếu không thì giảm về 0.
Cả hai dương thì vel.y=-340, onGround=false và xóa hai timer.
Nhờ vậy có thể bấm hơi sau khi rời mép hoặc hơi trước khi đáp.
Buffer sau khi đáp thường kích hoạt ở update tiếp theo.

Nhả nhảy khi vel.y<0 thì nhân 0,45. Ví dụ -200 thành -90: nhân vật
vẫn lên nhưng ít hơn. jumpHeld hiện chưa dùng cho giảm trọng lực ở đỉnh.

### 3. Vận tốc

facing lấy dấu moveX khi có di chuyển. approach đưa vel.x tới moveX*110,
chọn gia tốc hoặc ma sát theo grounded/air và có bấm hướng hay không.
Trọng lực: `vel.y = min(420, vel.y + 1100*dt)`.
Sau đó độ dịch là vận tốc nhân dt, không phải vận tốc cộng thẳng vào vị trí.

### 4. Giải va chạm X rồi Y

Thử di chuyển cả độ dịch. Nếu vị trí mới overlap solid thì quay lại,
tiến từng 0,25 pixel tới sát vật cản và đặt vận tốc trục đó bằng 0.
Va chạm Y khi rơi còn bật onGround; đụng trần không bật nền.
Tách trục giúp không phải chọn một hướng đẩy ra cho va chạm hai chiều cùng lúc.

Đây không phải thuật toán quét toàn bộ quãng đường: chỉ lùi/tiến nhỏ sau khi
vị trí mới đã chạm. Code dựa vào dt=1/120 và tốc độ giới hạn để không đi xuyên
tile; tăng dt hoặc tốc độ tùy ý có thể phá giả thiết này.

### 5. One-way, đầu đồng đội và dò nền

Lưu oldFoot trước dịch Y. Khi dy>=0, nền = chỉ đỡ nếu chân cũ chưa thấp hơn
mặt nền và chân mới đã tới mặt nền. Đặt pos.y=tileY*16-14.
Đi lên từ dưới không bị xét nên xuyên qua.

Đầu đồng đội dùng điều kiện chân cũ phía trên, chân mới chạm đỉnh mới và
hitbox chồng X. Đồng đội Dead không làm điểm tựa; AtExit có thể đỡ khi rơi,
nhưng khối carry đầu hàm chỉ xét đồng đội Alive.
Cuối hàm dịch một bản hitbox xuống 1 pixel để dò solid khi vel.y>=0.

## Ví dụ một bước nhảy

Nhân vật đang grounded, vừa bấm nhảy, dt=1/120:

| Thời điểm | Giá trị |
|---|---|
| Kiểm tra timer | Coyote=0,09; buffer=0,10 |
| Bắt đầu nhảy | vel.y=-340; onGround=false |
| Sau trọng lực | vel.y xấp xỉ -330,83 |
| Di chuyển Y nếu không vướng | pos.y giảm xấp xỉ 2,76 pixel |

## Debug và bài tập

Theo dõi previous, oldFoot, dy, onGround, coyoteTimer, jumpBufferTimer.
Đừng chỉ xem sprite: bật F3 để thấy hitbox và vector vận tốc.
Chỉnh RUN_SPEED để học gia tốc trước; chỉnh GRAVITY/JUMP_VELOCITY làm đổi cả
độ cao và thời gian bay, có thể khiến puzzle không còn giải được.

Đọc [test_physics](../tests/test_physics.md): các test có tình huống nền, nhảy
48–56px, nhả sớm, coyote/buffer, one-way, carry và bức tường cần đồng đội.
