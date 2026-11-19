# test_gameplay.cpp — kiểm tra nhiều module kết hợp

[Mã nguồn](../../../tests/test_gameplay.cpp) · [PlayState](../game/PlayState.md) · [Mục lục](../README.md)

## Tra cứu hàm và lời gọi sử dụng

`int main(int argc,char **argv)` nhận thư mục map qua argv[1], trả 0 khi đạt,
1 khi CHECK thất bại. Các hàm project khác được gọi như sau:

| Lời gọi | Tham số / kết quả / dữ liệu thay đổi |
|---|---|
| `Sfx sfx;` | Tạo đối tượng, tự gọi constructor nạp audio; destructor tự gọi khi hết scope |
| `sfx.hasMusic()` / `sfx.hasSound(Sound(i))` | bool có dữ liệu mẫu; không chứng minh thiết bị loa hoạt động |
| `game.load(path,error)` | bool; nạp/reset phiên hoặc ghi error khi lỗi |
| `game.update(inputs,1.f/120,sfx)` | void; mô phỏng và sửa gameplay/audio request |
| `sfx.play(Sound(i))` / `sfx.update()` | void; chọn voice / trộn và queue dữ liệu |
| `sfx.toggle()` / `sfx.silence()` | void; đổi mute / xóa voice và queue |
| `sfx.setPaused(true/false)` | void; đổi trạng thái pause thiết bị |

`SDL_SetHint(SDL_HINT_AUDIO_DRIVER,"dummy")` trả bool, yêu cầu driver giả;
`SDL_Init(0)` trả bool, khởi tạo SDL không bật video;
`SDL_Quit()` trả void, dọn SDL sau khi Sfx đã hủy.
`std::string(argv[1])` tạo chuỗi C++ để ghép tên map.
CHECK là macro return 1 từ main khi điều kiện sai.

Ví dụ `game.players[0].pos.y=height*16+65; game.update(...)`
đặt người dưới hố rồi cho logic phát hiện chết; riêng phép gán vị trí chưa tăng deaths.

## Khởi tạo

Nhận thư mục map ở argv[1]. Đặt audio driver dummy trước SDL_Init(0),
tạo Sfx/PlayState trong scope rồi SDL_Quit sau khi hủy chúng.
Dummy cho phép chạy không phát ra loa thật. CHECK sai in dòng/biểu thức và trả 1.

## Các nhóm kiểm tra trong main

| Nhóm | Hành động và kỳ vọng |
|---|---|
| Audio asset | hasMusic và năm hasSound đều true |
| Exit | Một người ở cửa chưa complete; cả hai thì complete |
| Chết/hồi sinh | Đặt P1 dưới đáy+65; deaths=1/Dead; chờ đủ bước thì Alive |
| Checkpoint 03 | P1 chạm cờ, P2 chết; sau hồi sinh P2 ở vùng checkpoint |
| Camera lệch khi respawn | Màn 05, camera ở xa; P1 vẫn hồi sinh đúng spawn |
| Từng checkpoint 05 | Kích hoạt, hồi sinh người kia; lệch 16px khi trùng và không chết sau đó |
| Audio điều khiển | play/update, mute/unmute, silence, pause/resume |

Test đặt trực tiếp vị trí người để cô lập tình huống, không điều khiển từ spawn
tới exit. Các vòng 122/125/180 bước dùng dt=1/120 cho thời gian xác định.
Kiểm tra checkpoint 03 chỉ xác nhận vùng X, không so đủ tọa độ chính xác.

## Vì sao chép mapObjects?

Trước vòng lặp checkpoint 05, danh sách objects được sao chép.
Bên trong có game.load làm thay vector gốc. Duyệt bản copy tránh tham chiếu
vào dữ liệu đang bị reset và thử mỗi cờ độc lập.

## Chạy và giới hạn

```powershell
ctest --test-dir build-windows -R '^gameplay$' --output-on-failure
```

Đặt breakpoint trước update để xem trạng thái rồi theo vào PlayState/Physics.
Test không kiểm chứng chơi hết năm màn; dummy không xác nhận âm lượng,
waveform hoặc chất lượng nghe trên loa. Phần cuối chủ yếu kiểm tra các lệnh
audio chạy qua, không so sánh tín hiệu đầu ra.
