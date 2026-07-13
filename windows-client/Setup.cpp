#include <windows.h>
#include <shlobj.h>
#include <shlguid.h>
#include <shlwapi.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "user32.lib")

// Hàm tạo Shortcut (.lnk) bằng COM
HRESULT CreateLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc) {
    HRESULT hres;
    IShellLinkW* psl;
    
    // Khởi tạo interface IShellLink
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl);
    if (SUCCEEDED(hres)) {
        IPersistFile* ppf;
        
        // Thiết lập đường dẫn tới file chạy và mô tả
        psl->SetPath(lpszPathObj);
        psl->SetDescription(lpszDesc);
        
        // Trỏ thư mục làm việc về thư mục cha của file chạy
        std::wstring pathStr(lpszPathObj);
        size_t lastSlash = pathStr.find_last_of(L"\\/");
        if (lastSlash != std::wstring::npos) {
            psl->SetWorkingDirectory(pathStr.substr(0, lastSlash).c_str());
        }

        // Truy vấn IPersistFile để lưu shortcut
        hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hres)) {
            // Lưu Shortcut
            hres = ppf->Save(lpszPathLink, TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    return hres;
}

// Chạy tar.exe ở chế độ ẩn để giải nén file 7z
bool RunTar(const std::wstring& zipPath, const std::wstring& destDir) {
    // Câu lệnh: tar.exe -xf "path_to_7z" -C "dest_dir"
    std::wstring cmd = L"tar.exe -xf \"" + zipPath + L"\" -C \"" + destDir + L"\"";
    
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // Ẩn cửa sổ cmd của tar
    ZeroMemory(&pi, sizeof(pi));
    
    std::vector<wchar_t> cmdBuffer(cmd.begin(), cmd.end());
    cmdBuffer.push_back(L'\0');
    
    if (CreateProcessW(NULL, cmdBuffer.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return exitCode == 0;
    }
    return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Hỏi người dùng xem có muốn cài đặt không
    int msgBoxID = MessageBoxW(NULL,
        L"Chương trình sẽ cài đặt Unikey-Wayland (Windows Edition) vào máy tính của bạn.\n\nĐường dẫn cài đặt mặc định: C:\\Program Files\\UnikeyWayland\n\nBạn có muốn tiếp tục?",
        L"Unikey-Wayland Setup",
        MB_YESNO | MB_ICONINFORMATION
    );
    
    if (msgBoxID != IDYES) {
        return 0;
    }
    
    // 2. Tìm file setup.7z trong thư mục chạy hiện tại
    WCHAR modulePath[MAX_PATH];
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);
    PathRemoveFileSpecW(modulePath);
    std::wstring currentDir(modulePath);
    std::wstring zipPath = currentDir + L"\\setup.7z";
    
    if (!PathFileExistsW(zipPath.c_str())) {
        MessageBoxW(NULL,
            L"Lỗi: Không tìm thấy file 'setup.7z' nằm cùng thư mục với trình cài đặt này!\nVui lòng đảm bảo file 'setup.7z' nằm cạnh file 'Setup.exe'.",
            L"Unikey-Wayland Setup - Lỗi",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }
    
    // 3. Tạo thư mục cài đặt C:\Program Files\UnikeyWayland
    std::wstring destDir = L"C:\\Program Files\\UnikeyWayland";
    HRESULT hr = SHCreateDirectoryExW(NULL, destDir.c_str(), NULL);
    if (hr != ERROR_SUCCESS && hr != ERROR_ALREADY_EXISTS) {
        MessageBoxW(NULL,
            L"Lỗi: Không thể tạo thư mục cài đặt tại C:\\Program Files\\UnikeyWayland!\nVui lòng chạy lại trình cài đặt bằng quyền Administrator.",
            L"Unikey-Wayland Setup - Lỗi",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }
    
    // 4. Tiến hành giải nén setup.7z qua tar.exe
    if (!RunTar(zipPath, destDir)) {
        MessageBoxW(NULL,
            L"Lỗi: Quá trình giải nén file 'setup.7z' thất bại!\nCó thể file 7z bị lỗi hoặc thư mục cài đặt đang bị khóa.",
            L"Unikey-Wayland Setup - Lỗi",
            MB_OK | MB_ICONERROR
        );
        return 1;
    }
    
    // 5. Tạo Shortcuts (Desktop và Start Menu)
    CoInitialize(NULL);
    
    std::wstring targetExe = destDir + L"\\UnikeyWayland.exe";
    
    // Shortcut ngoài Desktop (cho mọi User)
    WCHAR desktopPath[MAX_PATH];
    if (SHGetSpecialFolderPathW(NULL, desktopPath, CSIDL_COMMON_DESKTOPDIRECTORY, FALSE)) {
        std::wstring desktopLnk = std::wstring(desktopPath) + L"\\Unikey-Wayland.lnk";
        CreateLink(targetExe.c_str(), desktopLnk.c_str(), L"Unikey-Wayland (Windows Edition)");
    }
    
    // Shortcut trong Start Menu (cho mọi User)
    WCHAR programsPath[MAX_PATH];
    if (SHGetSpecialFolderPathW(NULL, programsPath, CSIDL_COMMON_PROGRAMS, FALSE)) {
        std::wstring startMenuFolder = std::wstring(programsPath) + L"\\Unikey-Wayland";
        SHCreateDirectoryExW(NULL, startMenuFolder.c_str(), NULL);
        std::wstring startMenuLnk = startMenuFolder + L"\\Unikey-Wayland.lnk";
        CreateLink(targetExe.c_str(), startMenuLnk.c_str(), L"Unikey-Wayland (Windows Edition)");
    }
    
    CoUninitialize();
    
    // 6. Kích hoạt chạy thử bộ gõ luôn
    ShellExecuteW(NULL, L"open", targetExe.c_str(), NULL, destDir.c_str(), SW_SHOWNORMAL);
    
    // 7. Báo thành công
    MessageBoxW(NULL,
        L"Chúc mừng! Unikey-Wayland (Windows Edition) đã được cài đặt thành công.\n\nBiểu tượng khởi chạy đã được tạo ngoài Desktop và Start Menu.",
        L"Unikey-Wayland Setup - Hoàn tất",
        MB_OK | MB_ICONINFORMATION
    );
    
    return 0;
}
