# Unikey-Wayland

Bộ gõ tiếng Việt UniKey được viết lại giao diện cấu hình bằng **Qt 6** tối ưu hóa cho môi trường **KDE Plasma Wayland**.

Giao diện cấu hình chia Tab và quản lý gõ tắt được thiết kế dựa trên nguồn cảm hứng từ EVKey/UniKey truyền thống nhưng loại bỏ các thiết lập không tương thích với cơ chế của Wayland (như khởi động cùng Windows, quyền Admin, v.v.).

> [!WARNING]
> **BẮT BUỘC SỬ DỤNG KDE PLASMA** (Wayland session). Giao thức input method grab bàn phím hoạt động tối ưu nhất trên bộ quản lý cửa sổ KWin của KDE.

## Hướng Dẫn Kích Hoạt Sau Khi Cài Đặt

Sau khi cài đặt gói tương ứng cho distro của bạn, hãy làm theo các bước sau để kích hoạt bộ gõ:
1. Mở **System Settings** (Cài đặt hệ thống).
2. Tìm đến mục **Keyboard** (Bàn phím) -> **Virtual Keyboard** (Bàn phím ảo).
3. Chọn **Unikey-Wayland** (hoặc tích chọn để kích hoạt nó).
4. Nhấn **Apply**.
5. Nhấp đúp chuột vào biểu tượng chữ **V/E** ở khay hệ thống (System Tray) hoặc nhấn tổ hợp phím `Ctrl + Shift + F5` để mở Bảng điều khiển cấu hình bộ gõ.

---

## Tải Xuống (Downloads)

Bạn có thể tải các gói cài đặt đóng gói sẵn cho từng Distro trực tiếp tại trang **[Releases](https://github.com/ubuntu2310fake/Unikey-Wayland/releases)** của dự án này:

- **Fedora (RPM)**: Tải file `.rpm`
- **Ubuntu / Debian (DEB)**: Tải file `.deb`
- **Arch Linux (tar.zst)**: Tải file `.pkg.tar.zst`

---

## Hướng Dẫn Cài Đặt

### 1. Fedora (RPM)

```bash
# Cài đặt file RPM tải về từ Releases
sudo dnf install -y ./unikey-wayland-1.0.0-1.fc44.x86_64.rpm
```

### 2. Ubuntu / Debian (DEB)

```bash
# Cài đặt file DEB tải về từ Releases
sudo apt install -y ./unikey-wayland_1.0.0_amd64.deb
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
