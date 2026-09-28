# Hướng dẫn thiết lập trên KDE Plasma (v3.0.0+)

Kể từ phiên bản 3.0.0, Unikey-Wayland sử dụng kiến trúc Kernel Ring 0, hoạt động ở cấp độ phần cứng. Điều này giúp bộ gõ tương thích 100% với KDE Plasma mà không cần cài đặt IBus hay Fcitx.

## Các bước thiết lập
1. Đảm bảo Kernel Module `ukw_driver` đã được cài đặt và service `ukw` đang chạy.
2. Mở ứng dụng **System Settings** của KDE.
3. Chuyển đến mục **Keyboard** -> **Layouts**.
4. Chọn **Add Layout**. Tìm layout có tên `Vietnamese (Kernel Driver)` (hoặc mã `ukw`).
5. Kéo layout này lên vị trí ưu tiên.
6. Khi muốn gõ tiếng Việt, chỉ cần đảm bảo biểu tượng Unikey trên System Tray hiện chữ `V`. (Để chuyển đổi nhanh, nhấn `Right Alt`).
