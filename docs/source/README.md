# Tài liệu cho từng file .cpp

Mỗi trang tương ứng một file nguồn, có liên kết tới code và header để mở song song.
Các trang mô tả cách cài đặt hiện tại, gồm cả những giới hạn cần biết khi sửa.
Mỗi trang có mục **Tra cứu hàm**: chữ ký, ý nghĩa tham số, giá trị trả về,
dữ liệu bị thay đổi, nơi gọi và ví dụ. Bảng hàm SDL/STL giải thích các lời gọi
được sử dụng ngay trong file để bạn có thể theo dõi từ hàm lớn xuống hàm nhỏ.

Khi đọc chữ ký: `void` không trả dữ liệu, khác hàm trả bool báo thành công;
constructor/destructor không có kiểu trả về. `const` cuối phương thức nghĩa
không sửa đối tượng đó, nhưng hàm vẫn có thể sửa đối tượng nhận qua tham chiếu
khác (ví dụ Particles::render nhận Renderer&). Lambda là hàm cục bộ;
macro CHECK trong test là phép thay thế mã, không có vòng đời gọi như hàm C++.

## Mã game: 14 file

| File | Nội dung học |
|---|---|
| [main.cpp](main.md) | Điểm vào, tham số và mã thoát |
| [core/App.cpp](core/App.md) | Vòng đời SDL, vòng lặp, chuyển màn hình |
| [core/Input.cpp](core/Input.md) | Sự kiện, bàn phím/gamepad, giữ cạnh nhấn |
| [game/Player.cpp](game/Player.md) | Dữ liệu nhân vật và hitbox |
| [game/Level.cpp](game/Level.md) | Đọc TXT, kiểm tra lỗi và truy vấn tile |
| [game/Physics.cpp](game/Physics.md) | Gia tốc, nhảy, va chạm, đứng đầu và carry |
| [game/Entities.cpp](game/Entities.md) | Nút, timer và cửa |
| [game/PlayState.cpp](game/PlayState.md) | Gameplay, hồi sinh, checkpoint, exit và chia màn |
| [game/Camera.cpp](game/Camera.md) | Trung điểm, giới hạn map và làm mượt |
| [game/MenuState.cpp](game/MenuState.md) | Vẽ các màn UI |
| [game/Particles.cpp](game/Particles.md) | Pool và cập nhật hạt |
| [render/Assets.cpp](render/Assets.md) | BMP, surface, texture và quyền sở hữu |
| [render/Renderer.cpp](render/Renderer.md) | Tọa độ vẽ, atlas, animation và viewport |
| [audio/Sfx.cpp](audio/Sfx.md) | WAV, voice, trộn âm và queue SDL |

## Mã kiểm thử: 5 file

| File | Nội dung học |
|---|---|
| [test_physics.cpp](tests/test_physics.md) | Mô phỏng vật lý không mở cửa sổ |
| [test_level_parser.cpp](tests/test_level_parser.md) | Fixture và trường hợp parser lỗi |
| [test_buttons_doors.cpp](tests/test_buttons_doors.md) | Kiểm tra thay đổi trạng thái theo thời gian |
| [test_gameplay.cpp](tests/test_gameplay.md) | Kiểm tra nhiều module kết hợp |
| [test_render_assets.cpp](tests/test_render_assets.md) | Vẽ vào surface, đọc pixel và ảnh preview |

## Cách đọc

Bắt đầu Player → Level → Physics → Entities → PlayState → Camera.
Sau đó đọc main/App/Input/MenuState để hiểu ai gọi các phần này.
Cuối cùng đọc Assets/Renderer/Particles/Sfx.

Trong mỗi trang, đọc mục vai trò trước, rồi tìm đúng tên hàm trong file nguồn.
Ví dụ là tình huống học hoặc đoạn minh họa; chỉ các đoạn trong src/tests mới được build.
`dt` là giây mỗi bước; vị trí là pixel logic; X tăng sang phải, Y tăng xuống dưới.

Kiến thức C++ dùng xuyên suốt: `const T&` để đọc dữ liệu không sao chép,
`T&` để có thể sửa đối tượng gốc, `T*` là con trỏ có thể rỗng,
`{}` khởi tạo theo giá trị mặc định, destructor chạy khi đối tượng hết vòng đời.

Xem thêm [tổng quan](../PROJECT_OVERVIEW.md), [build/chơi](../../README.md)
và [quyết định triển khai](../DECISIONS.md).
