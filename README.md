# Unikey-Wayland

Bộ gõ tiếng Việt đa nền tảng tối ưu hóa cho môi trường **KDE Plasma Wayland** (phiên bản gốc) và nay đã hỗ trợ chạy native trên hệ điều hành **Windows (Windows Edition)**.

Mặc dù dự án khởi nguồn từ việc nghiên cứu mã nguồn của UniKey, nhưng hiện tại **toàn bộ logic gõ tiếng Việt cốt lõi đã được thay thế và sử dụng hoàn toàn bằng Bamboo Engine**. Giao diện cấu hình được viết mới bằng **Qt 6**.

Giao diện cấu hình chia Tab và quản lý gõ tắt được thiết kế dựa trên nguồn cảm hứng từ EVKey/UniKey truyền thống nhưng loại bỏ các thiết lập không tương thích với cơ chế của Wayland (như khởi động cùng Windows, quyền Admin, v.v.).

> [!WARNING]
> ~~**BẮT BUỘC SỬ DỤNG KDE PLASMA** (Wayland session). Giao thức input method grab bàn phím hoạt động tối ưu nhất trên bộ quản lý cửa sổ KWin của KDE.~~
> 
> **Hiện tại đã hỗ trợ mọi Desktop Environment** (KDE Plasma, GNOME, X11, Sway, Window Managers...) và **hỗ trợ song song cả hai kiến trúc x86_64 & ARM64 (aarch64)** nhờ việc tích hợp song song cả Native Wayland Protocol, IBus Engine và tối ưu hóa biên dịch ngoại tuyến.

## Hướng Dẫn Kích Hoạt (Kiến trúc Ring-0 mới)

Bắt đầu từ phiên bản 3.0.0, Unikey-Wayland đã loại bỏ hoàn toàn Wayland Protocol cũ và IBus, chuyển sang kiến trúc **Ring-0 Kernel Daemon**.
Bạn **KHÔNG CẦN** sử dụng Virtual Keyboard hay IBus nữa! Thay vào đó, hãy kích hoạt bộ gõ thông qua XKB Layout.

### 1. Trên KDE Plasma
Sau khi cài đặt, hãy làm theo các bước sau để kích hoạt bộ gõ:
1. Mở **System Settings** (Cài đặt hệ thống).
2. Tìm đến mục **Keyboard** (Bàn phím) -> **Layouts**.
3. Bấm **Add** để thêm Layout mới. Tìm kiếm `English (US)` và ở mục Variant, hãy chọn **Vietnamese (Kernel Driver)**.
4. Xóa các Layout thừa và đặt `Vietnamese (Kernel Driver)` làm mặc định.
5. Mở tiếp mục **Virtual Keyboard** (Bàn phím ảo) (Nằm ngay dưới mục Keyboard).
6. Chọn **Unikey-Wayland** (hoặc tích chọn để kích hoạt nó) và nhấn **Apply**.
7. Biểu tượng chữ **V/E** ở khay hệ thống (System Tray) sẽ xuất hiện. Bạn có thể bấm chuột hoặc nhấn `Ctrl + Shift` / `Alt + Z` để chuyển đổi chế độ gõ.

### 2. Trên GNOME Wayland & Môi trường X11
1. Mở **Settings** (Cài đặt) -> **Keyboard** (Bàn phím).
2. Tại mục *Input Sources*, nhấn dấu cộng (+) và tìm bàn phím **Vietnamese (Kernel Driver)**.
3. Chuyển sang bàn phím này và gõ tiếng Việt.
4. **Chú ý:** Nếu bạn đang dùng IBus hoặc Fcitx, hãy tắt hoặc gỡ cài đặt chúng, vì kiến trúc Ring-0 tự chạy ngầm độc lập!

### 3. Trên Windows (Unikey-Wayland Windows Edition)
1. Tải về file `UnikeyWayland-Windows-x64.zip` hoặc `UnikeyWayland-Windows-ARM64.zip` từ mục **[Releases](https://github.com/ubuntu2310fake/Unikey-Wayland/releases)**.
2. Giải nén thư mục tải về.
3. Nhấp đúp chuột vào file `setup.bat` (chương trình sẽ tự động yêu cầu quyền Admin, cài đặt ứng dụng vào `C:\Program Files\UnikeyWayland`, tạo Shortcut ngoài Desktop và tự động kích hoạt bộ gõ).
4. Bạn có thể mở Bảng điều khiển bằng cách nhấp đúp vào biểu tượng chữ **V/E** dưới khay hệ thống (System Tray).

---


## Tải Xuống (Downloads)

Bạn có thể tải các gói cài đặt đóng gói sẵn cho từng hệ điều hành trực tiếp tại trang **[Releases](https://github.com/ubuntu2310fake/Unikey-Wayland/releases)** của dự án này:

- **Windows Edition**: Tải file `.zip` (hỗ trợ cả x64 và ARM64/Copilot+ PCs)
- **Fedora (RPM)**: Tải file `.rpm` (hỗ trợ cả x86_64 và aarch64/ARM64)
- **Ubuntu / Debian (DEB)**: Tải file `.deb` (hỗ trợ cả x86_64 và arm64)
- **Arch Linux (tar.zst)**: Tải file `.pkg.tar.zst` (hỗ trợ cả x86_64 và aarch64/ARM64)

---

## Hướng Dẫn Cài Đặt

### 1. Fedora (Sử dụng Copr)

```bash
# Kích hoạt kho phần mềm Copr của tác giả
sudo dnf copr enable -y truonghieu/Unikey-Wayland

# Tiến hành cài đặt
sudo dnf install -y unikey-wayland
```

### 2. Ubuntu (Sử dụng PPA)

```bash
# Thêm kho PPA của tác giả vào hệ thống
sudo add-apt-repository -y ppa:trex219961/unikey-wayland-ppa

# Cập nhật cơ sở dữ liệu gói và cài đặt
sudo apt update
sudo apt install -y unikey-wayland
```

### 3. Arch Linux (tar.zst)

```bash
# Cài đặt package tar.zst tải về từ Releases
sudo pacman -U ./unikey-wayland-1.0.0-1-x86_64.pkg.tar.zst
```

---

## Cách Tự Build Từ Mã Nguồn

Yêu cầu hệ thống cần cài đặt sẵn **Qt 6 (Widgets, Gui, Core)**, **Wayland Client**, **Wayland Scanner** và các công cụ build cơ bản (`gcc`, `g++`, `cmake`, `make`).

```bash
# Tạo thư mục build
cmake -B build wayland-client

# Biên dịch ứng dụng
cmake --build build
```

## Bản Quyền
Mã nguồn phát triển dựa trên UniKey Engine (bản quyền GPL). Vui lòng xem tệp [LICENSE](LICENSE) để biết thêm chi tiết.

## 🤣 Fun fact

GitHub hiện xác định ngôn ngữ chính của Unikey-Wayland là **Makefile (61,6%)**.

Nói cách khác, theo GitHub thì đây là một **bộ gõ tiếng Việt Native Wayland được viết bằng... Makefile**.

Không, tôi chưa implement bộ gõ Telex bằng GNU Make đâu 🤣

Phần xử lý thực tế vẫn là C/C++, Wayland protocol và Bamboo Engine viết bằng Go. Chỉ là đống Makefile trong repository đã giành quyền kiểm soát biểu đồ ngôn ngữ.

---

## Câu hỏi thường gặp (FAQ)

### 1. Tại sao tên dự án vẫn là Unikey-Wayland trên mọi nền tảng?
Dự án sinh ra trên Linux Wayland, sau đó kiến trúc được mở rộng dần để hỗ trợ các môi trường khác (IBus, Windows). Một cái tên chung chung có thể làm mất dấu nguồn gốc của dự án, vì vậy, tên Unikey-Wayland được giữ nguyên trên mọi nền tảng.

### 2. Unikey-Wayland (Windows Edition) có phải là phiên bản chính của dự án không?
Không. Unikey-Wayland là tên dự án. Windows Edition chỉ là một phiên bản của dự án dành riêng cho Windows, tương tự như GNOME/IBus Edition hay bản gốc KDE Plasma Wayland. Việc một Edition có nhiều người dùng hơn không làm thay đổi cội nguồn dự án.

### 3. Unikey-Wayland có phải là UniKey chính thức không?
Không. Đây là một dự án mã nguồn mở độc lập và không phải phiên bản chính thức của UniKey. Tên dự án không có nghĩa phần mềm được chứng thực bởi tác giả UniKey gốc.

### 4. Tại sao tên là “Unikey” nhưng lại dùng lõi Bamboo?
Unikey-Wayland sử dụng lõi xử lý tiếng Việt (Engine) được dịch lại bằng C++ từ mã nguồn mở Bamboo. Kiến trúc modular cho phép phần xử lý phím (Frontend) và phần biến đổi chữ (Backend) tách biệt hoàn toàn nhau. Vui lòng xem phần Giấy phép để biết chi tiết.

### 5. Tại sao bản KDE chỉ có tên Unikey-Wayland mà các bản khác lại có chữ “Edition”?
KDE Plasma Wayland là môi trường ban đầu mà dự án hướng tới. Các môi trường sau sử dụng chữ Edition để xác định backend chuyên biệt: GNOME (IBus) Edition, Windows Edition, v.v.

### 6. Unikey-Wayland có thực sự đa nền tảng không?
Có, nhưng theo triết lý "Native Backend". Unikey-Wayland sử dụng các module bắt phím **riêng biệt cho từng môi trường** thay vì dùng một framework giả lập. KDE dùng Wayland Protocol, GNOME dùng IBus, còn Windows dùng Global Hook.

### 7. Tại sao không dùng một backend duy nhất cho tất cả nền tảng?
Hệ thống nhập liệu hoàn toàn khác biệt giữa Wayland, IBus và Windows. Cố ép tất cả sử dụng chung một cơ chế (ví dụ: Qt Input Method) thường dẫn đến độ trễ, nháy chữ hoặc lặp ký tự.

### 8. Tại sao Unikey-Wayland không thích Preedit Mode (Gạch chân chữ)?
Đối với tiếng Việt (Telex/VNI), chúng tôi ưu tiên trải nghiệm "thay thế trực tiếp" để mang lại cảm giác gõ tự nhiên.
- **Trên Linux:** Preedit Mode vẫn được sử dụng như một cơ chế dự phòng an toàn cho các Terminal (thông qua D-Bus).
- **Trên Windows:** Preedit Mode đã bị **loại bỏ hoàn toàn** nhờ thuật toán lách lỗi Omnibox đặc trị (tiêm lại phím vật lý để xóa vùng chọn).

### 9. Unikey-Wayland có dùng TSF (Text Services Framework) trên Windows không?
**Tuyệt đối Không.** Windows Edition hoàn toàn không sử dụng TSF. TSF từng được chúng tôi thử nghiệm nhưng gây ra lỗi lặp chữ trên Chromium. Để đạt tốc độ tuyệt đối, chúng tôi sử dụng kiến trúc bắt phím mức thấp (Global Keyboard Hook) kết hợp API SendInput.

### 10. Tại sao đôi khi bộ gõ bị lặp chữ?
Lặp chữ xảy ra khi trạng thái bộ gõ và tốc độ render của ứng dụng bị mất đồng bộ. Đây là một lỗi nghiêm trọng. Nếu gặp lỗi này, hãy báo cáo Issue và ghi rõ nền tảng, tên ứng dụng và cách tái hiện.

### 11. Tại sao không thêm sleep(20ms) để sửa lỗi lặp chữ?
Dùng độ trễ (Delay) chỉ là cách che đậy lỗi (Race condition) trên máy tính này và sẽ sinh ra lỗi trên máy tính khác. Unikey-Wayland kiên quyết tìm ra cơ chế xử lý sự kiện chuẩn xác nhất thay vì dựa vào thời gian chờ.

### 12. Tại sao Terminal luôn là chỗ bộ gõ dễ gặp vấn đề?
Terminal có mô hình nhập liệu khác hẳn trình soạn thảo văn bản. Không thể kỳ vọng Terminal cung cấp các khả năng thay thế chữ giống như MS Word. Do đó, trên Linux, Terminal luôn bị ép vào Preedit Mode để bảo đảm an toàn.

### 13. Có thể cài Unikey-Wayland và UniKey gốc cùng lúc không?
Bạn có thể cài đặt, nhưng **tuyệt đối không bật đồng thời hai bộ gõ**. Hai phần mềm cùng tranh nhau bắt một phím sẽ gây ra lỗi văn bản không lường trước.

### 14. Windows Defender báo bộ gõ đang theo dõi bàn phím. Có phải keylogger không?
Bộ gõ nào cũng cần theo dõi phím (Global Hook) để biến T, E, L, E, X thành chữ Việt. Unikey-Wayland là mã nguồn mở 100%, xử lý ký tự hoàn toàn cục bộ (offline) và không bao giờ gửi bất kỳ dữ liệu nào lên Internet.

### 15. Unikey-Wayland có chạy trên Windows không?
Có. Hãy tải file `UnikeyWayland.exe` trong mục Release. Nó chạy hoàn toàn độc lập như mọi phần mềm Windows, không cần WSL, không cần cài Linux hay Wayland.

👉 **Đọc thêm toàn bộ các câu hỏi thú vị khác tại [FAQ.md](FAQ.md)**
