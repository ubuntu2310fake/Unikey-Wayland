# Quy định và Lưu ý về Phiên bản & Cơ chế OTA Update (Windows)

Tài liệu này lưu trữ các lưu ý đặc thù về cơ chế Magic Bytes, các file định nghĩa phiên bản (version) và luồng hoạt động của tính năng OTA Update để phục vụ cho việc bảo trì, phát triển ứng dụng sau này.

---

## 1. Cơ chế Magic Bytes trong file EXE
Để tránh việc phân tích cú pháp phức tạp cấu trúc PE của Windows Executable hoặc lưu file config ngoài, phiên bản của ứng dụng `UnikeyWayland.exe` được ghi trực tiếp vào cuối (EOF) của tệp nhị phân trong quá trình build CI/CD.

*   **Định dạng chuỗi đánh dấu:** `[UKW_VERSION]X.Y.Z[/UKW_VERSION]` (Ví dụ: `[UKW_VERSION]2.0.1[/UKW_VERSION]`).
*   **Tiến trình chèn (CI/CD):** Sau khi build xong tệp `UnikeyWayland.exe`, tiến trình GitHub Actions (`.github/workflows/ci-cd.yml`) chạy lệnh ghi đè chuỗi trên trước khi đóng gói thành file `.7z` và `.zip`:
    ```cmd
    echo | set /p="[UKW_VERSION]%VERSION%[/UKW_VERSION]" >> dist\UnikeyWayland.exe
    ```
*   **Đọc phiên bản trong App:** Ứng dụng đọc 1024 bytes cuối cùng của chính file thực thi (`applicationFilePath()`) và thực hiện quét Regex hoặc tìm vị trí cụm từ khóa để lấy thông tin phiên bản hiện tại.

---

## 2. Các file khai báo phiên bản (Version Files)
Khi nâng cấp phiên bản (ví dụ từ `2.0.1` lên bản tiếp theo), **bắt buộc** phải thay đổi đồng loạt giá trị version ở các tệp sau để CI/CD và bộ cài đặt chạy đúng:

1.  **`package_arch.sh`**: Thay đổi biến `PKGVER="2.0.1"` (Đây là file nguồn chính mà GitHub Actions đọc để tạo tên tag phát hành).
2.  **`unikey-wayland.spec`**: Dòng `Version: 2.0.1`.
3.  **`upload_ppa.sh`**: Dòng `VERSION="2.0.1"`.
4.  **`debian/changelog`**: Dòng đầu tiên (ví dụ: `unikey-wayland (2.0.1~ppa1~noble) noble; urgency=medium`).
5.  **`CHANGELOG.md`**: Cập nhật lịch sử thay đổi dưới dạng `## [2.0.1] - YYYY-MM-DD`.

---

## 3. Luồng Cập nhật tự động (OTA Update) trên Windows
*   **Nguồn kiểm tra:** API GitHub Releases: `https://api.github.com/repos/ubuntu2310fake/Unikey-Wayland/releases/latest`.
*   **Kiến trúc tải về:** 
    *   `ARM64`: Tải tệp `UnikeyWayland-Windows-ARM64.zip`.
    *   `x64` (Khác): Tải tệp `UnikeyWayland-Windows-x64.zip`.
*   **Quy trình thực thi:**
    1.  Tải file `.zip` tương ứng về thư mục tạm `%TEMP%/UnikeyWayland-Update.zip`.
    2.  Giải nén file `.zip` bằng PowerShell:
        ```powershell
        Expand-Archive -Path '%TEMP%/UnikeyWayland-Update.zip' -DestinationPath '%TEMP%/UnikeyWayland-Update' -Force
        ```
    3.  Khởi chạy độc lập tệp `%TEMP%/UnikeyWayland-Update/setup.bat` (yêu cầu quyền Admin để ghi đè vào `C:\Program Files\UnikeyWayland`).
    4.  Ứng dụng hiện tại lập tức đóng tiến trình (`QApplication::quit()`) để tránh tranh chấp khóa ghi tệp khi `setup.bat` giải nén.
