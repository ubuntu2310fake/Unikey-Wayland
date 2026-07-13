#include <windows.h>
#include <msctf.h>
#include <olectl.h>
#include <strsafe.h>

// GUIDs
const CLSID c_clsidTsfPoC = { 0x2ca46dc3, 0x2627, 0x4e22, { 0xb8, 0xe5, 0xa2, 0x53, 0x3f, 0x17, 0x35, 0xcd } };
const GUID c_guidProfile = { 0xb3a675bb, 0x9bfb, 0x4261, { 0xae, 0xf4, 0x51, 0xce, 0x11, 0xbb, 0xff, 0x13 } };

HINSTANCE g_hInst = NULL;
LONG g_cRefDll = 0;

void DllAddRef() { InterlockedIncrement(&g_cRefDll); }
void DllRelease() { InterlockedDecrement(&g_cRefDll); }

// ----------------------------------------------------------------------
// CEditSession: Performs the actual text replacement
// ----------------------------------------------------------------------
class CEditSession : public ITfEditSession {
    LONG _cRef;
    ITfContext* _pContext;
public:
    CEditSession(ITfContext* pContext) : _cRef(1), _pContext(pContext) {
        _pContext->AddRef();
    }
    ~CEditSession() {
        _pContext->Release();
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
        if (riid == IID_IUnknown || riid == IID_ITfEditSession) {
            *ppvObj = (ITfEditSession*)this;
            AddRef();
            return S_OK;
        }
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0) delete this;
        return cRef;
    }

    STDMETHODIMP DoEditSession(TfEditCookie ec) {
        TF_SELECTION sel;
        ULONG cFetched;
        // Lấy vị trí con trỏ (caret)
        if (FAILED(_pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &cFetched)) || cFetched != 1) {
            return S_OK;
        }

        // Semantics: Xóa 2 ký tự ngay trước con trỏ bằng cách kéo dãn range
        LONG cch;
        sel.range->ShiftStart(ec, -2, &cch, NULL);

        // Thay thế chúng bằng chuỗi UTF-16 "Việt Nam"
        const WCHAR* replacement = L"Vi\x1EC7t Nam"; // Unicode cho "Việt Nam"
        sel.range->SetText(ec, 0, replacement, (ULONG)wcslen(replacement));

        // Đưa con trỏ về cuối chuỗi vừa chèn
        sel.range->Collapse(ec, TF_ANCHOR_END);
        _pContext->SetSelection(ec, 1, &sel);

        sel.range->Release();
        return S_OK;
    }
};

// ----------------------------------------------------------------------
// CTextService: The main TSF TIP and Key Event Sink
// ----------------------------------------------------------------------
class CTextService : public ITfTextInputProcessor, public ITfKeyEventSink {
    LONG _cRef;
    ITfThreadMgr* _pThreadMgr;
    TfClientId _clientId;
public:
    CTextService() : _cRef(1), _pThreadMgr(NULL), _clientId(TF_CLIENTID_NULL) { DllAddRef(); }
    ~CTextService() { DllRelease(); }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
        if (riid == IID_IUnknown || riid == IID_ITfTextInputProcessor) {
            *ppvObj = (ITfTextInputProcessor*)this;
        } else if (riid == IID_ITfKeyEventSink) {
            *ppvObj = (ITfKeyEventSink*)this;
        } else {
            *ppvObj = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0) delete this;
        return cRef;
    }

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr* pThreadMgr, TfClientId tfClientId) {
        _pThreadMgr = pThreadMgr;
        _pThreadMgr->AddRef();
        _clientId = tfClientId;

        ITfKeystrokeMgr* pKeystrokeMgr;
        if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&pKeystrokeMgr))) {
            pKeystrokeMgr->AdviseKeyEventSink(_clientId, (ITfKeyEventSink*)this, TRUE);
            pKeystrokeMgr->Release();
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
            _pThreadMgr->Release();
            _pThreadMgr = NULL;
        }
        return S_OK;
    }

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground) { return S_OK; }
    STDMETHODIMP OnTestKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
        if (wParam == VK_SPACE) *pfEaten = TRUE; // Chặn phím Space
        else *pfEaten = FALSE;
        return S_OK;
    }
    STDMETHODIMP OnKeyDown(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
        if (wParam == VK_SPACE && pic != NULL) {
            *pfEaten = TRUE;
            CEditSession* pEditSession = new CEditSession(pic);
            HRESULT hrSession;
            pic->RequestEditSession(_clientId, pEditSession, TF_ES_READWRITE | TF_ES_SYNC, &hrSession);
            pEditSession->Release();
            return S_OK;
        }
        *pfEaten = FALSE;
        return S_OK;
    }
    STDMETHODIMP OnTestKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
        *pfEaten = FALSE; return S_OK;
    }
    STDMETHODIMP OnKeyUp(ITfContext* pic, WPARAM wParam, LPARAM lParam, BOOL* pfEaten) {
        *pfEaten = FALSE; return S_OK;
    }
    STDMETHODIMP OnPreservedKey(ITfContext* pic, REFGUID rguid, BOOL* pfEaten) {
        *pfEaten = FALSE; return S_OK;
    }
};

// ----------------------------------------------------------------------
// Class Factory
// ----------------------------------------------------------------------
class CClassFactory : public IClassFactory {
    LONG _cRef;
public:
    CClassFactory() : _cRef(1) { DllAddRef(); }
    ~CClassFactory() { DllRelease(); }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObj) {
        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppvObj = (IClassFactory*)this;
            AddRef();
            return S_OK;
        }
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }
    STDMETHODIMP_(ULONG) Release() {
        LONG cRef = InterlockedDecrement(&_cRef);
        if (cRef == 0) delete this;
        return cRef;
    }
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppvObj) {
        if (pUnkOuter) return CLASS_E_NOAGGREGATION;
        CTextService* pService = new CTextService();
        if (!pService) return E_OUTOFMEMORY;
        HRESULT hr = pService->QueryInterface(riid, ppvObj);
        pService->Release();
        return hr;
    }
    STDMETHODIMP LockServer(BOOL fLock) {
        if (fLock) DllAddRef(); else DllRelease();
        return S_OK;
    }
};

// ----------------------------------------------------------------------
// COM Exports & Registry
// ----------------------------------------------------------------------
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppvObj) {
    if (IsEqualIID(rclsid, c_clsidTsfPoC)) {
        CClassFactory* pFactory = new CClassFactory();
        HRESULT hr = pFactory->QueryInterface(riid, ppvObj);
        pFactory->Release();
        return hr;
    }
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow(void) {
    return (g_cRefDll == 0) ? S_OK : S_FALSE;
}

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hInstance;
    }
    return TRUE;
}

// Hàm hỗ trợ đăng ký Registry đơn giản
HRESULT RegisterServer() {
    WCHAR szPath[MAX_PATH];
    GetModuleFileNameW(g_hInst, szPath, MAX_PATH);

    HKEY hKey;
    if (RegCreateKeyExA(HKEY_CLASSES_ROOT, "CLSID\\{2CA46DC3-2627-4E22-B8E5-A2533F1735CD}\\InProcServer32", 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS) {
        RegSetValueExW(hKey, NULL, 0, REG_SZ, (const BYTE*)szPath, (DWORD)(wcslen(szPath) + 1) * sizeof(WCHAR));
        RegSetValueExA(hKey, "ThreadingModel", 0, REG_SZ, (const BYTE*)"Apartment", 10);
        RegCloseKey(hKey);
    }
    
    // Register TIP category
    ITfInputProcessorProfileMgr* pProfileMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr))) {
        pProfileMgr->RegisterProfile(c_clsidTsfPoC, 0x042A, c_guidProfile, L"Unikey TSF PoC", (ULONG)wcslen(L"Unikey TSF PoC"), szPath, (ULONG)wcslen(szPath), 0, 0, 0, TRUE, 0);
        pProfileMgr->Release();
    }
    
    ITfCategoryMgr* pCatMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCatMgr))) {
        pCatMgr->RegisterCategory(c_clsidTsfPoC, GUID_TFCAT_TIP_KEYBOARD, c_clsidTsfPoC);
        pCatMgr->Release();
    }
    return S_OK;
}

HRESULT UnregisterServer() {
    RegDeleteTreeA(HKEY_CLASSES_ROOT, "CLSID\\{2CA46DC3-2627-4E22-B8E5-A2533F1735CD}");
    
    ITfInputProcessorProfileMgr* pProfileMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER, IID_ITfInputProcessorProfileMgr, (void**)&pProfileMgr))) {
        pProfileMgr->UnregisterProfile(c_clsidTsfPoC, 0x042A, c_guidProfile, 0);
        pProfileMgr->Release();
    }
    
    ITfCategoryMgr* pCatMgr;
    if (SUCCEEDED(CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, (void**)&pCatMgr))) {
        pCatMgr->UnregisterCategory(c_clsidTsfPoC, GUID_TFCAT_TIP_KEYBOARD, c_clsidTsfPoC);
        pCatMgr->Release();
    }
    return S_OK;
}

STDAPI DllRegisterServer(void) {
    return RegisterServer();
}

STDAPI DllUnregisterServer(void) {
    return UnregisterServer();
}
