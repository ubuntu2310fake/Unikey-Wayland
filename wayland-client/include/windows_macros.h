#ifndef WINDOWS_MACROS_H
#define WINDOWS_MACROS_H

#undef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#undef min
#define min(a,b) (((a) < (b)) ? (a) : (b))

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))

#include <ctype.h>
#include <wchar.h>

#endif
