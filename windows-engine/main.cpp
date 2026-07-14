#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

// ---------------------------------------------------------
// Bamboo Wrapper definitions
// ---------------------------------------------------------
typedef void (*PFN_Bamboo_Init)();
typedef void (*PFN_Bamboo_ProcessKey)(uint32_t key);
typedef char* (*PFN_Bamboo_GetPreeditString)();
typedef bool (*PFN_Bamboo_CanProcessKey)(uint32_t key);
typedef void (*PFN_Bamboo_RemoveLastChar)();
typedef void (*PFN_Bamboo_Reset)();

PFN_Bamboo_Init Bamboo_Init = nullptr;
PFN_Bamboo_ProcessKey Bamboo_ProcessKey = nullptr;
PFN_Bamboo_GetPreeditString Bamboo_GetPreeditString = nullptr;
PFN_Bamboo_CanProcessKey Bamboo_CanProcessKey = nullptr;
PFN_Bamboo_RemoveLastChar Bamboo_RemoveLastChar = nullptr;
PFN_Bamboo_Reset Bamboo_Reset = nullptr;

bool LoadBambooDLL() {
    HMODULE hMod = LoadLibraryA("bamboo.dll");
    if (!hMod) return false;
    Bamboo_Init = (PFN_Bamboo_Init)GetProcAddress(hMod, "Bamboo_Init");
    Bamboo_ProcessKey = (PFN_Bamboo_ProcessKey)GetProcAddress(hMod, "Bamboo_ProcessKey");
    Bamboo_GetPreeditString = (PFN_Bamboo_GetPreeditString)GetProcAddress(hMod, "Bamboo_GetPreeditString");
    Bamboo_CanProcessKey = (PFN_Bamboo_CanProcessKey)GetProcAddress(hMod, "Bamboo_CanProcessKey");
    Bamboo_RemoveLastChar = (PFN_Bamboo_RemoveLastChar)GetProcAddress(hMod, "Bamboo_RemoveLastChar");
    Bamboo_Reset = (PFN_Bamboo_Reset)GetProcAddress(hMod, "Bamboo_Reset");
    return Bamboo_Init && Bamboo_ProcessKey && Bamboo_GetPreeditString && Bamboo_CanProcessKey;
}

// ---------------------------------------------------------
// Utility Functions
// ---------------------------------------------------------
int utf8_strlen(const std::string& str) {
    int len = 0;
    for (size_t i = 0; i < str.length(); i++) {
        if ((str[i] & 0xC0) != 0x80) len++;
    }
    return len;
}

std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// ---------------------------------------------------------
// Engine State & SendInput
// ---------------------------------------------------------
std::string _composedWord = "";
const ULONG_PTR BAMBOO_MAGIC_INJECT = 0xBAAB00;

void SendInputString(int backspaces, const std::wstring& str) {
    if (backspaces <= 0 && str.empty()) return;
    std::vector<INPUT> inputs;
    
    // Add backspaces
    for (int i = 0; i < backspaces; i++) {
        INPUT in = {};
        in.type = INPUT_KEYBOARD;
        in.ki.wVk = VK_BACK;
        in.ki.dwExtraInfo = BAMBOO_MAGIC_INJECT;
        inputs.push_back(in);
        in.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(in);
    }
    
    // Add unicode string
    for (size_t i = 0; i < str.length(); i++) {
        INPUT in = {};
        in.type = INPUT_KEYBOARD;
        in.ki.wScan = str[i];
        in.ki.dwFlags = KEYEVENTF_UNICODE;
        in.ki.dwExtraInfo = BAMBOO_MAGIC_INJECT;
        inputs.push_back(in);
        in.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        inputs.push_back(in);
    }
    
    SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));
}

// ---------------------------------------------------------
// Global Keyboard Hook
// ---------------------------------------------------------
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pkb = (KBDLLHOOKSTRUCT*)lParam;

        // Bỏ qua các phím do chính chúng ta tạo ra (tránh vòng lặp vô tận)
        if (pkb->dwExtraInfo == BAMBOO_MAGIC_INJECT) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        // Bỏ qua phím giả lập dạng Unicode (VK_PACKET)
        if (pkb->vkCode == VK_PACKET) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            bool isCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
            bool isAlt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;

            if (isCtrl || isAlt) {
                // Nhả bộ gõ nếu bấm phím tắt
                _composedWord = "";
                Bamboo_Reset();
                return CallNextHookEx(NULL, nCode, wParam, lParam);
            }

            uint32_t c = 0;
            if (pkb->vkCode == VK_BACK) {
                c = '\b';
            } else if (pkb->vkCode == VK_SPACE) {
                c = ' ';
            } else if (pkb->vkCode == VK_RETURN) {
                c = '\n';
            } else {
                BYTE ks[256] = {0};
                ks[VK_SHIFT] = (GetKeyState(VK_SHIFT) & 0x8000) ? 0x80 : 0;
                ks[VK_CAPITAL] = (GetKeyState(VK_CAPITAL) & 0x0001) ? 0x01 : 0;
                WCHAR wch[4] = {0};
                if (ToUnicode(pkb->vkCode, pkb->scanCode, ks, wch, 4, 0) > 0) {
                    if (wch[0] < 128) c = (char)wch[0];
                }
            }

            if (c != 0) {
                std::string old_composed = _composedWord;

                if (c == '\b') {
                    if (_composedWord.empty()) {
                        return CallNextHookEx(NULL, nCode, wParam, lParam);
                    }
                    Bamboo_RemoveLastChar();
                } else if (!Bamboo_CanProcessKey(c)) {
                    _composedWord = "";
                    Bamboo_Reset();
                    return CallNextHookEx(NULL, nCode, wParam, lParam);
                } else {
                    Bamboo_ProcessKey(c);
                }

                char* preedit_ptr = Bamboo_GetPreeditString ? Bamboo_GetPreeditString() : nullptr;
                std::string new_composed = preedit_ptr ? preedit_ptr : "";

                size_t common_bytes = 0;
                while (common_bytes < old_composed.length() && common_bytes < new_composed.length() &&
                       old_composed[common_bytes] == new_composed[common_bytes]) {
                    common_bytes++;
                }
                while (common_bytes > 0 && (old_composed[common_bytes] & 0xC0) == 0x80) {
                    common_bytes--;
                }

                int char_backs = 0;
                if (old_composed.length() > common_bytes) {
                    char_backs = utf8_strlen(old_composed.substr(common_bytes));
                }
                std::wstring to_insert = utf8_to_wstring(new_composed.substr(common_bytes));

                // LOGGING
                FILE* f = fopen("C:\\UnikeyWayland_Debug.log", "a");
                if (f) {
                    fprintf(f, "Key: %c, old: '%s', new: '%s', common: %zu, char_backs: %d\n", 
                            c == '\b' ? 'B' : c, old_composed.c_str(), new_composed.c_str(), common_bytes, char_backs);
                    fclose(f);
                }

                if (char_backs > 0 || !to_insert.empty()) {
                    SendInputString(char_backs, to_insert);
                }

                _composedWord = new_composed;
                return 1; // EAT the key! Không cho ứng dụng nhận phím gốc nữa!
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// ---------------------------------------------------------
// Main Entry
// ---------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    if (!LoadBambooDLL()) {
        MessageBoxA(NULL, "Failed to load bamboo.dll", "Error", MB_ICONERROR);
        return 1;
    }
    if (Bamboo_Init) Bamboo_Init();

    HHOOK hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);
    if (!hHook) {
        MessageBoxA(NULL, "Failed to install hook!", "Error", MB_ICONERROR);
        return 1;
    }

    // Vòng lặp bắt sự kiện để Hook hoạt động liên tục
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hHook);
    return 0;
}
