# Nhạc nền

background.wav chuyển từ bgsong.mp3 người dùng cung cấp, WAV PCM 16-bit mono
44100 Hz, khoảng 189,47 giây. Game nạp một lần và trộn cùng SFX qua SDL3.
Nhạc lặp ở menu và khi chơi, tiếp tục qua các màn. M bật/tắt nhạc và SFX;
pause/minimize tạm ngừng, quay lại tiếp tục từ vị trí hiện tại.
Gain nhạc 0,12, SFX 0,35. Thay WAV cùng tên, build rồi mở lại game để đổi nhạc.
WAV chiếm khoảng 16 MiB trên đĩa và khoảng 32 MiB RAM dạng float32.
Nguồn/tác giả/giấy phép chưa xác định từ file gửi; không thuộc CC0 Kenney.