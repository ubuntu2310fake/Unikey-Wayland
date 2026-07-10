#!/bin/bash
# Hướng dẫn sử dụng:
# ./upload_ppa.sh
# Kịch bản này tự động tạo source package, ký số GPG và upload lên FTP của Launchpad.
set -e

PKGNAME="unikey-wayland"
VERSION="1.0.3"
REVISION="ppa1"
GPG_KEY="${PPA_GPG_KEY_ID}"
FTP_SERVER="${PPA_FTP_SERVER}"
FTP_PATH="${PPA_FTP_PATH}"

# Danh sách các phiên bản Ubuntu được hỗ trợ
SUITES=("jammy" "noble" "oracular" "plucky" "questing" "resolute")

echo ">>> Bắt đầu quá trình CI/CD Upload PPA..."

# Sao lưu changelog gốc
cp debian/changelog debian/changelog.bak

for SUITE in "${SUITES[@]}"; do
    echo ""
    echo "================================================="
    echo ">>> Đóng gói và Upload cho Ubuntu: ${SUITE}"
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
    
    echo ">>> Uploading lên Launchpad qua FTP (curl)..."
    # Upload các tệp nguồn trước (.dsc, .tar.xz, .buildinfo)
    for FILE in $FILES_TO_UPLOAD; do
        if [ -f "../${FILE}" ]; then
            echo " -> Uploading ${FILE}..."
            curl -s -T "../${FILE}" "ftp://${FTP_SERVER}/${FTP_PATH}/"
        fi
    done
    
    # Upload file .changes CUỐI CÙNG để trigger tiến trình build trên server Launchpad
    echo " -> Uploading $(basename "${CHANGES_FILE}")..."
    curl -s -T "${CHANGES_FILE}" "ftp://${FTP_SERVER}/${FTP_PATH}/"
done

# Khôi phục changelog gốc
mv debian/changelog.bak debian/changelog

echo ""
echo ">>> Hoàn tất toàn bộ quy trình đẩy PPA CI/CD!"
