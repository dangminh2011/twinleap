# Sfx.cpp — WAV và bộ trộn âm thanh

[Mã nguồn](../../../src/audio/Sfx.cpp) · [Header](../../../include/twinleap/audio/Sfx.hpp) · [Mục lục](../README.md)

## Tra cứu từng hàm

| Chữ ký | Tham số | Trả về / tác động / nơi gọi |
|---|---|---|
| `bool loadSample(const char *filename, std::vector<float> &sample, float gain=0.35f)` | Đường dẫn tương đối dưới assets/audio, vector nhận mẫu, hệ số âm lượng | bool thành công; sửa sample; helper nội bộ do constructor gọi |
| `Sfx::Sfx()` | Không tham số | Constructor không kiểu trả về; nạp WAV, mở audio; C++ gọi khi tạo Sfx |
| `Sfx::~Sfx()` | Không tham số | Destructor hủy stream; C++ gọi khi hết scope |
| `void Sfx::play(Sound s)` | Enum Jump/Coin/Death/Door/Win | void; đặt voice nếu được phép; PlayState/test gọi |
| `bool Sfx::hasSound(Sound sound) const` | Enum cần kiểm tra | true nếu chỉ số hợp lệ và mẫu có dữ liệu; play/test gọi, không sửa state |
| `bool Sfx::hasMusic() const` | Không tham số | true nếu music không rỗng; test gọi, không sửa state |
| `void Sfx::setPaused(bool value)` | true pause, false resume | void; sửa paused và trạng thái thiết bị khi cần; App/test gọi |
| `void Sfx::silence()` | Không tham số | void; xóa voice/queue, giữ musicPosition; App/toggle/test gọi |
| `void Sfx::toggle()` | Không tham số | void; đảo muted rồi silence; App gọi khi M, test gọi |
| `void Sfx::update()` | Không tham số | void; trộn mẫu, tăng vị trí đọc và gửi queue; App/test gọi |

### Các hàm SDL/STL được sử dụng

| Hàm | Kết quả / cách dùng |
|---|---|
| `SDL_InitSubSystem(SDL_INIT_AUDIO)` | bool; lỗi thì giữ audio không hoạt động |
| `SDL_LoadWAV(path,&spec,&raw,&rawBytes)` | bool; đồng thời ghi format, con trỏ buffer và kích thước byte |
| `SDL_ConvertAudioSamples(...)` | bool; tạo buffer chuyển đổi, ghi con trỏ và số byte qua tham số đầu ra |
| `SDL_free(buffer)` | void; giải phóng raw/converted do SDL cấp, không dùng delete |
| `SDL_OpenAudioDeviceStream(...)` | SDL_AudioStream* hoặc nullptr; mở luồng phát mặc định |
| `SDL_ResumeAudioStreamDevice(stream)` | bool; bắt đầu/tiếp tục thiết bị |
| `SDL_PauseAudioStreamDevice(stream)` | bool; tạm ngừng thiết bị |
| `SDL_ClearAudioStream(stream)` | bool; xóa dữ liệu audio đang chờ |
| `SDL_GetAudioStreamQueued(stream)` | Số byte đang chờ; âm là lỗi, không phải số mẫu float |
| `SDL_PutAudioStreamData(stream,data,bytes)` | bool; gửi dữ liệu buffer, bytes là kích thước theo byte |
| `SDL_DestroyAudioStream(stream)` | void; hủy stream và thiết bị mà OpenAudioDeviceStream gắn vào |
| `SDL_GetBasePath()` | const char* hoặc nullptr; phục vụ tìm file, không free |
| `std::filesystem::is_regular_file(path,error)` | bool; xác định WAV có tồn tại dạng file |
| `std::numeric_limits<int>::max()` | Giá trị int lớn nhất; tránh đổi rawBytes quá lớn sang int |
| `sample.resize(n)` / `sample.data()` | Cấp độ dài vector / lấy vùng nhớ liên tục của float |
| `std::memcpy(dst,src,bytes)` | Chép đúng số byte đã chuyển đổi vào vector |
| `std::clamp(x,-1.f,1.f)` | Giới hạn biên mẫu sau khi cộng nhiều nguồn |
| `SDL_GetError()` / `SDL_Log(...)` | Lấy lỗi và log, không dừng gameplay |

`&raw` hoặc `&converted` lấy địa chỉ biến con trỏ để SDL ghi buffer đầu ra;
khác việc truyền raw để SDL đọc nội dung buffer.

### Ví dụ sử dụng

```cpp
// Sau khi SDL đã được khởi tạo, trong vòng đời game:
if (sfx.hasSound(twinleap::Sound::Coin))
    sfx.play(twinleap::Sound::Coin);
sfx.update(); // Cấp dữ liệu trộn tới thiết bị
```

Có mẫu không bảo đảm đang nghe được: play còn kiểm tra stream, muted và paused.
Không tạo Sfx mỗi frame vì constructor nạp toàn bộ âm thanh.

## Dữ liệu và người gọi

App tạo Sfx một lần, gọi update mỗi frame và điều khiển mute/pause.
PlayState gọi play khi có sự kiện. samples giữ 5 vector float cho SFX,
music giữ nhạc nền, musicPosition là chỉ số đang đọc.
voices có 8 phần tử; sound=-1 là slot trống, position là vị trí đọc SFX.
Mọi thao tác trộn trong code này nằm trên luồng chính, không có audio callback tự viết.

## loadSample(filename, sample, gain)

Helper nội bộ tìm file tại assets/audio trong working directory, cạnh exe,
rồi thư mục cha exe. Nhận tên tương đối như sfx/jump.wav.

1. SDL_LoadWAV trả spec, raw buffer và số byte.
2. Loại mẫu rỗng/quá lớn trước khi chuyển kích thước sang int.
3. SDL_ConvertAudioSamples chuyển về mono SDL_AUDIO_F32, 44100 Hz.
4. Free raw, kiểm tra số byte chuyển đổi dương và chia hết sizeof(float).
5. Resize vector, memcpy dữ liệu, nhân gain cho từng mẫu, free buffer chuyển đổi.

Trả true khi sample đã có dữ liệu; không được thì log và trả false.
Gain mặc định 0,35 chừa khoảng cho nhiều SFX cộng cùng lúc; nhạc dùng 0,12.

## Constructor và destructor

Constructor khởi tạo SDL_INIT_AUDIO; lỗi thì log và return.
Nạp jump/coin/death/door/win theo thứ tự enum Sound, nạp background.wav,
mở AudioDeviceStream với mono float32 44100, rồi resume thiết bị.
Nếu resume lỗi thì hủy stream và đặt nullptr. Không mở được thiết bị
không làm game thoát. Destructor hủy stream nếu có.

## Các hàm điều khiển

| Hàm | Hiệu quả |
|---|---|
| hasSound(sound) | Kiểm tra chỉ số hợp lệ và vector mẫu không rỗng |
| hasMusic() | Kiểm tra vector nhạc không rỗng |
| play(sound) | Nếu đang hoạt động, lấy voice trống và bắt đầu đọc mẫu từ 0 |
| setPaused(value) | Chỉ pause/resume thiết bị khi trạng thái thật sự đổi |
| silence() | Đặt mọi voice về trống và xóa queue SDL; không reset musicPosition |
| toggle() | Đảo muted rồi silence để tiếng cũ không phát tiếp |

play bỏ qua nếu mute, pause, thiếu stream hoặc thiếu mẫu. Hết tám slot thì
ghi đè voice đầu. play chỉ tạo yêu cầu, âm thực được ghép trong update.

## update(): bộ trộn

Không có stream hoặc đang mute/pause thì return. Lấy số byte đang chờ SDL;
giá trị âm là lỗi, log rồi return.
Trong khi queue dưới 4096 byte:

1. Tạo buffer 512 float bằng 0 (2048 byte).
2. Có nhạc thì chép mẫu, tăng con trỏ và quay về đầu khi hết bài.
3. Với mỗi voice, cộng mẫu SFX vào buffer; hết mẫu đặt sound=-1.
4. Clamp từng mẫu về [-1,1] để không vượt biên số âm thanh.
5. SDL_PutAudioStreamData gửi buffer; lỗi log và dừng vòng queue.

Nạp WAV chỉ lúc khởi tạo, không đọc đĩa mỗi frame. Nhạc dài chiếm RAM toàn bộ;
đây không phải streaming file theo từng đoạn.

## Ví dụ và kiểm tra

Nhặt xu gọi play(Coin) → một voice bắt đầu ở mẫu 0 → App gọi update
→ trộn coin với nhạc → SDL phát queue → voice trống khi hết mẫu.
Pause từ App còn gọi silence nên SFX cũ bị bỏ, nhạc tiếp tục từ con trỏ đã đọc
khi resume; dữ liệu queue bị xóa có thể làm bỏ qua đoạn nhạc đã chuẩn bị ngắn.

Thay WAV giữ tên xem [AUDIO.md](../../AUDIO.md). Muốn thêm Sound phải sửa enum,
kích thước samples, danh sách filename và nơi gọi play, cùng test nạp mẫu.
[test_gameplay](../tests/test_gameplay.md) dùng dummy, xác nhận đường nạp/điều khiển
hoạt động nhưng không xác nhận âm nghe trên loa.
