# Quyết định triển khai hiện tại

Tài liệu ghi cấu trúc đang dùng. Các cách cũ như audio procedural, thư mục
levels và assets/runtime đã được thay thế. Đặc tả gốc giữ tại [AGENTS.md](../AGENTS.md).

## Công nghệ và build

- C++17, CMake >=3.21, chỉ SDL3 ngoài thư viện chuẩn.
- Windows x64 là nền tảng phát triển chính; giữ cấu hình Linux/macOS.
- CMake tìm SDL3 >=3.2, thiếu thì fetch tag release-3.2.0. Giữ SDK cục bộ
  .deps/SDL3-3.4.18 để build khi không có mạng.
- Script Windows dùng MSVC/Ninja, tạo build-windows. Preset Visual Studio tạo
  out/build/vs-x64-debug hoặc vs-x64-release. Không dùng chung cache khác cấu hình.
- Nguồn ngoài main.cpp thuộc twinleap_core; test link cùng thư viện.
  Header/source ở hai cây include/twinleap và src; Math/Time/GameState/Palette header-only.
- Windows mặc định chạy GUI; tắt TWINLEAP_WINDOWS_GUI để có console.
  CPack tạo ZIP kèm executable, DLL/runtime, assets và tài liệu.

## Gameplay và hiển thị

- Enum Screen và requested trong App điều khiển màn hình. MenuState vẽ UI;
  App xử lý hành động; PlayState giữ phiên chơi.
- PlayerInput nằm trong Player.hpp để Level/Player/Physics/Entities không cần SDL.
- Logic 1/120 giây, giữ hằng số vật lý gốc; không bật apex gravity tùy chọn.
  Va chạm tách trục, tiến lại từng 0,25 pixel khi vị trí mới chạm solid.
- Hai người xuyên ngang nhau, đứng đầu và carry được; người dưới cập nhật trước.
- Object dùng kind/active/timer. Nhóm cửa A/B, nhiều nút cùng nhóm dùng OR;
  trễ đóng 0,2 giây và giữ mở khi còn người bên trong.
- Điểm/deaths/time reset khi nạp/restart màn. Checkpoint chung; AtExit đứng yên.
- Camera chung theo trung điểm; PlayState chia màn khi xa hơn 400px ngang hoặc
  180px dọc, gộp khi về trong 320px/140px. Hai vùng 240 × 240 dưới HUD.
  Trả viewport đầy đủ trước khi vẽ HUD/menu.
- UI ASCII tiếng Anh phù hợp font debug; tài liệu tiếng Việt.

## Asset và âm thanh

- Map gốc trong assets/maps, gồm năm màn; App quét và sắp theo tên file.
  Màn 05 dài 192 × 18 tile, có bốn checkpoint, dùng theme mặc định.
- 14 BMP: P1 gấu mèo, P2 chim xanh, bốn nền, terrain và spikes.
  Cell 32 × 32 vẽ 24 × 24; hitbox 12 × 14; magenta color key, nearest filtering.
  Idle 4 frame, run 8 frame, jump/fall 1 frame.
- Assets sở hữu texture, hủy trước renderer. Thiếu hình là lỗi startup.
  CMake copy toàn bộ assets cạnh executable mỗi lần build; chỉ sửa bản gốc.
- Sfx nạp năm WAV và một WAV nhạc, chuyển mono float32 44100 Hz,
  trộn tám voice cùng nhạc lặp. Gain SFX 0,35, nhạc 0,12.
  M mute cả hai; pause/minimize dừng thiết bị. Audio lỗi không chặn gameplay.
- Bốn SFX dùng Kenney CC0; coin/nhạc do người dùng cung cấp. Giữ thông tin nguồn
  và LICENSE-Kenney.txt; xem [AUDIO.md](AUDIO.md) để thay WAV.

## Dọn dự án ngày 19/11/2026

- Giữ cả 14 file .cpp vì đều được CMake dùng. Player.cpp nhỏ vẫn giữ để đúng cấu trúc.
- Giữ năm test, asset đang nạp, SDL3 cục bộ và thiết lập IDE cá nhân.
- Dọn build/build-ninja cũ, các bản out/build có thể tái tạo, bản đóng gói/thư mục
  thử trong build-windows, ảnh preview/contact cũ và tools rỗng.
- Đã dọn cache PCH và CopilotIndices của Visual Studio; giữ cấu hình, layout và .wsuo.
  FileContentIndex và Browse.VC.db cùng file phụ đang bị khóa nên để lại.
  IDE sẽ tái tạo cache khi cần.
- Giữ build-windows làm bản build chính để chạy ngay. out/build được tạo lại khi dùng preset.
- Bổ sung ignore out/ và .vs/ để cache không lẫn vào danh sách nguồn Git/rg.
- Viết lại PROJECT_OVERVIEW theo thứ tự học; sửa thông tin cũ trong README/asset/audio,
  gồm số màn và việc MP3 nguồn không có trong cây dự án hiện tại.
- Trình bày lại Renderer::line và khối debug trong PlayState thành nhiều dòng để dễ đọc;
  giữ nguyên các phép tính và hành vi gameplay.

Build/test tự động không thay thế chơi thử hai người hoặc thử tay cầm/loa thật.
Các giới hạn kiểm chứng nằm trong Known Issues của README.

## Khôi phục build Visual Studio sau khi dọn

- Lần dọn đã xóa cả out/build là cache/bản build của Visual Studio, trong khi
  IDE còn lưu lựa chọn x64-Debug cũ. Vì vậy build qua script thành công chưa đủ
  để xác nhận phiên IDE đang mở có thể build ngay.
- Đã cấu hình và build lại preset vs-x64-debug từ đầu, chạy đủ năm test và smoke.
  Cập nhật CurrentProjectSetting cục bộ sang vs-x64-debug.
- Khi dọn tiếp, giữ thư mục build của cấu hình đang dùng. Nếu xóa cache,
  cần cấu hình lại đúng preset trước khi build/chạy trong IDE.
