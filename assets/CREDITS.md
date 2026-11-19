# Nguồn hình ảnh và âm thanh

- P1 gấu mèo: tách từ RACCOONSPRITESHEET.png do người dùng cung cấp.
- P2 chim xanh: tách từ BIRDSPRITESHEET_Blue.gif do người dùng cung cấp.
- Nền Blue/Green/Purple/Brown, terrain và spikes: từ Pixel Adventure 1
  của Pixel Frog, do người dùng cung cấp.

Giữ 14 BMP, năm map, năm WAV hiệu ứng và một WAV nhạc nền. ZIP và bộ PNG nguồn dư đã được loại bỏ.
Tác giả hai file nhân vật riêng không được ghi trong file.

Strip nhân vật dùng ô 32×32: idle 4 frame, run 8 frame, jump/fall 1 frame.
BMP dùng màu RGB (255, 0, 255) cho vùng trong suốt, xử lý bằng SDL3 color key.

## Âm thanh

- coin.wav lấy đoạn 0,25–1,00 giây của getcoin.mp3; nhạc nền
  music/background.wav lấy toàn bộ bgsong.mp3. Cả hai do người dùng cung cấp;
  chưa xác định tác giả/giấy phép từ file. Giấy phép CC0 Kenney chỉ áp dụng
  jump/death/door/win còn dùng.

- **Digital Audio** — Kenney Vleugels (Kenney.nl), từ `kenney_digital-audio.zip`
  do người dùng cung cấp. Giấy phép **Creative Commons Zero (CC0)**;
  giữ bản gốc tại [audio/LICENSE-Kenney.txt](audio/LICENSE-Kenney.txt).
- Chọn phaseJump1, phaserDown1, lowDown và powerUp1; chuyển từ OGG
  thành jump/death/door/win.wav (PCM 16-bit, mono 44100 Hz).
- Ánh xạ chi tiết: [audio/sfx/README.md](audio/sfx/README.md).
  Các WAV được game nạp và phát qua module Sfx.
