#!/bin/bash
echo "Đang đóng tiến trình UniKey chạy ngầm (nếu có)..."
killall uk-wayland-im 2>/dev/null

echo "Đang khởi chạy UniKey Wayland ở chế độ Debug..."
echo "Hãy mở một ứng dụng khác (ví dụ KWrite) và gõ thử vài phím để xem log hiển thị tại đây nhé!"
echo "---------------------------------------------------"
WAYLAND_DEBUG=1 /home/truonghieu/Downloads/Uk362/build/uk-wayland-im
