# Changelog

Tất cả các thay đổi đáng chú ý của dự án bộ gõ Unikey Wayland sẽ được ghi chép trong tệp này.

Định dạng dựa trên [Keep a Changelog](https://keepachangelog.com/vi/1.0.0/).

## [Unreleased]

## [2.0.0] - 2026-07-13

### Đã thêm (Added)
- **Hỗ trợ hệ điều hành Windows (Windows Edition)**: Hỗ trợ chạy native trên Windows sử dụng kiến trúc bắt phím toàn hệ thống mức thấp (Global Keyboard Hook) và mô phỏng phím (SendInput). Loại bỏ hoàn toàn TSF (Text Services Framework) để triệt tiêu lỗi lặp chữ/nhấp nháy trên Chrome.
- **Phím tắt đổi E/V trên Windows**: Hỗ trợ phím tắt `Ctrl+Shift` (bắt sự kiện nhả phím thông minh, tránh xung đột phím tắt hệ thống khác) và `Alt+Z`.
- **Cơ chế đóng gói cài đặt tự động (CI/CD)**: Tích hợp kịch bản đóng gói Windows tự động qua GitHub Actions (`windows-2025-vs2026` runner), cung cấp bộ cài đặt gồm `7z.exe` + `setup.bat` + `UnikeyWayland.7z` nâng quyền cài đặt cực nhanh.
- **Cơ chế Hybrid lách lỗi Chrome Omnibox cho IBus Engine**: Cải tiến đột phá cơ chế xóa chữ trên IBus. Tự động kiểm tra vùng bôi đen (selection) để giả lập Backspace phá vỡ autocomplete trên Chrome Omnibox, còn khi gõ bình thường dùng `delete_surrounding_text` trực tiếp. Loại bỏ hoàn toàn lệnh ngủ `g_usleep(20ms)` trước đây, giúp gõ trên IBus nhanh và mượt mà hơn rất nhiều.
- **Tính năng Gõ tắt (Macro) cho IBus**: Cho phép IBus Engine đọc trực tiếp bảng cấu hình gõ tắt từ file JSON và tự động thay thế từ khi gõ.

### Đã thay đổi (Changed)
- **Tắt tính năng Gõ tắt (Macro) trên Terminal (Preedit mode) ở một số phiên bản**: Do cơ chế auto-commit của một số Terminal emulator xung đột với hành vi xử lý macro của IBus, tính năng Gõ tắt đã bị loại bỏ trong chế độ Preedit của IBus để đảm bảo độ ổn định. Tính năng Gõ tắt vẫn hoạt động hoàn hảo trên chế độ Native (KDE Plasma Wayland) và bản Windows.
- **Xóa bỏ hoàn toàn Chế độ Terminal (F12)**: Gỡ bỏ Menu và phím tắt F12 chuyển đổi chế độ Terminal thủ công trên cả 2 bản Wayland và Windows (do Windows không cần Preedit, còn Linux đã tự động nhận dạng ứng dụng qua D-Bus).

## [1.0.4] - 2026-07-11

### Đã sửa (Fixed)
- **Sửa lỗi phím tắt chuyển E/V**: Khắc phục lỗi phím tắt Alt+Z cứng ngắc, nay đã đọc cấu hình phím tắt chính xác từ giao diện người dùng. Sửa lỗi dính phím Alt khiến phím Z vô tình chuyển chế độ ngôn ngữ.
- **Tính năng gõ tắt (Macro)**: Tích hợp hoàn chỉnh tính năng thay thế từ gõ tắt trong chế độ Wayland, cho phép người dùng định nghĩa và tự động mở rộng các cụm từ yêu thích.


## [1.0.3] - 2026-07-10

### Đã thêm (Added)
- **Hỗ trợ kiến trúc ARM64 (aarch64)**: Tối ưu hóa toàn bộ quy trình đóng gói và biên dịch ngoại tuyến (offline-ready vendoring) cho phép chạy trên các hệ thống ARM64 (như Raspberry Pi, máy Mac Apple Silicon chạy Linux hoặc máy chủ ARM).
- **Engine IBus fallback cho GNOME**: Hỗ trợ chạy bộ gõ dưới dạng một IBus engine độc lập (`ibus-engine-unikey-wayland`) để Unikey có thể hoạt động hoàn toàn mượt mà trên môi trường desktop GNOME Wayland.
- **Sửa lỗi lặp chữ trên Chrome (Normal mode)**: Áp dụng cơ chế mô phỏng Backspace có độ trễ ngắn (sleep-delayed Backspaces) cho IBus engine để khắc phục triệt để lỗi lặp ký tự (như `coó`, `loônồn`) trên các trình duyệt dựa trên Chromium và ứng dụng Electron chạy Wayland.
- **Sửa lỗi nhảy cursor trong Preedit**: Đếm chính xác số lượng ký tự Unicode thực tế bằng `g_utf8_strlen` thay vì độ dài byte khi cập nhật preedit text, khắc phục triệt để lỗi nhảy con trỏ và nhân đôi chữ ở Terminal.

## [1.0.2] - 2026-07-09

### Đã thêm (Added)
- **Tích hợp Bamboo Engine**: Thay thế thành công lõi xử lý tiếng Việt từ Unikey C++ sang lõi mã nguồn mở Bamboo (viết bằng Go) thông qua CGO (`bamboo_wrapper.go`). Sự thay đổi này mang lại tốc độ xử lý nhanh hơn và đặc biệt là khả năng gõ tiếng Việt tự do (free-typing) vượt trội.
- **Giải pháp Hybrid lách lỗi Chrome**: Bổ sung thuật toán thông minh phát hiện vùng bôi đen ngược trên giao thức Wayland (đặc trị cho thanh địa chỉ Google Chrome Omnibox). Tự động kích hoạt luồng phím Backspace mô phỏng để phá vùng bôi đen thay vì dùng lệnh Wayland `delete_surrounding_text` (lệnh này bị dính bug toán học bên trong nội bộ mã nguồn Chrome).

### Đã thay đổi (Changed)
- **Kiến trúc xử lý phím gốc**: Cấu trúc lại toàn bộ hệ thống bắt phím trong `main.cpp` để tương thích hoàn toàn với cơ chế hoạt động, tính toán độ lệch byte và trả về chuỗi thay thế của thư viện Bamboo.
- **Cải thiện Backspace & Phím chức năng**: Bắt và xử lý mượt mà phím Backspace, Phím cách (Space), Enter, Esc để reset trạng thái của Bamboo kịp thời, loại bỏ hoàn toàn các lỗi dính chữ cũ hoặc lỗi khi xóa lùi.
- **Tối ưu hóa Wayland & X11 Input**: Tối ưu lại logic tính toán số ký tự cần xóa (`char_backs`) bằng thuật toán `common_bytes` thay vì xóa toàn bộ chuỗi. Đối với 99% các ứng dụng bình thường, bộ gõ duy trì sử dụng lệnh `delete_surrounding_text` thuần túy để đảm bảo hiệu năng.
- **Tính năng độc quyền cho Chrome/IDE**: Thêm cơ chế tự động nhận diện tên tiến trình qua D-Bus (ví dụ: Google Chrome, Alacritty, Android Studio). Khi phát hiện các ứng dụng không hỗ trợ tốt `delete_surrounding_text` trên X11, bộ gõ sẽ tự động chuyển sang cơ chế Preedit (gạch chân) để khắc phục triệt để lỗi "coó caiái" hoặc loạn chữ trên thanh địa chỉ.
- **Cập nhật Build System**: Viết lại `CMakeLists.txt` để hỗ trợ quy trình biên dịch chéo C++ và Go (Bamboo), đồng thời loại bỏ các tệp biên dịch của Unikey cũ.

### Đã xóa (Removed)
- Xóa bỏ hoàn toàn mã nguồn thư viện `vnconv` gốc của Unikey C++.
- Loại bỏ các file wrapper C++ cũ (`ukengine_wrapper.cpp`, `ukengine.h`) không còn sử dụng.
