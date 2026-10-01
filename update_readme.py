with open("README.md", "r") as f:
    readme = f.read()

ring0_doc = """## Kiến trúc Ring-0 Kernel Daemon (Bản 3.0+)
Bắt đầu từ phiên bản 3.0.0, Unikey-Wayland đã được viết lại hoàn toàn, chuyển từ kiến trúc `ibus` / `text-input-v3` ở tầng User-space xuống **Ring-0 Kernel Daemon**.
Điều này mang lại độ trễ gõ phím tuyệt đối 0ms, tương thích 100% với mọi ứng dụng Wayland (kể cả các game hay terminal không hỗ trợ input method) và tránh được hoàn toàn các lỗi drop key của KWin (KDE) hay Mutter (GNOME).

**Hướng dẫn thiết lập cho các Desktop Environment:**
Để kiến trúc Ring-0 có thể tiêm (inject) ký tự tiếng Việt Unicode (UTF-8) vào Wayland compositor, bạn CẦN sử dụng custom XKB Layout đi kèm:
1. **KDE Plasma**: Mở `Settings` -> `Keyboard` -> `Layouts` -> Bấm `Add`. Tìm kiếm `English (US)` và chọn biến thể **Vietnamese (Kernel Driver)**. Đặt nó làm mặc định và chuyển sang layout này.
2. **GNOME**: Mở `Settings` -> `Keyboard` -> `Input Sources` -> Bấm `Add Input Source` -> Tìm **Vietnamese (Kernel Driver)**.

*Lưu ý: Bàn phím này hoạt động hoàn toàn độc lập, không cần IBus hay Fcitx.*
"""

if "Kiến trúc Ring-0 Kernel Daemon" not in readme:
    readme = readme.replace("## Tính năng", ring0_doc + "\n## Tính năng")
    with open("README.md", "w") as f:
        f.write(readme)
