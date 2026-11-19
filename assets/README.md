# Phân loại asset

```text
assets/
├── characters/
│   ├── raccoon/         # P1: idle.bmp, run.bmp, jump.bmp, fall.bmp
│   └── bird/            # P2: idle.bmp, run.bmp, jump.bmp, fall.bmp
├── backgrounds/         # blue.bmp, green.bmp, purple.bmp, brown.bmp
├── terrain/             # terrain.bmp: atlas các tile địa hình
├── traps/               # spikes.bmp: gai
├── maps/                # 5 file .txt định nghĩa màn chơi
├── audio/
│   ├── sfx/             # WAV hiệu ứng: jump, coin, death, door, win
│   └── music/           # background.wav phát lặp
├── README.md
└── CREDITS.md
```

Game đang dùng 14 BMP và 5 map. Có 5 WAV hiệu ứng (4 Kenney và coin người dùng cung cấp) trong `audio/sfx/`
(PCM 16-bit, mono 44100 Hz), kèm giấy phép CC0 trong `audio/LICENSE-Kenney.txt`.
Game nạp WAV một lần khi khởi động; Sfx.cpp chuyển mẫu sang float32 để trộn/phát.

BMP trong thư mục này là asset chính để chỉnh sửa/thay thế; không cần ZIP,
PNG nguồn, GIF hay công cụ chuyển đổi khi build. Chỉ giữ đúng file đang dùng.

Nhân vật: ô 32×32, idle 4 frame, run 8 frame, jump/fall 1 frame; các frame
xếp ngang. Giữ vùng trong suốt RGB (255, 0, 255) và vị trí chân để khớp
hitbox. Nếu thay kích thước/số frame, cần sửa `Renderer::character`.

Map: thêm `.txt` vào `maps/`, đặt tiền tố số để sắp thứ tự. Xem README chính
về ký hiệu map. Thêm hình ảnh mới cần khai báo đường dẫn trong `src/render/Assets.cpp`.

Build tự copy toàn bộ `assets/` cạnh executable và đưa vào ZIP.
Chỉ sửa asset trong thư mục mã nguồn rồi build lại; không sửa bản copy trong build.
Hướng dẫn thay WAV nằm ở `docs/AUDIO.md` trong thư mục dự án.
