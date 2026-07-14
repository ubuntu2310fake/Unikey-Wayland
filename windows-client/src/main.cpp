#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QSettings>
#include "mainwindow.h"
#include "trayicon.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

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
    HMODULE hMod = LoadLibraryW(L"bamboo.dll");
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
std::unordered_set<DWORD> eaten_keys;
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
bool g_viet_mode = true;
HHOOK g_hHook = NULL;
MainWindow* g_mainWindow = nullptr;
class TrayIcon* g_trayIcon = nullptr;

// Tracking state for Ctrl+Shift hotkey
bool g_other_key_pressed = false;
bool g_ctrl_shift_down = false;

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pkb = (KBDLLHOOKSTRUCT*)lParam;

        if (pkb->dwExtraInfo == BAMBOO_MAGIC_INJECT) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        if (pkb->vkCode == VK_PACKET) {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        bool isCtrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        bool isAlt = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
        bool isShift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        bool isWin = (GetAsyncKeyState(VK_LWIN) & 0x8000) != 0 || (GetAsyncKeyState(VK_RWIN) & 0x8000) != 0;

        // Toggle logic
        int switchKeyConfig = g_mainWindow ? g_mainWindow->getSwitchKey() : 0;
        
        if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN) {
            if (pkb->vkCode != VK_CONTROL && pkb->vkCode != VK_SHIFT && pkb->vkCode != VK_LCONTROL && pkb->vkCode != VK_RCONTROL && pkb->vkCode != VK_LSHIFT && pkb->vkCode != VK_RSHIFT) {
                g_other_key_pressed = true;
            }
            if (switchKeyConfig == 0 && isCtrl && isShift && !g_other_key_pressed) {
                g_ctrl_shift_down = true;
            }
            // Alt + Z
            if (switchKeyConfig == 1 && isAlt && pkb->vkCode == 'Z') {
                g_viet_mode = !g_viet_mode;
                if (g_mainWindow) g_mainWindow->setVietMode(g_viet_mode);
                // Update icon implicitly via timer or direct call if available
                return 1; // Eat the Z key
            }
        }
        
        if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
            if (eaten_keys.count(pkb->vkCode)) {
                eaten_keys.erase(pkb->vkCode);
                return 1;
            }
            if (pkb->vkCode == VK_CONTROL || pkb->vkCode == VK_SHIFT || pkb->vkCode == VK_LCONTROL || pkb->vkCode == VK_RCONTROL || pkb->vkCode == VK_LSHIFT || pkb->vkCode == VK_RSHIFT) {
                if (switchKeyConfig == 0 && g_ctrl_shift_down && !g_other_key_pressed) {
                    // Check if both are now released or one is released? Usually toggle on release of either.
                    g_viet_mode = !g_viet_mode;
                    if (g_mainWindow) g_mainWindow->setVietMode(g_viet_mode);
                    g_ctrl_shift_down = false; // reset
                }
                if (!isCtrl && !isShift) {
                    g_other_key_pressed = false;
                    g_ctrl_shift_down = false;
                }
            } else {
                if (!isCtrl && !isShift) g_other_key_pressed = false;
            }
        }

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (isCtrl || isAlt || isWin || !g_viet_mode) {
                if (_composedWord != "") {
                    _composedWord = "";
                    Bamboo_Reset();
                }
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
                    bool macro_applied = false;
                    if (g_mainWindow && g_mainWindow->isMacroEnabled()) {
                        auto macros = g_mainWindow->getMacros();
                        if (macros.find(_composedWord) != macros.end()) {
                            std::string macro_val = macros[_composedWord];
                            int char_backs = utf8_strlen(_composedWord);
                            
                            std::wstring macro_wstr = utf8_to_wstring(macro_val);
                            if (c != '\b' && c != 0) {
                                macro_wstr += (wchar_t)c;
                            }
                            // CƠ CHẾ LÁCH LỖI CHROME OMNIBOX
                            if (char_backs > 0 && c != '\b' && c != 0) {
                                SendInputString(0, std::wstring(1, (wchar_t)c));
                                SendInputString(char_backs + 1, macro_wstr);
                            } else {
                                SendInputString(char_backs, macro_wstr);
                            }
                            macro_applied = true;
                        }
                    }
                    _composedWord = "";
                    Bamboo_Reset();
                    if (macro_applied) {
                        eaten_keys.insert(pkb->vkCode);
                        return 1; // Eat the terminating key if macro is triggered
                    }
                    return CallNextHookEx(NULL, nCode, wParam, lParam);
                } else {
                    Bamboo_ProcessKey(c);
                }

                char* preedit_ptr = Bamboo_GetPreeditString ? Bamboo_GetPreeditString() : nullptr;
                std::string new_composed = preedit_ptr ? preedit_ptr : "";
                // Do not free(preedit_ptr) on Windows due to cross-DLL heap corruption.

                // Native Wayland Surrounding Diff Logic
                int common_bytes = 0;
                size_t i = 0, j = 0;
                while (i < old_composed.length() && j < new_composed.length()) {
                    int len1 = 1, len2 = 1;
                    while (i + len1 < old_composed.length() && (old_composed[i + len1] & 0xC0) == 0x80) len1++;
                    while (j + len2 < new_composed.length() && (new_composed[j + len2] & 0xC0) == 0x80) len2++;
                    
                    if (len1 == len2 && old_composed.substr(i, len1) == new_composed.substr(j, len2)) {
                        common_bytes += len1;
                        i += len1;
                        j += len2;
                    } else break;
                }

                int char_backs = 0;
                size_t p = common_bytes;
                while (p < old_composed.length()) {
                    int len = 1;
                    while (p + len < old_composed.length() && (old_composed[p + len] & 0xC0) == 0x80) len++;
                    char_backs++;
                    p += len;
                }

                std::wstring to_insert = utf8_to_wstring(new_composed.substr(common_bytes));

                if (char_backs > 0 || !to_insert.empty()) {
                    // CƠ CHẾ LÁCH LỖI CHROME OMNIBOX
                    if (char_backs > 0 && c != '\b' && c != 0) {
                        SendInputString(0, std::wstring(1, (wchar_t)c));
                        SendInputString(char_backs + 1, to_insert);
                    } else {
                        SendInputString(char_backs, to_insert);
                    }
                }

                _composedWord = new_composed;
                eaten_keys.insert(pkb->vkCode);
                return 1; // EAT the key!
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// ---------------------------------------------------------
// Main Entry
// ---------------------------------------------------------
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // Đảm bảo thư mục lưu trữ cấu hình AppData tồn tại
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Unikey";
    QDir().mkpath(path);

    if (LoadBambooDLL()) {
        if (Bamboo_Init) Bamboo_Init();
    }

    g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, GetModuleHandle(NULL), 0);

    g_mainWindow = new MainWindow(&g_viet_mode, false);
    g_trayIcon = new TrayIcon(&g_viet_mode, g_mainWindow);

    // Kiểm tra cài đặt showOnStartup
    if (g_mainWindow->isShowOnStartupEnabled()) {
        g_mainWindow->show();
    }

    int ret = app.exec();

    if (g_hHook) {
        UnhookWindowsHookEx(g_hHook);
    }

    return ret;
}
