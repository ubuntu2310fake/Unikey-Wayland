#include <windows.h>
#include <msctf.h>
#include <olectl.h>
#include <strsafe.h>
#include <Shlobj.h>
#include <string>
#include <map>
#include <fstream>
#include <stdint.h>
#include <stdbool.h>
// Bamboo API Pointers
typedef void (*PFN_Bamboo_Init)();
typedef bool (*PFN_Bamboo_CanProcessKey)(uint32_t);
typedef void (*PFN_Bamboo_ProcessKey)(uint32_t);
typedef void (*PFN_Bamboo_RemoveLastChar)();
typedef char* (*PFN_Bamboo_GetPreeditString)();
typedef char* (*PFN_Bamboo_GetCommitString)();
typedef void (*PFN_Bamboo_Reset)();

PFN_Bamboo_Init Bamboo_Init = nullptr;
PFN_Bamboo_CanProcessKey Bamboo_CanProcessKey = nullptr;
PFN_Bamboo_ProcessKey Bamboo_ProcessKey = nullptr;
PFN_Bamboo_RemoveLastChar Bamboo_RemoveLastChar = nullptr;
PFN_Bamboo_GetPreeditString Bamboo_GetPreeditString = nullptr;
PFN_Bamboo_GetCommitString Bamboo_GetCommitString = nullptr;
PFN_Bamboo_Reset Bamboo_Reset = nullptr;
extern HINSTANCE g_hInst;

bool LoadBambooDLL() {
    char path[MAX_PATH];
    if (GetModuleFileNameA(g_hInst, path, MAX_PATH)) {
        char* lastSlash = strrchr(path, '\\');
        if (lastSlash) {
            strcpy(lastSlash + 1, "bamboo.dll");
        } else {
            strcpy(path, "bamboo.dll");
        }
    } else {
        strcpy(path, "bamboo.dll");
    }
    
    FILE* f = fopen("C:\\\\Users\\\\truonghieu\\\\tsf_log.txt", "a");
    if (f) {
        fprintf(f, "Trying to load: %s\\n", path);
    }
    HMODULE hMod = LoadLibraryA(path);
    if (f) {
        if (!hMod) {
            fprintf(f, "Failed! LastError = %lu\\n", GetLastError());
        } else {
            fprintf(f, "Success! hMod = %p\\n", hMod);
        }
        fclose(f);
    }
    if (!hMod) return false;
    Bamboo_Init = (PFN_Bamboo_Init)GetProcAddress(hMod, "Bamboo_Init");
    Bamboo_CanProcessKey = (PFN_Bamboo_CanProcessKey)GetProcAddress(hMod, "Bamboo_CanProcessKey");
    Bamboo_ProcessKey = (PFN_Bamboo_ProcessKey)GetProcAddress(hMod, "Bamboo_ProcessKey");
    Bamboo_RemoveLastChar = (PFN_Bamboo_RemoveLastChar)GetProcAddress(hMod, "Bamboo_RemoveLastChar");
    Bamboo_GetPreeditString = (PFN_Bamboo_GetPreeditString)GetProcAddress(hMod, "Bamboo_GetPreeditString");
    Bamboo_GetCommitString = (PFN_Bamboo_GetCommitString)GetProcAddress(hMod, "Bamboo_GetCommitString");
    Bamboo_Reset = (PFN_Bamboo_Reset)GetProcAddress(hMod, "Bamboo_Reset");
    return (Bamboo_Init != nullptr);
}

// GUIDs
const CLSID c_clsidTsfEngine = { 0x3d5b7c41, 0x1839, 0x4e8d, { 0xa5, 0x9f, 0x9c, 0xe2, 0xb6, 0xd4, 0x8a, 0x11 } };
const GUID c_guidProfile = { 0xf81c2b5d, 0x7160, 0x4a93, { 0x9b, 0x88, 0xc1, 0xd2, 0xe3, 0xf4, 0xa5, 0xb6 } };

HINSTANCE g_hInst = NULL;
LONG g_cRefDll = 0;

void DllAddRef() { InterlockedIncrement(&g_cRefDll); }
void DllRelease() { InterlockedDecrement(&g_cRefDll); }

std::map<std::string, std::string> g_macros;
bool g_macroEnabled = false;

void reload_macros() {
    g_macros.clear();
    g_macroEnabled = false;
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        std::string jsonPath = std::string(path) + "\\Unikey\\global.json";
        std::ifstream f(jsonPath);
        if (!f.is_open()) return;
        std::string json((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        
        if (json.find("\"macroEnabled\": true") != std::string::npos || json.find("\"macroEnabled\":true") != std::string::npos) {
            g_macroEnabled = true;
        }
        
        size_t macro_start = json.find("\"macros\":");
        if (macro_start != std::string::npos) {
            size_t obj_start = json.find("{", macro_start);
            size_t obj_end = json.find("}", macro_start);
            if (obj_start != std::string::npos && obj_end != std::string::npos) {
                std::string macros_str = json.substr(obj_start + 1, obj_end - obj_start - 1);
                size_t pos = 0;
                while ((pos = macros_str.find("\"", pos)) != std::string::npos) {
                    size_t key_end = macros_str.find("\"", pos + 1);
                    if (key_end == std::string::npos) break;
                    std::string key = macros_str.substr(pos + 1, key_end - pos - 1);
                    size_t colon = macros_str.find(":", key_end + 1);
                    if (colon == std::string::npos) break;
                    size_t val_start = macros_str.find("\"", colon + 1);
                    if (val_start == std::string::npos) break;
                    size_t val_end = macros_str.find("\"", val_start + 1);
                    if (val_end == std::string::npos) break;
                    std::string val = macros_str.substr(val_start + 1, val_end - val_start - 1);
                    g_macros[key] = val;
                    pos = val_end + 1;
                }
            }
        }
    }
}

int utf8_strlen(const std::string& str) {
    int len = 0;
    for (size_t i = 0; i < str.length(); ++i) {
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

class CCompositionSink : public ITfCompositionSink {
    LONG _cRef;
public:
    CCompositionSink() : _cRef(1) {}
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
        if (riid == IID_IUnknown || riid == IID_ITfCompositionSink) {
            *ppvObj = (ITfCompositionSink*)this;
            AddRef(); return S_OK;
        }
        *ppvObj = NULL; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0) delete this;
        return cRef;
    }
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition) {
        return S_OK;
    }
};

class CEditSession : public ITfEditSession {
    LONG _cRef;
    ITfContext* _pContext;
    ITfComposition** _ppComposition;
    bool _end_comp;
    std::wstring _replacement;
public:
    CEditSession(ITfContext* pContext, ITfComposition** ppComposition, bool end_comp, const std::wstring& replacement) 
        : _cRef(1), _pContext(pContext), _ppComposition(ppComposition), _end_comp(end_comp), _replacement(replacement) {
        _pContext->AddRef();
    }
    ~CEditSession() { _pContext->Release(); }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
        if (riid == IID_IUnknown || riid == IID_ITfEditSession) {
            *ppvObj = (ITfEditSession*)this;
            AddRef(); return S_OK;
        }
        *ppvObj = NULL; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0) delete this;
        return cRef;
    }
    STDMETHODIMP DoEditSession(TfEditCookie ec) {
        if (_end_comp) {
            if (*_ppComposition) {
                (*_ppComposition)->EndComposition(ec);
                (*_ppComposition)->Release();
                *_ppComposition = nullptr;
            }
            return S_OK;
        }
        ITfRange* pRange = nullptr;
        if (!*_ppComposition) {
            TF_SELECTION sel;
            ULONG cFetched;
            if (FAILED(_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &cFetched)) || cFetched != 1) return S_OK;
            ITfContextComposition* pContextComp = nullptr;
            if (SUCCEEDED(_pContext->QueryInterface(IID_ITfContextComposition, (void**)&pContextComp))) {
                CCompositionSink* pSink = new CCompositionSink();
                HRESULT hrComp = pContextComp->StartComposition(ec, sel.range, pSink, _ppComposition);
                
                FILE* f = fopen("C:/Users/truonghieu/tsf_comp.txt", "a");
                if (f) { fprintf(f, "StartComposition hr=%lx, comp=%p\\n", hrComp, *_ppComposition); fclose(f); }

                pSink->Release();
                pContextComp->Release();
            }
            pRange = sel.range;
            pRange->AddRef();
        } else {
            (*_ppComposition)->GetRange(&pRange);
        }
        if (pRange) {
            pRange->SetText(ec, 0, _replacement.c_str(), (ULONG)_replacement.length());
            TF_SELECTION sel;
            pRange->Clone(&sel.range);
            sel.style.ase = TF_AE_NONE;
            sel.style.fInterimChar = FALSE;
            sel.range->Collapse(ec, TF_ANCHOR_END);
            _pContext->SetSelection(ec, 1, &sel);
            sel.range->Release();
            pRange->Release();
        }
        return S_OK;
    }
};

class CTextService : public ITfTextInputProcessor, public ITfKeyEventSink {
    LONG _cRef;
    ITfThreadMgr* _pThreadMgr;
    TfClientId _clientId;
    std::string _composedWord;
    ITfComposition* _pComposition;
public:
    CTextService() : _cRef(1), _pThreadMgr(NULL), _clientId(TF_CLIENTID_NULL), _pComposition(NULL) { 
        DllAddRef(); 
        if (LoadBambooDLL() && Bamboo_Init) {
            Bamboo_Init(); 
        }
        reload_macros(); 
    }
    ~CTextService() { DllRelease(); }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
        if (riid == IID_IUnknown || riid == IID_ITfTextInputProcessor) {
            *ppvObj = (ITfTextInputProcessor*)this;
        } else if (riid == IID_ITfKeyEventSink) {
            *ppvObj = (ITfKeyEventSink*)this;
        } else { *ppvObj = NULL; return E_NOINTERFACE; }
        AddRef(); return S_OK;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() { LONG cRef = InterlockedDecrement(&_cRef); if (cRef == 0) delete this; return cRef; }

    STDMETHODIMP Activate(ITfThreadMgr* ptm, TfClientId tid) {
        FILE* f = fopen("C:\\\\Users\\\\truonghieu\\\\tsf_activate.txt", "a");
        if (f) { fprintf(f, "Activate called. tid=%d\\n", tid); fclose(f); }

        _pThreadMgr = ptm;
        _pThreadMgr->AddRef();
        _clientId = tid;
        
        ITfKeystrokeMgr* pKeystrokeMgr = NULL;
        if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr))) {
            HRESULT hr = pKeystrokeMgr->AdviseKeyEventSink(tid, this, TRUE);
            f = fopen("C:\\\\Users\\\\truonghieu\\\\tsf_activate.txt", "a");
            if (f) { fprintf(f, "AdviseKeyEventSink returned: %lx\\n", hr); fclose(f); }
            pKeystrokeMgr->Release();
        } else {
            f = fopen("C:\\\\Users\\\\truonghieu\\\\tsf_activate.txt", "a");
            if (f) { fprintf(f, "Failed to get KeystrokeMgr\\n"); fclose(f); }
        }
        return S_OK;
    }
    STDMETHODIMP Deactivate() {
        if (_pThreadMgr) {
            ITfKeystrokeMgr* pKeystrokeMgr;
            if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr))) {
                pKeystrokeMgr->UnadviseKeyEventSink(_clientId);
                pKeystrokeMgr->Release();
            }
            _pThreadMgr->Release(); _pThreadMgr = NULL;
        }
        return S_OK;
    }

    STDMETHODIMP OnSetFocus(BOOL fForeground) { return S_OK; }
    STDMETHODIMP OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) { 
        if (!Bamboo_CanProcessKey || !pic) { *pfEaten = FALSE; return S_OK; }
        BYTE ks[256]; GetKeyboardState(ks);
        if ((ks[VK_CONTROL] & 0x80) != 0 || (ks[VK_MENU] & 0x80) != 0 || wParam < 0x20 || wParam > 0x7E) { *pfEaten = FALSE; return S_OK; }
        uint32_t c = (uint32_t)wParam;
        if (!(ks[VK_SHIFT] & 0x80) && c >= 'A' && c <= 'Z') c += 32;
        *pfEaten = Bamboo_CanProcessKey(c) ? TRUE : FALSE;
        return S_OK; 
    }
    STDMETHODIMP OnTestKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) { *pfEaten = FALSE; return S_OK; }
    STDMETHODIMP OnKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) { *pfEaten = FALSE; return S_OK; }
    STDMETHODIMP OnPreservedKey(ITfContext* pic, REFGUID rguid, BOOL* pfEaten) { *pfEaten = FALSE; return S_OK; }

    STDMETHODIMP OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
        FILE* f = fopen("C:\\Users\\truonghieu\\tsf_keydown.txt", "a");
        if (f) { fprintf(f, "OnKeyDown: %u\n", (uint32_t)wParam); fclose(f); }

        if (!Bamboo_CanProcessKey || !pic) { *pfEaten = FALSE; return S_OK; }

        BYTE ks[256];
        GetKeyboardState(ks);
        WCHAR wch[4] = {0};
        char c = 0;
        if (ToUnicode(wParam, (lParam >> 16) & 0xFF, ks, wch, 4, 0) == 1) {
            if (wch[0] < 128) c = (char)wch[0];
        }

        if (wParam == VK_BACK) c = '\b';

        if (c == 0) {
            *pfEaten = FALSE;
            return S_OK;
        }

        if (c == '\b') {
            if (_composedWord.empty()) {
                *pfEaten = FALSE;
                return S_OK;
            }
            Bamboo_RemoveLastChar();
        } else if (!Bamboo_CanProcessKey(c)) {
            std::string current_word = _composedWord;
            _composedWord = "";
            Bamboo_Reset();

            if (_pComposition) {
                CEditSession* pEditSession = new CEditSession(pic, &_pComposition, true, L"");
                HRESULT hrSession;
                pic->RequestEditSession(_clientId, pEditSession, TF_ES_READWRITE | TF_ES_SYNC, &hrSession);
                pEditSession->Release();
            }

            *pfEaten = FALSE; 
            return S_OK;
        } else {
            Bamboo_ProcessKey(c);
        }

        char* preedit_ptr = Bamboo_GetPreeditString ? Bamboo_GetPreeditString() : nullptr;
        std::string new_composed = preedit_ptr ? preedit_ptr : "";
        
        if (new_composed.empty()) {
            if (_pComposition) {
                CEditSession* pEditSession = new CEditSession(pic, &_pComposition, true, L"");
                HRESULT hrSession;
                pic->RequestEditSession(_clientId, pEditSession, TF_ES_READWRITE | TF_ES_SYNC, &hrSession);
                pEditSession->Release();
            }
        } else {
            std::wstring to_insert = utf8_to_wstring(new_composed);
            CEditSession* pEditSession = new CEditSession(pic, &_pComposition, false, to_insert);
            HRESULT hrSession;
            pic->RequestEditSession(_clientId, pEditSession, TF_ES_READWRITE | TF_ES_SYNC, &hrSession);
            pEditSession->Release();
        }

        _composedWord = new_composed;
        *pfEaten = TRUE;
        return S_OK;
    }
};

class CClassFactory : public IClassFactory {
    LONG _cRef;
public:
    CClassFactory() : _cRef(1) { DllAddRef(); }
    ~CClassFactory() { DllRelease(); }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppvObj = (IClassFactory*)this; AddRef(); return S_OK;
        }
        *ppvObj = NULL; return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() { LONG cRef = InterlockedDecrement(&_cRef); if (cRef == 0) delete this; return cRef; }
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj) {
        if (pUnkOuter) return CLASS_E_NOAGGREGATION;
        CTextService* pService = new CTextService();
        if (!pService) return E_OUTOFMEMORY;
        HRESULT hr = pService->QueryInterface(riid, ppvObj);
        pService->Release();
        return hr;
    }
    STDMETHODIMP LockServer(BOOL fLock) { if (fLock) DllAddRef(); else DllRelease(); return S_OK; }
};

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObj) {
    if (IsEqualIID(rclsid, c_clsidTsfEngine)) {
        CClassFactory* pFactory = new CClassFactory();
        HRESULT hr = pFactory->QueryInterface(riid, ppvObj);
        pFactory->Release();
        return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}
STDAPI DllCanUnloadNow(void) { return (g_cRefDll == 0) ? S_OK : S_FALSE; }
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) g_hInst = hInstance;
    return TRUE;
}

HRESULT RegisterServer() {
    WCHAR szPath[MAX_PATH];
    GetModuleFileNameW(g_hInst, szPath, MAX_PATH);

    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CLASSES_ROOT, "CLSID\\{3D5B7C41-1839-4E8D-A59F-9CE2B6D48A11}\\InProcServer32", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExW(hKey, NULL, 0, REG_SZ, (const BYTE*)szPath, (DWORD)(wcslen(szPath) + 1) * sizeof(WCHAR));
        RegSetValueExA(hKey, "ThreadingModel", 0, REG_SZ, (const BYTE*)"Apartment", 10);
        RegCloseKey(hKey);
    }
    
    ITfInputProcessorProfileMgr* pProfileMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr))) {
        pProfileMgr->RegisterProfile(c_clsidTsfEngine, 0x042A, c_guidProfile, L"Unikey Windows TSF", (ULONG)wcslen(L"Unikey Windows TSF"), szPath, (ULONG)wcslen(szPath), 0, 0, 0, TRUE, 0);
        pProfileMgr->Release();
    }
    
    ITfCategoryMgr* pCatMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCatMgr))) {
        pCatMgr->RegisterCategory(c_clsidTsfEngine, GUID_TFCAT_TIP_KEYBOARD, c_clsidTsfEngine);
        pCatMgr->Release();
    }
    return S_OK;
}

HRESULT UnregisterServer() {
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "CLSID\\{3D5B7C41-1839-4E8D-A59F-9CE2B6D48A11}");
    ITfInputProcessorProfileMgr* pProfileMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr))) {
        pProfileMgr->UnregisterProfile(c_clsidTsfEngine, 0x042A, c_guidProfile, 0);
        pProfileMgr->Release();
    }
    ITfCategoryMgr* pCatMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCatMgr))) {
        pCatMgr->UnregisterCategory(c_clsidTsfEngine, GUID_TFCAT_TIP_KEYBOARD, c_clsidTsfEngine);
        pCatMgr->Release();
    }
    return S_OK;
}
STDAPI DllRegisterServer(void) { return RegisterServer(); }
STDAPI DllUnregisterServer(void) { return UnregisterServer(); }
