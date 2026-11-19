# Level.cpp — đọc và truy vấn màn chơi

[Mã nguồn](../../../src/game/Level.cpp) · [Header](../../../include/twinleap/game/Level.hpp) · [Mục lục](../README.md)

## Tra cứu từng hàm

| Chữ ký | Tham số | Trả về / dữ liệu thay đổi / nơi gọi |
|---|---|---|
| `char Level::tileAt(int x, int y) const` | x/y là cột/hàng tile, không phải pixel | Ký tự tile hoặc ký tự biên; không sửa level; Physics/PlayState gọi |
| `bool Level::isSolidAt(int x, int y) const` | Chỉ số ô cần xét | true nếu tường/cửa đóng; không sửa level; Physics và debug gọi |
| `bool Level::load(const std::string &path, std::string &error)` | path chỉ đọc; error là chuỗi có thể ghi lỗi | true thành công, false lỗi; thay toàn bộ Level; PlayState và test gọi |
| Lambda `fail(reason)` trong load | Lý do lỗi dạng string | Ghi error theo path:line:reason và trả false để kết thúc load |

### Các hàm được dùng khi đọc map

| Hàm | Ý nghĩa và kết quả trong file này |
|---|---|
| `std::getline(file,line)` | Đọc một dòng vào line; kiểm tra stream để biết còn đọc được không |
| `line.compare(0,3,...)` | So ba byte đầu; trả 0 khi khớp BOM UTF-8 |
| `line.erase(0,n)` / `line.pop_back()` | Bỏ n ký tự đầu / ký tự cuối; sửa line trực tiếp |
| `line.find(':')` | Trả vị trí dấu hai chấm hoặc std::string::npos nếu không có |
| `line.substr(pos,count)` | Tạo chuỗi con; không sửa line; count bỏ qua nghĩa lấy tới cuối |
| `std::istringstream in(value)` | Tạo stream đọc từ chuỗi; `in >> parTime` thử đọc số |
| `std::max(width,int(line.size()))` | Lấy chiều rộng lớn hơn |
| `vector.push_back(value)` | Thêm dòng hoặc Object vào cuối vector |
| `row.resize(width,'.')` | Kéo dài dòng ngắn bằng dấu chấm |
| `std::to_string(lineNo)` | Đổi số dòng thành string cho thông báo lỗi |
| `overlaps(cell,o.box)` | Hàm Math.hpp: true khi hai Rect giao nhau; xét cửa đóng |

`std::ifstream file(path)` mở file qua constructor; `if (!file)` kiểm tra
trạng thái stream. `file` tự đóng khi ra khỏi scope, không cần gọi SDL đọc file.

### Ví dụ gọi

```cpp
twinleap::Level level;
std::string error;
if (level.load("assets/maps/01_first_steps.txt", error)) {
    char tile = level.tileAt(3, 2);
    bool blocked = level.isSolidAt(3, 2);
    // tile/blocked là kết quả truy vấn, không làm đổi map.
}
```

Nếu load=false, chỉ dùng error để báo lỗi; không dùng level như một map hoàn chỉnh.

## Vai trò

Chuyển TXT thành tiles, spawn, exit và objects. Không vẽ, phát âm hay cập nhật
vật lý. App chọn đường dẫn; PlayState gọi Level::load.

## load(path, error)

Đầu vào là đường dẫn và tham chiếu error. Trả true khi hợp lệ; false khi lỗi.
Khi thất bại, error chứa đường dẫn, số dòng đang đọc và lý do.
Hàm reset Level ngay từ đầu; không giữ nguyên màn cũ nếu đọc thất bại.
Không dựa vào nội dung error khi hàm trả true vì code không xóa chuỗi lỗi cũ.

1. Mở ifstream; lambda fail tạo thông báo lỗi dùng lại.
2. Bỏ UTF-8 BOM ở dòng đầu và ký tự CR cuối dòng để đọc file Windows/Linux.
3. Trước `--`, bỏ dòng trống/chú thích; đọc name và par. Metadata lạ có dấu
   hai chấm hiện được bỏ qua; không có kiểm tra nghiêm ngặt mọi khóa.
4. Sau `--`, bỏ dòng rỗng hoàn toàn; chỉ chấp nhận `.#=^oCEabABPQ`.
5. Đếm ký tự, đổi tọa độ tile thành Rect 16 × 16, lưu spawn/exit/objects.
6. Kiểm tra P/Q/E mỗi loại đúng một; a/A và b/B phải có cả hai phía.
7. Bù dấu chấm ở dòng ngắn, đặt tên bằng path nếu chưa có name, trả true.

Mảng counts có 256 phần tử. Ép ký tự sang unsigned char trước khi đánh chỉ số
tránh chỉ số âm trên hệ thống có char có dấu. Nhiều nút/cửa cùng nhóm được phép.

## tileAt(x, y) const

Nhận chỉ số tile, không phải pixel. Trong map trả `tiles[y][x]`.
Y dưới đáy trả dấu chấm; trái/phải/trên trả #. Nhánh dưới đáy được xét trước,
nên điểm vừa ra ngang vừa dưới đáy vẫn được coi trống.

## isSolidAt(x, y) const

Trả true khi tile là # hoặc ô 16 × 16 giao cửa A/B chưa active.
Các ký tự khác không tự trở thành solid: nền một chiều do Physics xử lý riêng.
Cửa active là mở; xu active là đã nhặt. Cùng tên cờ nhưng ý nghĩa tùy kind.

## Ví dụ đọc map

```text
name: Learn
par: 30
--
P.Q.E
##
```

Kết quả width=5, height=2, dòng cuối thành `##...`.
Spawn P=(2,2), Q=(34,2), exit=(64,0,16,16).
Đây chỉ là ví dụ parser, không phải thiết kế một màn hoàn chỉnh.

## Debug và thay đổi

Đặt breakpoint ở vòng for ký tự, xem c, r, counts và objects.
Lỗi ký tự báo dòng chứa nó; lỗi thiếu spawn/ghép cặp phát hiện cuối file nên
số dòng là vị trí đọc cuối, không chỉ ra một đối tượng bị thiếu.
Parser hợp lệ không chứng minh map giải được.

Thêm ký tự mới cần cập nhật whitelist, cách tạo dữ liệu, Physics/PlayState/Renderer
và [test_level_parser](../tests/test_level_parser.md) tương ứng.
