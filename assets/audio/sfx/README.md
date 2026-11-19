# Hiệu ứng âm thanh

Bốn hiệu ứng từ **Kenney Digital Audio**, riêng coin thay bằng file người dùng cung cấp:

| File asset | File gốc trong ZIP | Mục đích | Độ dài |
|---|---|---|---|
| `jump.wav` | `Audio/phaseJump1.ogg` | Nhảy, hiệu ứng chuyển cao độ | 0,47 giây |
| `coin.wav` | `getcoin.mp3` (0,25–1,00 giây) | Nhặt xu | 0,75 giây |
| `death.wav` | `Audio/phaserDown1.ogg` | Chết/rơi, hiệu ứng hạ cao độ | 0,47 giây |
| `door.wav` | `Audio/lowDown.ogg` | Cửa đổi trạng thái, âm trầm | 0,78 giây |
| `win.wav` | `Audio/powerUp1.ogg` | Hoàn thành màn, hiệu ứng power-up | 1,20 giây |

Đã chuyển đổi thật sự từ OGG/MP3 sang **WAV PCM signed 16-bit, mono 44100 Hz**
bằng FFmpeg; không chỉ đổi đuôi. Coin được cắt ngắn; các âm khác giữ nguyên thời lượng, không chỉnh gain khi chuyển đổi.
FFmpeg không cần thiết khi build hoặc chạy game.

Bốn âm Kenney có giấy phép CC0 gốc: `../LICENSE-Kenney.txt`. Chỉ trích xuất năm âm trên;
đã bỏ ZIP nguồn, preview, liên kết quảng bá và các biến thể không dùng.

**Trạng thái:** Sfx.cpp nạp năm WAV này một lần khi khởi động;
hướng dẫn thay file nằm trong `docs/AUDIO.md` ở thư mục gốc
dự án. Bộ trộn tự chuyển PCM sang float32 trước khi phát.
