# test_render_assets.cpp — software renderer và ảnh kiểm tra

[Mã nguồn](../../../tests/test_render_assets.cpp) · [Renderer](../render/Renderer.md) · [Mục lục](../README.md)

## Tra cứu hàm và lời gọi sử dụng

`int main(int argc,char **argv)` nhận thư mục map, trả 0 nếu passed=true,
1 nếu có lỗi thiết lập/vẽ/kiểm tra. Không có helper project định nghĩa trong file;
test phối hợp các phương thức Renderer/PlayState/Sfx.

| Hàm SDL được gọi | Tham số / kết quả |
|---|---|
| `SDL_SetHint(...,"dummy")`, `SDL_Init(0)` | bool; chuẩn bị audio giả/SDL không mở video |
| `SDL_CreateSurface(480,270,RGBA32)` | Surface* hoặc nullptr; vùng RAM nhận ảnh |
| `SDL_CreateSoftwareRenderer(surface)` | Renderer* hoặc nullptr; vẽ vào surface |
| `SDL_RenderPresent(software)` | bool; hoàn tất lượt render cần kiểm tra |
| `SDL_SaveBMP(surface,path)` | bool; ghi ảnh ra file, false làm test thất bại |
| `SDL_GetRenderViewport(software,&viewport)` | bool; ghi Rect viewport hiện tại qua tham số đầu ra |
| `SDL_ReadSurfacePixel(surface,x,y,&r,&g,&b,&a)` | bool; ghi các kênh pixel đầu ra để test so màu |
| `SDL_GetError()` | Chuỗi lỗi SDL để in khi passed=false |
| `SDL_DestroyRenderer`, `SDL_DestroySurface`, `SDL_Quit` | void; giải phóng sau khi wrapper game ra khỏi scope |

| Hàm project/STL | Việc sử dụng |
|---|---|
| `renderer.assets.load(software)` | bool nạp đủ BMP thành texture |
| `game.load(path,error)` | bool nạp/reset map; false in error và dừng thử theme |
| `game.update({},0,sfx)` | void; input rỗng, không tiến thời gian nhưng vẫn tính layout |
| `game.render(renderer,debug)` | void; vẽ toàn bộ phiên theo layout hiện tại |
| `renderer.character(...)` | void; chủ động vẽ các tư thế để kiểm tra |
| `renderer.background()` / `MenuState::render(...)` | void; kiểm tra đường vẽ menu |
| `std::to_string(i+1)` | Đổi số thành chuỗi để tạo tên preview |
| `std::fprintf(stderr,...)` | Ghi lỗi; không mở hộp thoại tương tác trong test |

Ví dụ `SDL_ReadSurfacePixel(surface,239,100,&red,&green,&blue,&alpha)`
vừa trả trạng thái thành công vừa ghi bốn giá trị vào biến. Cần kiểm tra bool
trước khi xem các kênh; bool không phải màu pixel.

## Thiết lập

Nhận thư mục map ở argv[1], dùng audio dummy và SDL_Init(0).
Tạo surface RGBA32 480 × 270 và software renderer vẽ vào RAM.
Wrapper Renderer của game dùng cùng con trỏ software, không cần cửa sổ/GPU thật.
passed lưu kết quả; mã thoát 0 là đạt, 1 là lỗi.

## Theme và animation

Nạp assets, lần lượt map 01–04. Đặt time=0,25 để chọn frame xác định,
vẽ game có debug. Vẽ thêm hai người với bốn state idle/run/jump/fall,
hướng trái/phải, mờ tại exit. Kiểm tra failed/RenderPresent rồi lưu
render-preview-1.bmp tới render-preview-4.bmp.
Lệnh vẽ thành công chưa chứng minh sprite đẹp; ảnh để xem thủ công.

## Chia/gộp màn

Nạp màn 05, đặt hai người xa nhau, gọi update({},0,sfx) rồi render.
dt=0 không tiến thời gian nhưng vẫn chạy nhánh tính splitView.
Kiểm tra viewport đã về (0,0,480,270), pixel đường chia (239,100)
có RGB(20,23,43), lưu render-preview-split.bmp.

Đặt khoảng cách X=350 thì màn đã chia phải tiếp tục chia;
giảm còn 100 phải gộp, pixel không còn màu đường chia.
Test suy ra trạng thái qua render thay vì truy cập cờ private.
Cuối cùng vẽ nền/menu chính để kiểm tra đường vẽ UI sau thay đổi viewport.

## Giải phóng và chạy

Wrapper ra khỏi scope, hủy texture trước khi hủy software renderer,
rồi hủy surface và SDL_Quit. Lỗi in SDL_GetError; lỗi so pixel có thể
không có thông báo SDL hữu ích nên cần xem điều kiện và ảnh preview.

```powershell
ctest --test-dir build-windows -R '^render_assets$' --output-on-failure
```

CTest đặt working directory tại thư mục build; ảnh nằm ở đó và Assets tìm
được bản copy. Test không chứng minh hiệu năng GPU, fullscreen hoặc resize cửa sổ.
