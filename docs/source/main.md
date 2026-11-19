# main.cpp — điểm bắt đầu chương trình

[Mã nguồn](../../src/main.cpp) · [App.hpp](../../include/twinleap/core/App.hpp) · [Mục lục](README.md)

## Tra cứu hàm và cách gọi

```cpp
int main(int argc, char **argv);
```

| Thành phần | Giải thích |
|---|---|
| Nơi gọi | Runtime của chương trình gọi khi khởi động; các module không tự gọi main |
| argc | Số chuỗi trong argv, gồm cả tên chương trình |
| argv | Mảng con trỏ chuỗi C; chỉ đọc argv[1] nếu argc>1 |
| Giá trị trả về | Mã int lấy từ App::run; 0 thành công, 1 lỗi do App báo |
| Tác động | Tạo App trên stack, gọi run và chờ phiên game kết thúc |

Hàm được dùng trong thân main là `app.run(argc, argv)`: phương thức của App,
nhận nguyên tham số dòng lệnh và thực hiện vòng đời game.
`twinleap::App app;` là khai báo/tạo đối tượng, không phải lời gọi một hàm tên app.

Ví dụ chạy `twinleap.exe --smoke`: argc thường bằng 2, argv[1] là "--smoke".
Đọc [App::run](core/App.md) để theo tiếp lời gọi này.

## Vai trò

Đây là điểm vào của executable twinleap. File tạo một App và giao toàn bộ
vòng đời chương trình cho App::run. Không có vòng lặp gameplay tại đây.

## Hàm main

```cpp
int main(int argc, char **argv)
{
    twinleap::App app;
    return app.run(argc, argv);
}
```

`argc` là số phần tử của danh sách tham số; `argv` trỏ tới các chuỗi tham số.
`argv[0]` thường là tên/đường dẫn chương trình. Khi chạy `twinleap --smoke`,
App kiểm tra `argv[1]` để bật chế độ chạy thử ngắn.

`app` có thời gian sống trong main. Khi run kết thúc, giá trị trả về trở thành
mã thoát tiến trình: 0 thành công, 1 khi App phát hiện lỗi startup/render/map.

## Vì sao include SDL_main.h?

SDL_main.h hỗ trợ điểm vào phù hợp nền tảng, trong đó có ứng dụng GUI Windows.
Chỉ file chứa main cần include header này. App.hpp là giao diện project
được include trước theo quy ước của dự án.

## Theo dõi và thử học

Đặt breakpoint tại dòng tạo App, Step Into lời gọi run để tới
[App.cpp](core/App.md). Muốn học xử lý tham số mới thì xem phần đầu run;
muốn sửa nhảy hoặc vẽ thì tìm module tương ứng, không thêm vào main.

Để thử entry point, build và chạy game bình thường hoặc thêm --smoke.
Smoke nạp màn đầu rồi thoát sau 31 frame, không chứng minh giải được màn.
