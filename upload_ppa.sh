#!/bin/bash
# Hướng dẫn sử dụng:
# ./upload_ppa.sh
# Kịch bản này tự động tạo source package, ký số GPG và upload lên FTP của Launchpad.
set -e

PKGNAME="unikey-wayland"
VERSION="1.0.3"
REVISION="${PPA_REVISION:-ppa1}"
GPG_KEY="${PPA_GPG_KEY_ID}"
FTP_SERVER="${PPA_FTP_SERVER}"
FTP_PATH="${PPA_FTP_PATH}"

# Danh sách các phiên bản Ubuntu được hỗ trợ (chạy các bản bị lỗi chữ ký trước đó, bỏ qua jammy đã chạy ngon)
SUITES=("noble" "oracular" "plucky" "questing" "resolute")

echo ">>> Bắt đầu quá trình CI/CD Upload PPA..."

# Sao lưu changelog gốc ra ngoài /tmp để tránh bị debuild clean xóa mất
cp debian/changelog /tmp/changelog.bak

# Gom toàn bộ các file nguồn của tất cả các hệ điều hành để upload trong 1 phiên duy nhất
ALL_UPLOAD_LIST=""

for SUITE in "${SUITES[@]}"; do
    echo ""
    echo "================================================="
    echo ">>> Đóng gói cho Ubuntu: ${SUITE}"
    echo "================================================="
    FULL_VER="${VERSION}~${REVISION}~${SUITE}"
    
    # Khởi tạo changelog động theo từng hệ điều hành
    cat <<EOF > debian/changelog
${PKGNAME} (${FULL_VER}) ${SUITE}; urgency=medium

  * CI/CD Automated Source Build for ${SUITE}.

 -- Truong Hieu <11Leak1234@protonmail.com>  $(date -R)
EOF

    # 1. Build source package (Dành riêng cho PPA)
    debuild -S -uc -us -d
    
    CHANGES_FILE="../${PKGNAME}_${FULL_VER}_source.changes"
    
    # 2. Ký số GPG lên file changes
    echo ">>> Ký số GPG..."
    debsign -k${GPG_KEY} "${CHANGES_FILE}"
    
    # 3. Trích xuất danh sách các tệp con cần upload từ file .changes
    FILES_TO_UPLOAD=$(awk '/^Files:/ {flag=1; next} /^ / {if(flag) print $5} /^[^ ]/ {if(flag) flag=0}' "${CHANGES_FILE}")
    
    # Gom tất cả các file nguồn của suite hiện tại vào danh sách tổng
    for FILE in $FILES_TO_UPLOAD; do
        if [ -f "../${FILE}" ]; then
            ALL_UPLOAD_LIST="${ALL_UPLOAD_LIST} ../${FILE}"
        fi
    done
    ALL_UPLOAD_LIST="${ALL_UPLOAD_LIST} ${CHANGES_FILE}"
done

echo ""
echo "================================================="
echo ">>> Đẩy TẤT CẢ các bản đóng gói lên Launchpad qua 1 phiên FTP duy nhất..."
echo "================================================="
python3 upload_ftp.py "$FTP_SERVER" "$FTP_PATH" $ALL_UPLOAD_LIST

# Khôi phục changelog gốc
mv /tmp/changelog.bak debian/changelog

echo ""
echo ">>> Hoàn tất toàn bộ quy trình đẩy PPA CI/CD!"
