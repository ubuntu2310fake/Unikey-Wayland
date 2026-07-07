# Unikey-Wayland

Bộ gõ tiếng Việt UniKey được viết lại giao diện cấu hình bằng **Qt 6** tối ưu hóa cho môi trường **KDE Plasma Wayland**.

Giao diện cấu hình chia Tab và quản lý gõ tắt được thiết kế dựa trên nguồn cảm hứng từ EVKey/UniKey truyền thống nhưng loại bỏ các thiết lập không tương thích với cơ chế của Wayland (như khởi động cùng Windows, quyền Admin, v.v.).

## Các Tính Năng Nổi Bật

- **Bố cục EVKey truyền thống**: Cực kỳ quen thuộc và dễ cấu hình.
- **Hỗ trợ gõ tắt (Macro)**: Giao diện trực quan cho phép thêm, sửa, xóa từ gõ tắt.
- **Lưu cấu hình tự động**: Cấu hình và bảng gõ tắt được lưu vào `~/UnikeyWayland/global.json`.
- **Hỗ trợ phím tắt**:
  - Chuyển chế độ E/V nhanh bằng phím tắt `Ctrl + Shift` hoặc `Alt + Z` (có thể tùy cấu hình).
  - Bật nhanh bảng cài đặt (HUD) bằng phím tắt `Ctrl + Shift + F5` hoặc gõ đúp chuột trái vào biểu tượng khay hệ thống (System Tray).
- **Hỗ trợ kiểu gõ Telex giản lược**: Kiểu gõ giống Telex thông thường nhưng ký tự `w` gõ độc lập sẽ không bị chuyển thành `ư`.

## Hướng Dẫn Cài Đặt Cho Các Distro

### 1. Fedora (RPM)

Bộ gõ đã được đóng gói sẵn dưới định dạng `.rpm`.

```bash
# Cài đặt file RPM
sudo dnf install -y ./rpmbuild/RPMS/x86_64/unikey-wayland-1.0.0-1.fc44.x86_64.rpm

# Nếu muốn cài đè lại
sudo dnf reinstall -y ./rpmbuild/RPMS/x86_64/unikey-wayland-1.0.0-1.fc44.x86_64.rpm
```

### 2. Ubuntu / Debian (DEB)

```bash
# Cài đặt file DEB
sudo apt install -y ./unikey-wayland_1.0.0_amd64.deb
```

### 3. Arch Linux (tar.zst)

Bạn có thể giải nén file package hoặc cài đặt trực tiếp bằng `pacman`:

```bash
# Cài đặt package tar.zst qua pacman
sudo pacman -U unikey-wayland-1.0.0-1-x86_64.pkg.tar.zst
```

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
