# Renderer.cpp — các thao tác vẽ dùng chung

[Mã nguồn](../../../src/render/Renderer.cpp) · [Header](../../../include/twinleap/render/Renderer.hpp) · [Mục lục](../README.md)

## Tra cứu chữ ký đầy đủ và cách sử dụng từng hàm

Các hàm dưới đây trả **void**: kết quả là lệnh vẽ/trạng thái renderer,
không phải ảnh trả về. Lỗi SDL được ghi vào failed thông qua check.
Giá trị mặc định ghi dưới đây nằm trong Renderer.hpp.

### check và viewport

```cpp
void check(bool success);
void viewport(const SDL_Rect *area);
```

check nhận kết quả SDL: false thì log và failed=true; true không xóa lỗi cũ.
App hoặc các helper vẽ gọi check.
viewport nhận con trỏ Rect **nguyên** giới hạn vùng vẽ; nullptr trả toàn vùng.
PlayState gọi khi chia/gộp màn. Nó không thay camera nên người gọi phải đặt
camera phù hợp với mỗi viewport.

### rect và line

```cpp
void rect(Rect b, SDL_Color c, bool world = true);
void line(Vec2 from, Vec2 to, SDL_Color color);
```

rect: b là x/y/w/h; c là RGBA; world=true trừ camera, false vẽ theo màn hình.
line: hai điểm đầu/cuối và màu, luôn trừ camera. Các đối số hình học là bản sao,
không sửa vị trí đối tượng game. PlayState/Particles/UI dùng rect,
debug trong PlayState dùng line.

```cpp
r.rect({100, 80, 16, 16}, {255, 0, 0, 255});       // Ô đỏ trong thế giới
r.rect({0, 0, 480, 30}, {20, 23, 43, 255}, false); // Thanh HUD đứng yên
r.line({100, 80}, {120, 80}, {0, 255, 0, 255});
```

### text và centered

```cpp
void text(float x, float y, const char *text,
          SDL_Color c = {235, 237, 250, 255});
void text(float x, float y, const std::string &s,
          SDL_Color c = {235, 237, 250, 255});
void centered(float y, const char *text);
void centered(float y, const std::string &s);
```

x/y theo vùng nhìn, không trừ camera; text là chuỗi C kết thúc bằng byte 0,
s là chuỗi C++ chỉ đọc. Hai overload string nằm inline trong header,
gọi phiên bản chuỗi C bằng c_str(). centered căn theo chiều rộng 480 và font 8px,
chỉ có màu mặc định. App/MenuState/PlayState gọi để vẽ chữ.
Ví dụ `r.text(4,4,"P1"); r.centered(100,"PAUSED");`.

### sprite

```cpp
void sprite(Sprite image, Rect source, Rect destination,
            bool world = true, bool flip = false, Uint8 alpha = 255);
```

image chọn texture; source cắt vùng pixel trong ảnh; destination định vị và
kích thước trên vùng vẽ. world quyết định trừ camera; flip lật ngang;
alpha từ 0 trong suốt tới 255 đậm hoàn toàn.
Helper character/terrain/background và PlayState gọi sprite.
Ví dụ `r.sprite(Sprite::Spikes,{0,0,16,16},{100,80,16,16});` sau khi load assets.

### character

```cpp
void character(int player, Rect hitbox, float velocityX, float velocityY,
               bool grounded, int facing, float time, bool atExit = false);
```

player=0 hoặc 1; hitbox định vị nhân vật; velocityX/Y dùng chọn tư thế,
không di chuyển hitbox; grounded chọn nhánh nền/không khí; facing=-1 lật trái,
1 nhìn phải; time là giây animation; atExit bật idle mờ.
PlayState, MenuState và test render gọi.
Ví dụ `r.character(0,p.bounds(),p.vel.x,p.vel.y,p.onGround,p.facing,time);`.

### terrain và background

```cpp
void terrain(int x, int y, bool top, bool left, bool right, int theme);
void background(int theme = 0);
```

terrain: x/y là chỉ số tile; top/left/right báo cạnh đang lộ;
theme chọn vùng atlas. PlayState truyền từ các ô lân cận.
background: theme chọn nền 0–3; sử dụng camera đang lưu để tính parallax.
App và PlayState gọi. Ví dụ `r.background(0); r.terrain(3,5,true,false,false,0);`.

### Các hàm SDL/STL bên dưới wrapper

| Hàm | Kết quả và công dụng |
|---|---|
| `SDL_SetRenderDrawColor(r,R,G,B,A)` | bool; đặt màu cho lệnh hình/chữ sau đó |
| `SDL_RenderFillRect(r,&rect)` | bool; tô SDL_FRect |
| `SDL_RenderLine(r,x1,y1,x2,y2)` | bool; vẽ đường |
| `SDL_RenderDebugText(r,x,y,s)` | bool; vẽ font debug |
| `SDL_SetRenderViewport(r,area)` | bool; đặt vùng vẽ |
| `SDL_SetTextureAlphaMod(texture,alpha)` | bool; đặt độ mờ cho texture |
| `SDL_RenderTextureRotated(...)` | bool; vẽ vùng texture có góc/lật; ở đây góc=0 |
| `assets.get(image)` | SDL_Texture* thuộc Assets; không tự hủy |
| `std::round(value)` | Làm tròn vị trí sprite để giữ pixel rõ |
| `std::abs(velocityX)` | Tốc độ không xét hướng, chọn idle/run |
| `std::fmod(value,period)` | Phần dư float, tạo nền lặp |
| `std::strlen(s)` | Số byte chuỗi C, dùng tính bề rộng chữ ASCII |
| `SDL_GetError()` / `SDL_Log(...)` | Lấy lỗi và ghi log trong check |

## Vai trò và trạng thái

Wrapper giữ con trỏ SDL_Renderer do App sở hữu, Assets, camera và cờ failed.
Không tự hủy SDL_Renderer; App hủy sau khi texture đã được giải phóng.
Gameplay chuẩn bị dữ liệu, Renderer chuyển dữ liệu thành lệnh SDL.

## Các hàm vẽ cơ bản

| Hàm | Cách hoạt động |
|---|---|
| check(ok) | Nếu false, log SDL_GetError và giữ failed=true |
| viewport(area) | Chọn vùng SDL render; nullptr trả về toàn vùng mặc định |
| rect(b,c,world) | Nếu world thì trừ camera, đổi Rect sang SDL_FRect rồi tô |
| line(from,to,c) | Trừ camera ở cả hai đầu, vẽ đường theo thế giới |
| text(x,y,s,c) | Vẽ debug text tại tọa độ vùng nhìn, không trừ camera |
| centered(y,s) | X=(480-strlen(s)*8)/2, sau đó gọi text |

Tham số b nhận theo giá trị nên trừ camera không sửa Rect của người gọi.
centered luôn dựa trên 480, không tự đổi thành 240 khi chia màn;
HUD/menu hiện được vẽ sau khi trả viewport đầy đủ.

## sprite(image, source, destination, world, flip, alpha)

Lấy texture từ Assets; thiếu texture thì failed=true và return.
source là vùng cắt trong ảnh, destination là vùng đích.
Nếu world, trừ camera và round tọa độ đích để ảnh pixel không rơi giữa pixel.
Đặt alpha rồi SDL_RenderTextureRotated với góc 0, có thể flip ngang.
Mỗi lần gọi đều đặt alpha nên không vô tình giữ độ mờ của lần trước.

Ví dụ vật ở (600,80) và camera=(400,20) được vẽ ở (200,60)
trong viewport hiện tại. world=false dành cho phần đã ở tọa độ màn hình như nền/HUD.

## character(...)

Chọn state: trên nền và |vx|>5 là run=1, đứng là idle=0;
trên không và vy<0 là jump=2, còn lại fall=3. AtExit ép về idle.
Idle: 4 frame, tốc độ 6 frame/giây; run: 8 frame, 12 frame/giây;
jump/fall: frame 0. Lấy frame bằng phần nguyên của time*tốc độ rồi modulo.

Sprite enum tính từ player*4+state: player=0 là gấu mèo, 1 là chim.
Ô nguồn 32 × 32, đích 24 × 24 tại x=hitbox.x-6,
y=hitbox.y+hitbox.h-21 để căn phần chân trong vùng có padding.
facing<0 thì lật; AtExit alpha=140 thay vì 255. Số 1/2 vẽ phía trên đầu.

## terrain(x,y,top,left,right,theme)

x/y là chỉ số tile. Chọn vùng atlas theo theme: theme 3 dùng atlasX=192,
các theme khác 96; atlasY chọn 0/64/128. Cột trái/giữa/phải dựa vào cạnh lộ;
nếu cả left/right cùng true thì nhánh left ưu tiên. top chọn hàng mặt trên
hoặc thân. Đích luôn `(x*16,y*16,16,16)`.

## background(theme)

Chọn một trong bốn ảnh nền, lặp ô 64px phủ vùng nhìn.
Offset theo camera*0,2 tạo lớp nền di chuyển chậm. Các khối silhouette
dùng camera*0,45 tạo lớp gần hơn. Cả hai vẽ world=false vì đã tự tính parallax;
trừ camera thêm lần nữa sẽ làm nền cuộn sai.

## Debug và bài tập

Nếu hình đúng nhưng va chạm sai, xem Physics/hitbox; Renderer không tính nền.
Thử đổi tốc độ run từ 12 để thấy animation đổi mà tốc độ vật lý giữ nguyên.
Thêm ảnh cần đồng bộ [Assets](Assets.md).
[test_render_assets](../tests/test_render_assets.md) vẽ các state/theme,
kiểm tra viewport và đọc pixel đường chia; vẫn cần nhìn ảnh để đánh giá hình đẹp hay không.
