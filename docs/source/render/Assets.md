# Assets.cpp — nạp và sở hữu texture

[Mã nguồn](../../../src/render/Assets.cpp) · [Header](../../../include/twinleap/render/Assets.hpp) · [Mục lục](../README.md)

## Tra cứu từng hàm

| Chữ ký | Tham số và trả về | Tác động và nơi gọi |
|---|---|---|
| `bool Assets::load(SDL_Renderer *renderer)` | Renderer SDL hợp lệ; true khi nạp hết, false nếu bất kỳ ảnh/bước thiết lập lỗi | Tạo/giữ texture; App và test gọi lúc khởi tạo |
| `SDL_Texture *Assets::get(Sprite sprite) const` | Enum ảnh hợp lệ; trả con trỏ texture đang giữ | Không đổi Assets, không chuyển sở hữu; Renderer::sprite gọi |
| `Assets::~Assets()` | Không tham số, không kiểu trả về | Hủy texture khác nullptr; C++ tự gọi khi Assets hết vòng đời |

### Các hàm SDL/STL được sử dụng

| Lời gọi | Kết quả và cách xử lý |
|---|---|
| `SDL_LoadBMP(path.c_str())` | SDL_Surface* hoặc nullptr; lỗi thì log và return false |
| `SDL_MapSurfaceRGB(surface,255,0,255)` | Giá trị pixel tương ứng màu magenta trong format surface |
| `SDL_SetSurfaceColorKey(surface,true,key)` | bool; chọn pixel key làm trong suốt |
| `SDL_CreateTextureFromSurface(renderer,surface)` | SDL_Texture* hoặc nullptr; ảnh dùng để render |
| `SDL_DestroySurface(surface)` | void; giải phóng bản ảnh tạm sau khi tạo texture |
| `SDL_SetTextureScaleMode(texture,NEAREST)` | bool; giữ pixel sắc nét khi phóng |
| `SDL_SetTextureBlendMode(texture,BLEND)` | bool; bật hòa trộn alpha |
| `SDL_DestroyTexture(texture)` | void; destructor dùng để giải phóng tài nguyên |
| `SDL_GetBasePath()` | const char* hoặc nullptr; tìm assets cạnh exe, không tự free |
| `SDL_SetError(...)` | Đặt thông điệp lỗi SDL; file này dùng khi không tìm được root |
| `SDL_GetError()` / `SDL_Log(...)` | Lấy chuỗi lỗi và ghi lại |
| `std::filesystem::is_regular_file(path,error)` | bool, lỗi qua error_code; kiểm tra ảnh mốc |
| `path / relative`, `path.string()`, `string.c_str()` | Ghép path, đổi thành string, lấy con trỏ chuỗi C cho SDL |

### Ví dụ lấy ảnh sau khi nạp

```cpp
if (renderer.assets.load(sdlRenderer)) {
    SDL_Texture *texture = renderer.assets.get(twinleap::Sprite::Terrain);
    // texture do assets sở hữu; không tự SDL_DestroyTexture(texture).
}
```

Đây là ví dụ khởi tạo; game đã load trong App nên không thêm lời gọi load
lần nữa vào update/render. get không kiểm tra biên enum, không truyền Sprite::Count.

## Vai trò và vòng đời

Assets giữ mảng SDL_Texture*, được nhúng trong Renderer. App gọi load một lần
lúc khởi động; Renderer::sprite gọi get để lấy ảnh; destructor hủy texture.
Surface là dữ liệu ảnh tạm trong RAM; texture là tài nguyên dùng để render.

## load(renderer)

Danh sách names chứa 14 đường dẫn không đuôi: 8 strip nhân vật, terrain,
spikes và 4 nền. Thứ tự phải khớp enum Sprite trong header.
Tìm root lần lượt tại ./assets, cạnh executable và thư mục cha executable;
chọn ứng viên có characters/raccoon/idle.bmp. File mốc này tồn tại chưa chứng
minh đủ cả bộ; các ảnh tiếp theo vẫn được kiểm tra khi đọc.

Với từng tên:

1. Ghép root, tên và .bmp; SDL_LoadBMP tạo surface.
2. SetSurfaceColorKey dùng RGB(255,0,255) làm màu trong suốt.
3. Tạo texture từ surface nếu đặt color key thành công.
4. Hủy surface dù texture có tạo được hay không.
5. Kiểm tra texture, đặt nearest scale mode và blend mode; lỗi thì log, trả false.

Thành công hết mới trả true. Ảnh thiếu/hỏng là lỗi startup, không tự thay hình
chữ nhật dự phòng. Texture nạp trước lúc lỗi vẫn nằm trong mảng và được destructor hủy.

## get(sprite) const và destructor

get ép enum về size_t rồi trả phần tử mảng. Không chuyển quyền sở hữu:
người gọi dùng con trỏ để vẽ, không SDL_DestroyTexture nó.
get không kiểm tra chỉ số, nên chỉ truyền enum hợp lệ, không Sprite::Count.
Destructor duyệt mảng và hủy con trỏ khác nullptr trước khi SDL renderer bị hủy.
Header cấm sao chép Assets để tránh hai đối tượng hủy cùng texture.

## Thứ tự và điều cần giữ

Renderer::character chọn `player*4+state`; bốn trạng thái mỗi người phải giữ
idle/run/jump/fall đúng thứ tự. Thêm ảnh cần sửa enum, mảng names và nơi dùng.
Hàm load được thiết kế dùng một lần trong vòng đời Assets; gọi lại trên đối tượng
đã nạp không có bước dọn texture cũ trong code hiện tại.

## Bài tập

Đặt breakpoint tại SDL_LoadBMP và CreateTextureFromSurface để quan sát
surface chỉ sống trong vòng lặp, texture sống tới cuối chương trình.
Thay BMP bản gốc, giữ kích thước strip/color key rồi build để copy.
Kiểm tra bằng [test_render_assets](../tests/test_render_assets.md) và quan sát ảnh preview.
