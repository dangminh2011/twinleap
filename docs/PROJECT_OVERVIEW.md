# Học mã nguồn Twinleap

Muốn đọc từng file riêng, mở [mục lục tài liệu .cpp](source/README.md):
14 trang mã game và 5 trang kiểm thử, có liên kết tới code và header tương ứng.

Tài liệu giải thích **code hiện có**: 14 file `.cpp`, năm màn chơi, hai nhân vật
và camera tự chia đôi. Mở các liên kết nguồn để đọc song song. Cách build và
điều khiển nằm trong [README](../README.md); cách thay WAV ở [AUDIO.md](AUDIO.md).

## 1. Thứ tự học

Bạn nên biết biến, hàm, struct/class, tham chiếu và vòng lặp C++ trước.
Chưa cần hiểu hết SDL để học phần vật lý.

| Lượt | File | Câu hỏi cần trả lời |
|---|---|---|
| 1 | Player.hpp, Player.cpp, Math.hpp | Nhân vật có dữ liệu gì? Hitbox là gì? |
| 2 | Level.hpp, Level.cpp | TXT trở thành màn chơi thế nào? |
| 3 | Physics.hpp, Physics.cpp | Nhân vật chạy, nhảy và đứng trên nền thế nào? |
| 4 | Entities.cpp, PlayState.cpp, Camera.cpp | Hai người và các đối tượng phối hợp ra sao? |
| 5 | main.cpp, App.cpp, Input.cpp, MenuState.cpp | Ai gọi các phần trên, vào lúc nào? |
| 6 | Assets.cpp, Renderer.cpp, Particles.cpp, Sfx.cpp | Dữ liệu trở thành hình và tiếng thế nào? |

`include/twinleap/` chứa `.hpp`: khai báo dữ liệu và giao diện hàm.
`src/` chứa `.cpp`: cài đặt hàm, với cấu trúc thư mục con tương ứng.
`main.cpp` là điểm vào riêng. Math, Time, GameState và Palette là header-only,
chỉ chứa kiểu, hằng số hoặc hàm ngắn nên không cần `.cpp`.

## 2. Luồng chạy chung

```text
main.cpp
  -> App::run()
       -> khởi tạo SDL, cửa sổ, renderer, input, audio
       -> tìm và sắp xếp assets/maps/*.txt
       -> vòng lặp:
            sự kiện -> Input::event(), Input::poll()
            xử lý menu / yêu cầu chuyển màn hình
            đang chơi -> PlayState::update() theo bước 1/120 giây
            PlayState::render() hoặc nền menu
            MenuState::render() -> SDL_RenderPresent()
            Sfx::update()
       -> giải phóng tài nguyên -> SDL_Quit()
```

`App` quyết định màn hình hiện tại; `PlayState` giữ phiên chơi; `Physics`
thay đổi vị trí/vận tốc; `Renderer` thể hiện dữ liệu lên màn hình.
Hiện game dùng enum `Screen`, không dùng cây lớp State ảo trong đặc tả gốc.

**Frame** là một lượt hiển thị, còn **update** là một bước mô phỏng cố định.
Một frame có thể có 0, 1 hoặc nhiều update. `Time.hpp` giữ thời gian chưa dùng
trong accumulator. Frame dài khoảng 1/60 giây thường cần hai update 1/120 giây.
App giới hạn thời gian cộng vào ở 0,25 giây để tránh chạy bù quá nhiều.
Menu/minimize reset accumulator; pause không cập nhật gameplay.

## 3. Dữ liệu và hệ tọa độ

| Tên | Ý nghĩa |
|---|---|
| `Vec2` | Cặp x/y dùng cho vị trí hoặc vận tốc |
| `Rect` | Hình chữ nhật x/y/w/h dùng kiểm tra giao nhau |
| `Player::pos` | Góc trên trái hitbox trong thế giới |
| `Player::previous` | Vị trí trước bước vật lý, dùng va chạm/carry |
| `Player::vel` | Vận tốc, pixel logic/giây |
| `Player::onGround` | Có điểm tựa dưới chân |
| `Object::kind` | Ký tự phân loại đối tượng |
| `Object::active` | Xu đã nhặt, cờ đã bật, nút đang giữ hoặc cửa đang mở |

Một tile là 16 × 16 pixel. Màn hình logic 480 × 270 được phóng lên cửa sổ
1280 × 720 và giữ tỉ lệ. Gốc ở góc trên trái, Y tăng xuống dưới:
`vel.y < 0` là bay lên; `vel.y > 0` là rơi.
`overlaps()` trong [Math.hpp](../include/twinleap/core/Math.hpp) kiểm tra
hai hình chữ nhật giao nhau theo cạnh, gọi là AABB.

## 4. Player.cpp: dữ liệu nhân vật

Nguồn: [Player.cpp](../src/game/Player.cpp), [Player.hpp](../include/twinleap/game/Player.hpp).

File chỉ cài đặt `Player::bounds()`, trả `{pos.x, pos.y, 12, 14}`.
Nó nhỏ vì chuyển động nằm trong Physics. Giữ file để cấu trúc header/nguồn
nhất quán và có nơi định nghĩa hitbox rõ ràng.

Sprite vẽ 24 × 24 nhưng hitbox là 12 × 14. Ảnh lớn hơn không làm va chạm sớm hơn.
Nếu đổi hitbox phải rà cả Physics/PlayState: code có các số 12/14 dùng trực tiếp,
không chỉ gọi bounds().

Player có ba trạng thái: Alive, Dead, AtExit. Các timer phục vụ coyote,
jump buffer và hồi sinh. `PlayerInput` cũng nằm trong header này để Physics
không phải include SDL. `moveX` từ -1 đến 1; jumpPressed là vừa bấm,
jumpHeld là đang giữ, jumpReleased là vừa nhả. Physics hiện dùng pressed/released
cho nhảy; không có giảm trọng lực ở đỉnh theo jumpHeld.

## 5. Level.cpp: đọc màn chơi

Nguồn: [Level.cpp](../src/game/Level.cpp), [Level.hpp](../include/twinleap/game/Level.hpp).

`Level::load(path, error)` trả true khi thành công; nếu lỗi trả false và
điền thông báo kèm đường dẫn, số dòng đang đọc vào error.

1. Xóa dữ liệu cũ bằng `*this = Level{}`, mở file.
2. Trước `--`: đọc name/par, bỏ dòng trống và chú thích metadata.
3. Sau `--`: kiểm tra ký tự rồi lưu dòng vào tiles.
4. Gặp P/Q lưu spawn, E lưu vùng exit; o/C/a/b/A/B thêm vào objects.
5. Kiểm tra đúng một P, Q, E; nhóm nút/cửa có đủ hai phía.
6. Bù dấu chấm bên phải dòng ngắn cho bằng chiều rộng lớn nhất.

Ví dụ P ở cột 3, hàng map 2 tạo spawn `(3*16+2, 2*16+2) = (50,34)`;
hàng/cột tính từ 0. Cộng 2 giúp hitbox nằm trong tile.

`tileAt(x,y)` nhận **chỉ số tile**, trả ký tự. Trái/phải/trên ngoài biên là #;
phía dưới là dấu chấm để nhân vật có thể rơi. `isSolidAt()` còn xét cửa A/B
đang đóng, nên Physics dùng hàm này để nhận biết vật cản.

Parser xác nhận định dạng, chưa chứng minh màn giải được. parTime hiện chỉ
là dữ liệu tham khảo, không phải giới hạn thua.

## 6. Physics.cpp: một bước chạy và nhảy

Nguồn: [Physics.cpp](../src/game/Physics.cpp), hằng số: [Physics.hpp](../include/twinleap/game/Physics.hpp).

Hàm chính `Physics::step(p, in, level, other, dt)` sửa trực tiếp p qua tham chiếu.
Input và level chỉ đọc; other là con trỏ đồng đội, có thể nullptr trong test.
Hàm không gọi SDL nhưng có sửa p, nên không phải hàm thuần theo nghĩa toán học.

Đọc thân hàm theo từng khối:

1. Bỏ qua Dead/AtExit, lưu previous.
2. Đang đứng trên đầu đồng đội thì cộng dịch chuyển ngang của người dưới nếu
   không đụng solid; người dưới nhảy có thể truyền vận tốc lên cho người trên.
3. Cập nhật coyoteTimer/jumpBufferTimer; cả hai còn thời gian thì nhảy.
4. Nhả nhảy khi đang bay lên thì nhân vel.y với 0,45.
5. `approach()` đưa vel.x dần tới moveX * RUN_SPEED, với gia tốc/ma sát tùy nền/không khí.
6. Cộng trọng lực và giới hạn tốc độ rơi ở 420.
7. Di chuyển X và xử lý va chạm, sau đó làm tương tự với Y.
8. Khi rơi thì xét nền một chiều/đầu đồng đội; dò thêm 1 pixel dưới chân cho solid.

Công thức chính: `vel.y += GRAVITY * dt`, `pos.y += vel.y * dt`.
Với dt=1/120, trọng lực 1100 tăng vel.y khoảng 9,17 pixel/giây mỗi bước.
Nhảy đặt vel.y=-340, đưa nhân vật lên trước khi trọng lực kéo xuống.

Coyote time 0,09 giây cho phép nhảy ngay sau khi rời mép. Jump buffer 0,10 giây
giữ yêu cầu bấm trước khi đáp; yêu cầu có thể được thực hiện ở update kế tiếp
sau khi onGround đã bật. Nhả sớm làm vận tốc hướng lên bớt âm, nên nhảy thấp hơn.

Hàm phụ `solid()` chỉ duyệt tile mà hitbox phủ lên. Nếu vị trí mới bị chặn,
code quay lại rồi tiến từng 0,25 pixel tới sát vật cản. Đây là xử lý sau khi
phát hiện overlap, không quét toàn bộ quãng đường; fixed timestep và giới hạn
vận tốc giữ dịch chuyển bình thường đủ nhỏ.

Nền `=` chỉ chặn khi đang rơi và chân cũ chưa vượt mặt nền. Nhảy từ dưới
thì xuyên qua. Hai nhân vật không chặn nhau theo chiều ngang.

## 7. Entities.cpp: nút và cửa

Nguồn: [Entities.cpp](../src/game/Entities.cpp).

`updateDoors(level, players, dt)` sửa nút/cửa và trả có cửa nào đổi trạng thái
hay không để PlayState phát âm thanh. Hàm chạy hai lượt:

1. Nút a/b được giữ khi người chưa chết, onGround và chân giao vùng nút.
2. Cập nhật cửa A/B theo nhóm nút, timer và người bên trong.

Nhiều nút cùng nhóm dùng OR: chỉ cần một nút giữ là mở cả nhóm.
Đang giữ thì timer cửa đặt 0,2 giây; nhả thì giảm dần đến 0 mới đóng.
Cửa đang mở còn người trong sẽ giữ mở để tránh nhốt người trong vật cản.
Xu, checkpoint và exit do PlayState xử lý.

## 8. PlayState.cpp: điều phối phiên chơi

Nguồn: [PlayState.cpp](../src/game/PlayState.cpp), [PlayState.hpp](../include/twinleap/game/PlayState.hpp).

`load()` gọi parser, chọn theme theo tên file, đặt hai người ở spawn, reset
điểm/deaths/time, checkpoint, particles, camera, splitView. Màn 02/03/04
có theme riêng; 01 và 05 dùng mặc định.

`update()` ghép các module theo thứ tự sau:

1. Đã complete thì dừng; chưa thì tăng time, giảm shake.
2. Chọn người có Y lớn hơn để cập nhật trước, giúp người trên có dịch chuyển mới của người dưới.
3. Người chết giảm respawnTimer; hết giờ tạo Player tại checkpoint chung hoặc
   spawn riêng. Nếu trùng đồng đội thì dịch ngang 16 pixel.
4. Người chưa chết gọi Physics; tạo bụi khi đáp, âm khi phát hiện nhảy từ nền.
5. Kiểm tra gai/hố; chết thì tăng deaths, chờ một giây, tạo hạt và rung hình.
6. Giao xu chưa nhặt thì active=true, cộng 100 điểm và phát hiệu ứng.
   Giao checkpoint mới thì đổi cờ và điểm hồi sinh dùng chung.
7. Chạm exit thì AtExit, vel=0. Physics bỏ qua trạng thái này.
8. Cập nhật cửa; cả hai AtExit thì complete và phát âm thắng.
9. Cập nhật camera, chia/gộp vùng nhìn, giới hạn ngang và particles.

`renderWorld()` vẽ nền, tile, đối tượng, exit, người chơi, debug và hạt.
Rung hình chỉ cộng vào camera render, không đổi vị trí vật lý.
`render()` chọn một hoặc hai viewport rồi gọi renderWorld, cuối cùng vẽ HUD.
Khi chia màn phải `viewport(nullptr)` trước HUD để trở lại toàn màn hình.

PlayState hiện chứa cả điều phối gameplay và trình bày. Khi đọc file dài này,
tìm từng hàm trước rồi theo các khối ở trên; logic thấp hơn đã tách ra Physics/Level/Entities.

## 9. Camera.cpp: góc nhìn chung và chia màn

Nguồn: [Camera.cpp](../src/game/Camera.cpp).

`Camera::update()` lấy trung điểm tâm người chưa chết, gồm AtExit, rồi trừ
nửa màn hình để ra góc trên trái camera. Map nhỏ căn giữa, map lớn kẹp trong biên.
Cả hai chết thì giữ nguyên. Độ mượt `1-exp(-8*dt)` giúp camera tiến dần tới mục tiêu.
Vật ở X=600, camera X=400 được vẽ tại X=200.

Chia màn do **PlayState**, không nằm trong Camera.cpp:

| Điều kiện khi cả hai chưa chết | Kết quả |
|---|---|
| Chưa chia, cách ngang >400 hoặc dọc >180 pixel | Bật chia |
| Đã chia, còn cách ngang >320 hoặc dọc >140 | Giữ chia |
| Đã chia, cả hai khoảng cách về trong ngưỡng gộp | Gộp |

Hai ngưỡng tránh nhấp nháy ở ranh giới. Mỗi viewport 240 × 240 nằm dưới HUD:
P1 trái, P2 phải. Khi dùng camera chung, PlayState còn chỉnh camera để bắt kịp
hồi sinh và hạn chế đi ngang ra khỏi vùng nhìn. Chế độ chia bỏ kẹp ngang đó.

## 10. main.cpp và App.cpp: vòng đời chương trình

Nguồn: [main.cpp](../src/main.cpp), [App.cpp](../src/core/App.cpp).

main tạo App và trả kết quả từ run(argc,argv). SDL_main.h chỉ nằm ở điểm vào
để hỗ trợ entry point theo nền tảng.

Trong App, screen là màn hình hiện tại, requested là yêu cầu chuyển tới.
Gán screen=requested sau phần xử lý giúp gom thời điểm chuyển trạng thái.
MenuState vẽ UI; hành động Start/Select/Resume/Restart/Next/Quit nằm trong App.
Lambda load dùng current để nạp file, yêu cầu Play hoặc Error và consume input.

Map được tìm lần lượt tại ./assets/maps, cạnh executable, rồi ở thư mục cha
executable; dùng thư mục đầu tiên có TXT. Danh sách được sắp theo tên file,
không viết cứng số màn trong App.

Renderer, Input và Sfx nằm trong một scope bên trong run. Ra khỏi scope,
destructor hủy texture/gamepad/audio trước khi App hủy SDL renderer, window
và SDL_Quit. Texture phải hủy trước renderer mà nó thuộc về.

Tham số --smoke bỏ menu, nạp màn đầu và thoát sau 31 frame để kiểm tra startup;
nó không tự chơi qua màn.

## 11. Input.cpp: biến phím thành ý định

Nguồn: [Input.cpp](../src/core/Input.cpp).

`event()` nhận SDL_Event, lưu cạnh bấm/nhả nhảy, điều khiển menu/pause;
cắm gamepad mở slot trống, rút thì đóng và xóa con trỏ. Destructor đóng các pad còn giữ.

`poll()` đọc bàn phím và tay cầm hiện tại; cộng nguồn di chuyển rồi kẹp [-1,1].
Stick có deadzone 0,25. Bàn phím vẫn dùng cùng gamepad. held lưu trạng thái
nhảy ở lần poll trước để nhận ra thay đổi.

`consume()` xóa cạnh nhấn/nhả và lệnh menu sau khi dùng, giữ moveX/jumpHeld.
Frame chưa đủ thời gian update thì giữ cạnh nhấn lại. Frame có nhiều update
chỉ xử lý cạnh nhảy ở lượt đầu. Vì vậy không reset input ngay sau mỗi frame.

## 12. MenuState.cpp: vẽ giao diện

Nguồn: [MenuState.cpp](../src/game/MenuState.cpp).

`render()` nhận Screen, dòng chọn, danh sách file và game. Lambda menu vẽ
nhãn cách nhau 18 pixel và dấu >. Các nhánh vẽ Menu, Select, Controls, Pause,
Complete, Congrats, Error. Screen::Play không cần menu phủ lên.
Đổi chữ/vị trí ở đây; đổi hành động khi chọn thì đọc App.cpp.

## 13. Assets.cpp và Renderer.cpp: từ BMP tới hình

Nguồn: [Assets.cpp](../src/render/Assets.cpp), [Renderer.cpp](../src/render/Renderer.cpp).

Assets::load tìm thư mục assets, đọc 14 BMP thành SDL_Surface, đặt magenta
RGB(255,0,255) làm trong suốt, tạo SDL_Texture rồi hủy surface tạm.
Texture dùng nearest filtering để giữ pixel sắc nét, blend để vẽ mờ.
get(Sprite) trả texture; destructor hủy texture đã nạp.
Thứ tự enum Sprite trong Assets.hpp phải khớp danh sách đường dẫn Assets.cpp:
Renderer chọn animation bằng chỉ số player*4+state.

| Hàm Renderer | Trách nhiệm |
|---|---|
| check | Log lỗi SDL, bật failed để App kết thúc |
| rect, line | Vẽ hình/đường; tọa độ world được trừ camera |
| text, centered | Chữ theo màn hình; căn giữa dựa font 8 pixel/ký tự |
| sprite | Cắt vùng source của atlas, vẽ tới destination; hỗ trợ lật/mờ |
| character | Chọn idle/run/jump/fall theo vận tốc, nền và AtExit |
| terrain | Chọn ô atlas theo theme và cạnh tile bị lộ |
| background | Lặp nền và cuộn chậm theo camera tạo chiều sâu |
| viewport | Chọn vùng vẽ khi chia đôi |

Idle có 4 frame ở 6 frame/giây, run 8 frame ở 12 frame/giây;
jump/fall một frame. Ô nguồn 32 × 32 vẽ 24 × 24, căn chân theo hitbox.
world=false giúp HUD/nền không bị trừ camera lần nữa.

## 14. Particles.cpp: hiệu ứng hạt

Nguồn: [Particles.cpp](../src/game/Particles.cpp).

burst tìm slot hết tuổi thọ trong pool cố định, gán vị trí/vận tốc/màu và
life=0,6 giây. Bộ sinh số giả ngẫu nhiên nằm trong đối tượng, không dùng global.
Pool đầy thì bỏ phần hạt còn thiếu. update giảm life, cộng trọng lực, di chuyển;
render vẽ hạt sống thành ô 2 × 2. Tái dùng slot tránh cấp phát mỗi lần nổ hạt.

## 15. Sfx.cpp: nạp và trộn tiếng

Nguồn: [Sfx.cpp](../src/audio/Sfx.cpp), asset: [AUDIO.md](AUDIO.md).

loadSample tìm WAV, đọc bằng SDL_LoadWAV, chuyển mono float32 44100 Hz, nhân gain
rồi giữ trong vector. Buffer SDL tạm được SDL_free. Constructor nạp năm SFX,
một nhạc nền và mở/resume audio stream. Không có thiết bị thì game vẫn chạy.

play(Sound) chọn một trong tám voice trống và đặt vị trí đọc về 0; nó chưa
queue cả WAV ngay. Hết voice thì thay voice đầu. update trộn voice với nhạc
vào buffer 512 float, kẹp [-1,1] rồi queue sang SDL khi còn dưới 4096 byte chờ.
Nhạc hết mẫu thì quay về đầu. Gain SFX 0,35; nhạc 0,12.

toggle đổi mute và gọi silence để xóa SFX/queue. setPaused dừng/tiếp tục thiết bị.
silence không đặt vị trí nhạc về đầu. Destructor hủy stream.
Không đọc file âm thanh trong vòng lặp update/render.

## 16. Theo dõi một lần nhảy bằng debugger

Build Debug trong Visual Studio, đặt breakpoint tại Physics::step rồi bấm W:

1. Input::event/poll đặt jumpPressed=true.
2. Physics đặt buffer=0,10; nếu có nền thì coyote=0,09.
3. Khi nhảy, vel.y=-340 trước cộng trọng lực, onGround=false.
4. pos.y giảm dần; nhả W làm vận tốc lên nhân 0,45.
5. Rơi chạm nền: vel.y=0, onGround=true.
6. Renderer::character chọn hình theo trạng thái vừa tính.

Nhặt xu cũng có chuỗi rõ ràng: overlaps -> active=true -> score+100 -> particles
và Sound::Coin -> render bỏ xu đã active. Theo một sự kiện sẽ dễ học hơn
đọc toàn bộ file trong một lượt.

## 17. Bài tập nhỏ và test

| Thử thay đổi | Chỗ sửa | Quan sát |
|---|---|---|
| Chạy nhanh hơn | RUN_SPEED trong Physics.hpp | Chạy trên nền |
| Cửa chờ lâu hơn | Timer 0,2 trong Entities.cpp | Nhả nút, quan sát đóng cửa |
| Thêm xu | Ký tự o trong assets/maps/*.txt | Build, nhặt và xem điểm |
| Đổi chữ pause | MenuState.cpp | Esc khi chơi |
| Đổi tốc độ animation | Renderer::character | So sánh chạy/đứng |

Mỗi lần đổi một việc, build và test lại. Test có thể kiểm tra giá trị cũ;
đọc lỗi để phân biệt hồi quy với yêu cầu bạn chủ động thay đổi.

| Test | Nội dung |
|---|---|
| [test_level_parser.cpp](../tests/test_level_parser.cpp) | Dữ liệu hợp lệ/lỗi, quét tất cả map |
| [test_physics.cpp](../tests/test_physics.cpp) | Nền, nhảy, coyote/buffer, one-way, carry |
| [test_buttons_doors.cpp](../tests/test_buttons_doors.cpp) | Nút, trễ cửa, an toàn khi đóng |
| [test_gameplay.cpp](../tests/test_gameplay.cpp) | Exit, hồi sinh, checkpoint, audio dummy |
| [test_render_assets.cpp](../tests/test_render_assets.cpp) | Sprite/theme, software renderer, chia/gộp màn |

Chạy `.\build-windows.cmd` để build và chạy cả năm test. Test render tạo
render-preview-*.bmp trong build-windows; có thể xóa, lần test sau sẽ tạo lại.

## 18. Thư mục cần giữ và thư mục tự sinh

| Đường dẫn | Vai trò |
|---|---|
| src/, include/ | Code cần học và sửa |
| assets/ | 14 BMP, 5 map, 5 SFX WAV, 1 WAV nhạc, thông tin nguồn/giấy phép |
| tests/ | Code kiểm thử, cần giữ để phát hiện lỗi khi sửa |
| docs/, README.md | Hướng dẫn học, build và dùng game |
| CMakeLists.txt, CMakePresets.json | Cấu hình build, preset Visual Studio |
| build-windows.cmd/.ps1 | Launcher và script build Windows |
| .deps/ | SDL3 cục bộ, giữ để không cần tải lại |
| build-windows/ | Build hiện tại, tái tạo bằng script |
| out/build/ | Visual Studio tạo khi dùng preset |
| .vs/ | Trạng thái IDE và cache có thể tạo lại |

Sửa bản gốc trong src/include/assets rồi build; không sửa bản copy cạnh executable
hoặc code SDL trong _deps. Xem [quyết định cấu trúc và dọn dự án](DECISIONS.md).
