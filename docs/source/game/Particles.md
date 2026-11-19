# Particles.cpp — pool hiệu ứng hạt

[Mã nguồn](../../../src/game/Particles.cpp) · [Header](../../../include/twinleap/game/Particles.hpp) · [Mục lục](../README.md)

## Tra cứu từng hàm

| Chữ ký | Tham số | Kết quả / nơi gọi |
|---|---|---|
| `void Particles::burst(Vec2 pos, SDL_Color color, int count)` | Tâm phát hạt theo pixel thế giới, màu RGBA, số hạt yêu cầu không âm | void; sửa các slot trống và seed random; PlayState gọi khi có sự kiện |
| `void Particles::update(float dt)` | Giây của một bước | void; sửa life/vel/pos của hạt sống; PlayState::update gọi |
| `void Particles::render(Renderer &r) const` | Renderer nhận lệnh vẽ | void; không sửa pool, có đổi trạng thái renderer; renderWorld gọi |

burst/update tính toán trực tiếp, không gọi hàm SDL hay sinh số ngẫu nhiên STL.
render gọi `r.rect({x,y,2,2},color)`, wrapper trả void và tự trừ camera.
`SDL_Color` ở đây là kiểu dữ liệu bốn kênh màu, không phải lời gọi hàm SDL.

```cpp
particles.burst({100, 80}, {255, 213, 74, 255}, 12);
particles.update(1.f / 120);
particles.render(renderer);
```

Ví dụ tạo 12 hạt vàng, cho chúng tiến một bước, rồi vẽ. Trong game,
update chạy theo fixed step và render theo frame; không buộc số lần gọi bằng nhau.

## Dữ liệu

Particles giữ array 512 Particle. Mỗi hạt có pos, vel, life và color.
life<=0 nghĩa là slot trống. Biến random thuộc đối tượng, seed ban đầu 1729;
không dùng biến global. PlayState gọi burst khi chết, nhặt xu hoặc đáp nền.

## burst(pos, color, count)

Duyệt pool, tìm hạt đã hết life. `count-- > 0` kiểm tra số lượng trước khi giảm;
khi đủ số yêu cầu thì không gán thêm. Hạt đang sống không bị ghi đè.
Pool đầy thì bỏ lượng còn thiếu, không cấp phát thêm.

Mỗi slot dùng công thức giả ngẫu nhiên `random=random*1664525u+1013904223u`.
Số unsigned cho phép tràn theo modulo. Vận tốc ngang được chọn từ -60 đến 60,
vận tốc Y từ -119 đến 0 để ban đầu bay lên; life đặt 0,6 giây.
Giữ cùng seed và cùng chuỗi gọi sẽ tạo cùng chuỗi số, thuận tiện khi quan sát.

## update(dt)

Với hạt đang sống: giảm life, tăng vel.y theo trọng lực 200*dt,
rồi cộng vận tốc nhân dt vào vị trí. Hạt có thể vừa hết life trong bước này;
render sẽ bỏ nó vì chỉ vẽ life>0.

## render(r) const

Duyệt hạt còn sống và vẽ Rect 2 × 2 tại pos bằng màu đã lưu.
rect mặc định theo thế giới nên Renderer trừ camera. Hàm const không sửa pool.
Trong split view, PlayState vẽ cùng pool trong cả hai viewport với camera khác nhau.

## Ví dụ và bài tập

burst ở chân với count=5 tạo bụi đáp đất; nhặt xu dùng 12, chết dùng 25.
Đặt breakpoint tại phép gán hạt để xem số slot tái dùng.
Thử đổi life 0,6 hoặc trọng lực 200 để phân biệt thời gian tồn tại và đường bay.
Không đổi hitbox/gameplay khi chỉnh hạt; đây là hiệu ứng hiển thị.
