#ifndef WINDOWS_STUB_H
#define WINDOWS_STUB_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

// Basic Windows types
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef uint32_t UINT;
typedef void* PVOID;
typedef void* LPVOID;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HINSTANCE;
typedef void* HICON;
typedef void* HHOOK;
typedef long LRESULT;
typedef uintptr_t WPARAM;
typedef intptr_t LPARAM;
typedef unsigned char UCHAR;
typedef char TCHAR;
typedef int BOOL;

#define CALLBACK
#define WINAPI
#define TRUE 1
#define FALSE 0

#define LOBYTE(w) ((BYTE)(((DWORD)(w)) & 0xff))
#define HIBYTE(w) ((BYTE)((((DWORD)(w)) >> 8) & 0xff))

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

typedef short SHORT;
#define VK_SHIFT 0x10
#define VK_CAPITAL 0x14
#define CP_US_ANSI 1252

// Dummy functions
inline SHORT GetKeyState(int nVirtKey) { return 0; }
inline int WideCharToMultiByte(UINT, DWORD, const WORD*, int, char*, int, const char*, BOOL*) { return 0; }
extern BYTE KeyState[256];

#include <stdlib.h>

// Dummy struct to satisfy CodeInfo
#ifndef MAX_MACRO_ITEMS
#define MAX_MACRO_ITEMS 1024
#endif

#define _TEXT(x) (TCHAR*)x
#define TEXT(x) (TCHAR*)x

// Some string functions used in Windows
#define lstrcpy strcpy
#define lstrcpyn strncpy
#define lstrlen strlen
#define lstrcat strcat
#define lstrcmp strcmp
#define lstrcmpi strcasecmp

#endif // WINDOWS_STUB_H
