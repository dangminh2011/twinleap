# PlayState.cpp — điều phối gameplay và hiển thị phiên chơi

[Mã nguồn](../../../src/game/PlayState.cpp) · [Header](../../../include/twinleap/game/PlayState.hpp) · [Mục lục](../README.md)

## Tra cứu từng hàm

```cpp
bool PlayState::load(const std::string &path, std::string &error);
void PlayState::update(const std::array<PlayerInput, 2> &input,
                       float dt, Sfx &sfx);
void PlayState::renderWorld(Renderer &r, bool debug); // private
void PlayState::render(Renderer &r, bool debug);
```

| Hàm | Ý nghĩa tham số | Kết quả và thay đổi | Nơi gọi |
|---|---|---|---|
| load | path chỉ đọc; error nhận thông báo khi parser lỗi | bool; true khi nạp/reset phiên thành công; sửa level, players, thống kê và hiệu ứng | App, các test |
| update | input cho hai người chỉ đọc; dt giây; sfx để gửi yêu cầu âm thanh | void; sửa gameplay, objects, camera, timer, hạt; có thể đổi trạng thái audio qua sfx | App theo fixed step, test |
| renderWorld | r nhận lệnh vẽ; debug bật hitbox/vector | void; đổi camera render và trạng thái renderer; không tiến mô phỏng | Chỉ render trong chính PlayState |
| render | r để vẽ; debug bật thông tin kiểm tra | void; chọn viewport, vẽ thế giới và HUD; không tăng time/điểm | App mỗi frame, test render |

### Các hàm được sử dụng bên trong

| Lời gọi | Trả về / mục đích |
|---|---|
| `level.load(path,error)` | bool; đọc và kiểm tra map trước khi reset phiên |
| `std::filesystem::path(path).stem().string()` | Tên file không đuôi để chọn theme |
| `filename.rfind("02_",0)` | Vị trí tìm tiền tố; bằng 0 nghĩa bắt đầu bằng 02_ |
| `Physics::step(...)` | void; sửa một Player theo input và va chạm |
| `p.bounds()` / `overlaps(a,b)` | Rect / bool; vùng nhân vật và kiểm tra tương tác |
| `level.tileAt(x,y)` | char; nhận diện gai và chọn cách vẽ tile |
| `level.isSolidAt(x,y)` | bool; đếm tile chặn gần người trong debug |
| `updateDoors(level,players,dt)` | bool; có cửa đổi thì gọi âm Door |
| `view.update(players,w,h,dt)` | void; sửa vị trí camera chung |
| `particles.burst/update/render` | void; tạo hạt, mô phỏng hạt, vẽ hạt; ba bước riêng |
| `sfx.play(Sound::...)` | void; đặt voice SFX, không nạp WAV tại đây |
| `r.viewport(&area/nullptr)` | void; giới hạn vùng vẽ / trả toàn màn |
| `r.background/terrain/sprite/character/rect/line/text/centered` | void; vẽ qua wrapper Renderer, lỗi được giữ ở r.failed |
| `std::abs`, `std::min/max`, `std::clamp` | Độ lớn, chọn biên và kẹp camera/người/timer |
| `std::sin/cos` | Tạo độ lệch rung camera hiển thị |
| `std::fmod(time,60.f)` | Lấy phần giây trong phút khi vẽ HUD |
| `std::snprintf(...)` | Ghi chuỗi thống kê vào buffer có giới hạn kích thước |

Lambda `bound(value,size,screen)` trong render trả góc nhìn được căn giữa
hoặc kẹp theo map; riêng ở split view, screen=240.

### Ví dụ sử dụng tối thiểu

```cpp
std::string error;
if (game.load("assets/maps/01_first_steps.txt", error)) {
    game.update(inputs, 1.f / 120, sfx);
    game.render(renderer, false);
}
```

Ví dụ giả định SDL, sfx, renderer đã khởi tạo và inputs là array hai PlayerInput.
Đặt render trong vòng frame; update phải theo fixed timestep như App,
không gọi thêm update chỉ để làm hình ảnh chạy mượt.

## Vai trò và dữ liệu

App giữ một PlayState rồi gọi load/update/render. Đây là nơi ghép Level,
Physics, Entities, Camera, Particles, Renderer và Sfx.

| Dữ liệu | Vai trò |
|---|---|
| level, players[2] | Map và hai nhân vật |
| score/deaths/time | Thống kê riêng của màn |
| complete | Cả hai đã tới exit |
| checkpoint/hasCheckpoint | Điểm hồi sinh chung |
| camera | Góc nhìn chung đang lưu |
| splitView | Đang chia màn hay không |
| theme | Chọn nền/địa hình |
| particles/shake | Hạt và thời gian rung còn lại |

## load(path, error)

Gọi level.load; thất bại trả false ngay. Thành công chọn theme bằng tiền tố
02_/03_/04_ tương ứng 1/2/3, file khác mặc định 0.
Reset players, đặt pos và previous về spawn; reset điểm, chết, time, shake,
complete, checkpoint flag, splitView, particles và camera. Âm thanh thuộc App
nên mute/nhạc không bị tạo lại khi load. Reset checkpoint flag đủ để tọa độ
checkpoint cũ không còn được dùng.

## update(input, dt, sfx): theo từng khối

### Chuẩn bị và thứ tự nhân vật

complete=true thì return để đóng băng phiên đã hoàn thành.
Tăng time, giảm shake về 0. Người có pos.y lớn hơn cập nhật trước.
Trong tọa độ màn hình Y tăng xuống dưới, đây là người thấp hơn,
giúp carry của người trên dùng chuyển động mới của người dưới.

### Hồi sinh

Dead giảm respawnTimer. Hết thời gian thì `p=Player{}` để xóa vận tốc/timer,
đặt pos tại checkpoint chung nếu có hoặc spawn riêng. Nếu hitbox giao đồng đội,
dịch X thêm 16 pixel rồi gán previous=pos. Nhánh này continue nên không chạy
vật lý/tương tác trong cùng lượt vừa hồi sinh.

### Vật lý và hiệu ứng

Lưu grounded cũ, gọi Physics::step. Nếu trước đó có nền và sau bước vel.y âm
thì yêu cầu Jump. Nếu từ không grounded thành grounded thì tạo 5 hạt bụi.
Điều kiện phát âm nhảy này dựa vào trạng thái nền trước bước, nên không phải
mọi thay đổi vel.y âm đều tự có âm Jump.

### Gai và hố

Dead khi pos.y vượt level.height*16+64 hoặc hitbox giao vùng gai.
Vùng nguy hiểm của mỗi tile gai là `(tileX*16+2,tileY*16+8,12,8)`;
ảnh 16 × 16 không phải toàn bộ vùng chết.
Khi chết: 25 hạt, status Dead, timer=RESPAWN_DELAY, deaths+1,
shake=0,15 giây, Sound::Death rồi continue để bỏ các tương tác còn lại.

### Xu, checkpoint và exit

Giao xu chưa active: active=true, score+100, 12 hạt, Sound::Coin.
Giao checkpoint chưa active: tắt các cờ checkpoint khác, bật cờ này,
lưu `(box.x+2,box.y+2)` cho cả hai. Quay lại chạm cờ cũ có thể kích hoạt lại nó.

Giao exit: status AtExit, vel={}. Physics bỏ qua AtExit nên nhân vật đứng yên
chờ bạn. Có nhánh trả về Alive nếu ra khỏi vùng exit, nhưng input thường
không tự đưa AtExit ra ngoài vì Physics đã bỏ qua trạng thái đó.

### Sau lượt hai người

updateDoors đổi trạng thái cửa, có thay đổi thì phát Door.
Cả hai AtExit thì complete=true và phát Win; update kế tiếp return ngay
nên không phát Win lặp mãi. App thấy complete sẽ chuyển màn hình Complete.

### Camera và chia/gộp

Tính Camera chung, rồi khoảng cách X/Y giữa hai pos. Chỉ chia khi cả hai khác Dead.
Chưa chia thì bật nếu ngang>400 hoặc dọc>180. Đã chia thì giữ nếu ngang>320
hoặc dọc>140. Ngưỡng khác nhau tránh đổi layout liên tục khi đứng sát biên.

Không chia thì chỉnh camera giữ người sống trong khung và kẹp camera theo map.
Sau đó kẹp X của người Alive theo camera, nhưng dùng previous để tránh kéo
người hồi sinh ở xa vào địa hình khác. Không kẹp Y, cho phép rơi xuống hố.
Cuối cùng particles.update(dt).

## renderWorld(r, debug)

Nếu shake còn thì cộng sin/cos biên độ 2 vào camera render, không sửa pos.
Vẽ theo thứ tự: background → tile solid/one-way/gai → objects → exit
→ nhân vật còn sống → hitbox/vector debug → particles.
Cửa mở vẽ còn một dải rộng 2px; cửa đóng vẽ toàn ô.
Xu active không vẽ; cờ đổi màu; nút hạ thấp khi được giữ.

Render không cập nhật va chạm. Di chuyển hình vẽ mà không sửa hitbox
chỉ đổi cách nhìn, không đổi gameplay.

## render(r, debug)

Chia màn: hai viewport `(0,30,240,240)` và `(240,30,240,240)`.
Mỗi bên tính camera bám riêng vào người tương ứng, kẹp theo kích thước vùng 240.
Gọi renderWorld hai lần cho cùng thế giới. Sau đó viewport(nullptr) trả về
toàn màn, vẽ vạch chia và nhãn P1/P2.

Không chia: dùng camera chung và renderWorld một lần.
HUD nằm trên cùng 30px, hiển thị status hai người, tên màn, score/deaths/time.
snprintf định dạng thời gian phút/giây. Debug đếm solid trong vùng 5 × 5
quanh mỗi người; vùng trùng có thể bị đếm hai lần, đây không phải số tile duy nhất.

## Theo dõi một chuỗi sự kiện

P1 nhặt xu: overlaps → active → score → particles/Sfx. P2 chạm lại không cộng điểm.
P1 chạm C rồi P2 chết: checkpoint được chia sẻ → P2 chờ một giây → tạo Player mới
ở checkpoint → camera cập nhật hoặc chia nếu hai người cách xa.

Đặt breakpoint tại các phép gán status, checkpoint, complete và splitView.
Đọc [test_gameplay](../tests/test_gameplay.md) và
[test_render_assets](../tests/test_render_assets.md) để xem cách tái tạo các tình huống.
