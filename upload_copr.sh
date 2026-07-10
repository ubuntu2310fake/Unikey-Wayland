#!/bin/bash
# Hướng dẫn sử dụng:
# ./upload_copr.sh
# Yêu cầu: Bạn cần cung cấp sẵn API token tại file cấu hình `~/.config/copr`
set -e

# Đảm bảo lệnh copr đã được cài đặt
if ! command -v copr &> /dev/null; then
    echo "Lỗi: Không tìm thấy lệnh 'copr'."
    echo "Vui lòng cài đặt: sudo dnf install copr-cli"
    exit 1
fi

PROJECT_NAME="truonghieu/Unikey-Wayland"
SRPM_FILE=$(ls releases/*.src.rpm 2>/dev/null | head -n 1)

if [ -z "$SRPM_FILE" ] || [ ! -f "$SRPM_FILE" ]; then
    echo "Lỗi: Không tìm thấy file SRPM nguồn trong thư mục releases/"
    echo "Bạn cần chạy quy trình đóng gói RPM/SRPM trước khi upload."
    exit 1
fi

echo "================================================="
echo ">>> Bắt đầu đẩy bản build lên Fedora Copr..."
echo "Dự án: $PROJECT_NAME"
echo "File:  $SRPM_FILE"
echo "================================================="

# Thực hiện lệnh build trên hệ thống Copr
copr build "$PROJECT_NAME" "$SRPM_FILE"

echo ""
echo ">>> Lệnh đẩy lên Copr đã được thực thi thành công!"
echo ">>> Hãy kiểm tra tiến trình build trên giao diện web Copr của bạn."
