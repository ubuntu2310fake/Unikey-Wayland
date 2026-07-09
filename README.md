# Unikey-Wayland

Bộ gõ tiếng Việt UniKey được viết lại giao diện cấu hình bằng **Qt 6** tối ưu hóa cho môi trường **KDE Plasma Wayland**.

Giao diện cấu hình chia Tab và quản lý gõ tắt được thiết kế dựa trên nguồn cảm hứng từ EVKey/UniKey truyền thống nhưng loại bỏ các thiết lập không tương thích với cơ chế của Wayland (như khởi động cùng Windows, quyền Admin, v.v.).

> [!WARNING]
> ~~**BẮT BUỘC SỬ DỤNG KDE PLASMA** (Wayland session). Giao thức input method grab bàn phím hoạt động tối ưu nhất trên bộ quản lý cửa sổ KWin của KDE.~~
> 
> **Hiện tại đã hỗ trợ mọi Desktop Environment** (KDE Plasma, GNOME, X11, Sway, Window Managers...) nhờ việc tích hợp song song cả Native Wayland Protocol và IBus Engine.

## Hướng Dẫn Kích Hoạt Sau Khi Cài Đặt

### 1. Trên KDE Plasma (Wayland Session)
Sau khi cài đặt gói tương ứng cho distro của bạn, hãy làm theo các bước sau để kích hoạt bộ gõ:
1. Mở **System Settings** (Cài đặt hệ thống).
2. Tìm đến mục **Keyboard** (Bàn phím) -> **Virtual Keyboard** (Bàn phím ảo).
3. Chọn **Unikey-Wayland** (hoặc tích chọn để kích hoạt nó).
4. Nhấn **Apply**.
5. Nhấp đúp chuột vào biểu tượng chữ **V/E** ở khay hệ thống (System Tray) hoặc nhấn tổ hợp phím `Ctrl + Shift + F5` để mở Bảng điều khiển cấu hình bộ gõ.

### 2. Trên GNOME Wayland & Môi trường X11 (Sử dụng IBus Engine)
Do GNOME sử dụng IBus làm nền tảng gõ mặc định, gói cài đặt đã bao gồm sẵn một IBus Engine tương thích.
1. Mở **Settings** (Cài đặt) -> **Keyboard** (Bàn phím).
2. Tại mục *Input Sources*, nhấn dấu cộng (+) và thêm bộ gõ **Vietnamese (Unikey-Wayland)**.
3. Đảm bảo biến môi trường `GTK_IM_MODULE=ibus` và `QT_IM_MODULE=ibus` được hệ thống kích hoạt.
4. Chuyển đổi bộ gõ bằng phím tắt `Super + Space`.
5. Bạn có thể mở Bảng điều khiển bằng cách chuột phải vào biểu tượng IBus trên thanh Top Bar (Khay hệ thống) và chọn **Preferences**.

---

## Tải Xuống (Downloads)

Bạn có thể tải các gói cài đặt đóng gói sẵn cho từng Distro trực tiếp tại trang **[Releases](https://github.com/ubuntu2310fake/Unikey-Wayland/releases)** của dự án này:

- **Fedora (RPM)**: Tải file `.rpm`
- **Ubuntu / Debian (DEB)**: Tải file `.deb`
- **Arch Linux (tar.zst)**: Tải file `.pkg.tar.zst`

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
