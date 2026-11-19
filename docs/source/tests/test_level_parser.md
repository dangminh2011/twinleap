# test_level_parser.cpp — dữ liệu map hợp lệ và lỗi

[Mã nguồn](../../../tests/test_level_parser.cpp) · [Level](../game/Level.md) · [Mục lục](../README.md)

## Tra cứu hàm và lời gọi sử dụng

```cpp
int main(int argc, char **argv);
// Lambda cục bộ có dạng:
// bool parse(const std::string &content);
```

main nhận argc=2 và argv[1] là thư mục map, trả 0 khi đạt hoặc 1 qua CHECK.
Lambda parse nhận nội dung TXT, ghi fixture, gọi level.load rồi trả bool của load;
nó sửa level/error và file fixture qua các biến bắt bằng tham chiếu.
Lambda được main gọi nhiều lần; CHECK là macro báo lỗi/return, không phải hàm.

| Hàm được gọi | Kết quả / mục đích |
|---|---|
| `std::filesystem::directory_iterator(argv[1])` | Tạo bộ duyệt file/thư mục; main chỉ xét extension()==".txt" |
| `level.load(path,error)` | bool; sửa level và error khi lỗi |
| `std::filesystem::current_path()` | Path working directory để đặt fixture |
| `std::ofstream out(fixture)` / `out.close()` | Mở file ghi / đóng file trước đọc lại |
| `error.find(text)` | Vị trí chuỗi hoặc npos; dùng kiểm tra nội dung thông báo |
| `std::filesystem::remove(fixture)` | bool cho biết đã xóa; code gọi dọn file cuối test |

Ví dụ `CHECK(!parse("--\nP.Q\n"))` mong thất bại vì thiếu E.
Muốn kiểm tra đúng lý do phải xem error ngay sau lần parse này.

## Đầu vào và luồng main

main yêu cầu argc=2; CTest truyền thư mục assets/maps vào argv[1].
Duyệt mọi TXT, gọi load, kiểm tra width/height dương và tổng ít nhất năm màn.
Map mới tự tham gia test mà không cần thêm tên vào một danh sách.

Sau đó tạo parser_fixture.txt trong working directory. Lambda parse ghi chuỗi
được truyền vào, đóng file rồi gọi Level::load. Đóng trước khi đọc bảo đảm
ghi xong dữ liệu. Cuối main xóa fixture; CHECK lỗi return sớm có thể để file tạm lại.

## Các trường hợp fixture

| Dữ liệu | Kỳ vọng |
|---|---|
| Tên Test, dòng dài 5 và dòng ## | width=5, dòng cuối thành ##... |
| Thiếu P/Q/E | Thất bại, error nhắc ký hiệu thiếu |
| PPQE | Thất bại vì P lặp |
| PQEA/PQEa | Thất bại vì cặp nút/cửa thiếu |
| PQE? | Thất bại, thông báo có :2: |
| par: bad | Thất bại vì par không đọc được thành số |
| Chỉ metadata thiếu -- | Thất bại |

CHECK in dòng/biểu thức rồi return 1 khi sai. Test chỉ đọc error sau lần
load thất bại tương ứng; code parser không xóa chuỗi error khi thành công.

## Chạy và tự học

```powershell
ctest --test-dir build-windows -R '^level_parser$' --output-on-failure
```

Đặt breakpoint trong lambda parse và Step Into load để theo dõi từng ký tự.
Thêm tình huống bằng CHECK(!parse(...)) rồi kiểm tra error.
Test xác nhận định dạng, chưa xác nhận puzzle hoặc đường tới exit giải được.
