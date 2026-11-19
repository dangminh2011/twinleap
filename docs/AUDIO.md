# Âm thanh WAV trong Twinleap

Game phát năm WAV hiệu ứng và một WAV nhạc nền. Bốn SFX từ Kenney (CC0),
coin lấy từ getcoin.mp3 và nhạc nền từ bgsong.mp3 do người dùng cung cấp.
`Sfx.cpp` vẫn là module C++ nạp, chuyển đổi và trộn âm thanh; WAV chứa dữ liệu
âm thanh thực tế. Không dùng file WAV thay trực tiếp cho file mã nguồn.

## Các file đang dùng

| Sự kiện | Asset trong assets/audio/sfx/ | Lệnh trong PlayState.cpp |
|---|---|---|
| Nhảy | jump.wav | sfx.play(Sound::Jump) |
| Nhặt xu | coin.wav | sfx.play(Sound::Coin) |
| Chết | death.wav | sfx.play(Sound::Death) |
| Cửa đổi trạng thái | door.wav | sfx.play(Sound::Door) |
| Hai người cùng vào exit | win.wav | sfx.play(Sound::Win) |

Nguồn từng âm: [README của sfx](../assets/audio/sfx/README.md).
Giấy phép gốc: `assets/audio/LICENSE-Kenney.txt`.

## Thay âm thanh

1. Chuẩn bị WAV PCM, ưu tiên mono 44100 Hz, 16-bit. Phải chuyển đổi định dạng
   thật sự, không đổi đuôi OGG/MP3 thành WAV.
2. Ghi đè file tương ứng trong `assets/audio/sfx/`, giữ nguyên tên.
3. Build lại để copy asset cạnh executable, rồi mở lại game.

Không cần sửa code nếu giữ năm tên trên. Chỉ chỉnh asset trong thư mục nguồn,
không chỉnh bản copy trong `build-windows/` hoặc `out/build/`.

```powershell
.\build-windows.cmd -Run
```

Trong Visual Studio, chọn preset `vs-x64-debug`, build và chạy `twinleap.exe`.
Nếu phát hành ZIP, tạo lại gói bằng `.\build-windows.cmd -Package`.

## Cách hoạt động

Sfx tìm file ở `./assets/audio/sfx`, cạnh executable, rồi thư mục cha của
executable. WAV chỉ nạp một lần trong constructor bằng `SDL_LoadWAV`;
`SDL_ConvertAudioSamples` chuyển về mono float32 44100 Hz cho bộ trộn.
Gain là 0,35 để chừa khoảng âm lượng cho nhiều hiệu ứng đồng thời.

Module giữ tối đa tám giọng và gửi buffer ngắn tới SDL audio stream.
Không đọc file trong vòng lặp update/render. Phím **M** bật/tắt âm thanh;
pause dừng âm đang phát. Thiếu/hỏng WAV thì log và bỏ qua hiệu ứng đó.
Không mở được thiết bị audio thì game vẫn chạy không tiếng.

Không dùng SDL_mixer hoặc thư viện giải mã OGG. FFmpeg chỉ dùng một lần để
chuyển asset nguồn sang WAV, không cần khi build hoặc chạy game.

## Kiểm tra

CTest gameplay dùng audio dummy và xác nhận cả năm mẫu được nạp. Chạy game
để nghe thử nhảy, nhặt xu, chết, cửa, thắng màn, M và pause/resume. Dummy
không xác nhận chất lượng nghe hoặc âm lượng trên loa thực tế.

## Nhạc nền

`assets/audio/music/background.wav` được chuyển từ bgsong.mp3, dài khoảng
189,47 giây, phát lặp ở menu và khi chơi. Sfx nạp nhạc một lần, chuyển thành
float32, trộn cùng các hiệu ứng qua cùng audio stream với gain 0,12.
Khi tới cuối mẫu, con trỏ nhạc quay về đầu. M tắt/bật cả nhạc lẫn SFX;
pause hoặc minimize tạm ngừng thiết bị và tiếp tục khi quay lại.

Coin mới cắt đoạn 0,25–1,00 giây của getcoin.mp3 để tránh phát toàn bộ đoạn
12 giây mỗi lần nhặt xu. Hai MP3 nguồn không có trong cây dự án hiện tại;
game chỉ cần WAV đã chuyển đổi. Coin và nhạc không thuộc giấy phép CC0 của Kenney.

Thay background.wav cùng tên, build và mở lại game để đổi nhạc. WAV mono
chiếm khoảng 16 MiB trên đĩa và 32 MiB RAM ở dạng float32. Không thêm dependency.
