# test_buttons_doors.cpp — trạng thái cửa theo thời gian

[Mã nguồn](../../../tests/test_buttons_doors.cpp) · [Entities](../game/Entities.md) · [Mục lục](../README.md)

## Tra cứu hàm và lời gọi sử dụng

`int main()` không nhận tham số; tạo Level/Player cục bộ, trả 0 khi đạt.
`CHECK(x)` là macro: x=false thì ghi số dòng/biểu thức và return 1 từ main.
Không có helper hay hàm SDL trong file này.

Hàm project được gọi là
`bool updateDoors(Level &level,const std::array<Player,2> &players,float dt)`.
level bị sửa active/timer; players chỉ đọc; dt theo giây;
kết quả true nghĩa có cửa **đổi trạng thái**. Ví dụ:

```cpp
CHECK(updateDoors(l, p, 0.01f)); // Kiểm tra có thay đổi
CHECK(l.objects[1].active);     // Kiểm tra trạng thái mới là mở
```

Hai CHECK khác ý nghĩa: cửa đã mở sẵn có thể vẫn active nhưng updateDoors=false.
std::cerr được macro dùng để in lỗi; code còn lại chủ yếu gán dữ liệu thử.

## Thiết lập

Gọi updateDoors trực tiếp, không mở SDL hay chạy Physics.
Level có nút a ở (16,16), cửa A ở (64,16), cùng kích thước 16 × 16.
P1 ở (18,18), onGround=true để chân chạm nút; P2 ở xa.
Không cần tiles vì hàm chỉ đọc objects và players.

## Chuỗi kiểm tra main

1. Cập nhật 0,01 giây: changed=true, cửa active.
2. Dời P1 khỏi nút, cập nhật 0,1 giây: cửa còn mở vì timer 0,2 chưa hết.
3. Thêm 0,11 giây: cửa đóng.
4. Đưa P1 lại nút để mở, rồi vào cửa; cập nhật một giây:
   timer hết nhưng occupied giữ mở.
5. Dời P1 ra ngoài, cập nhật: cửa đóng.

dt=1 ở đây chỉ để timer hết nhanh trong test đối tượng;
không phải cho Physics chạy một bước dài một giây.
CHECK sai in dòng và biểu thức, trả 1; đi hết main trả 0.

## Chạy và tự học

```powershell
ctest --test-dir build-windows -R '^buttons_doors$' --output-on-failure
```

Quan sát l.objects[1].active/timer và chân người ở debugger.
Đổi độ trễ cửa cần điều chỉnh các mốc kỳ vọng theo yêu cầu mới.
Test hiện kiểm tra một cặp a/A và người ở trong; chưa có tình huống riêng
cho nhiều nút cùng nhóm hoặc b/B. Có thể dùng cùng cách thiết lập để bổ sung.
