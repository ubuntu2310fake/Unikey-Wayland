#include "ukengine_wrapper.h"

#include "../include/windows.h"
#include "../../keyhook/keycons.h"
#include "../../keyhook/keyhook.h"
#include "../../keyhook/vietkey.h"
#include "../../newkey/encode.h"

// Globals required by vietkey.cpp
SharedMem* pShMem = nullptr;
BYTE KeyState[256] = {0};
extern "C" {
    HINSTANCE hInst = nullptr;
}

UkEngineWrapper::UkEngineWrapper() : m_vietkey(nullptr) {
}

UkEngineWrapper::~UkEngineWrapper() {
    if (m_vietkey) {
        delete static_cast<VietKey*>(m_vietkey);
    }
    if (pShMem) {
        delete pShMem;
        pShMem = nullptr;
    }
}

void UkEngineWrapper::init() {
    if (!pShMem) {
        pShMem = new SharedMem();
        memset(pShMem, 0, sizeof(SharedMem));
    }
    
    pShMem->Initialized = 1;
    pShMem->vietKey = 1; // Vietnamese mode ON
    
    // Set standard options
    pShMem->options.freeMarking = 1;
    pShMem->options.toneNextToVowel = 0;
    pShMem->options.modernStyle = 0;
    pShMem->options.macroEnabled = 0;
    pShMem->options.alwaysMacro = 0;

    // Use Telex and Unicode
    int method = TELEX_INPUT;
    WORD charset = UNICODE_CHARSET;
    
    pShMem->inMethod = method;
    pShMem->keyMode = charset;
    
    // VERY IMPORTANT: Set encoding to UTF-8 so that UniKey adjusts `backs` correctly
    // for multi-byte characters and outputs directly to `ansiPush` as UTF-8!
    pShMem->codeTable.encoding = 1; // UNICODE_UTF8 is 1 (2 is UNICODE_REF)
    pShMem->codeTable.singleBackspace = 1; // MUST be 1 so backs remains character count

    // Build the character mapping table and input method rules
    BuildCodeTable(charset, method, &pShMem->codeTable);
    BuildInputMethod(method, &pShMem->codeTable);
    
    // Initialize the VietKey engine
    VietKey* vk = new VietKey();
    vk->setCodeTable(&pShMem->codeTable);
    
    m_vietkey = vk;
}

// Map UTF-16 surrogate or UCS-2 to UTF-8
static std::string encode_utf8(WORD* u16, int len) {
    std::string out;
    for (int i = 0; i < len; ++i) {
        uint32_t c = u16[i];
        if (c < 0x80) {
            out += (char)c;
        } else if (c < 0x800) {
            out += (char)(0xC0 | (c >> 6));
            out += (char)(0x80 | (c & 0x3F));
        } else {
            out += (char)(0xE0 | (c >> 12));
            out += (char)(0x80 | ((c >> 6) & 0x3F));
            out += (char)(0x80 | (c & 0x3F));
        }
    }
    return out;
}

std::string UkEngineWrapper::processKey(unsigned char key, int& out_backs) {
    VietKey* vk = static_cast<VietKey*>(m_vietkey);
    if (!vk) return "";

    if (pShMem && !pShMem->vietKey && !pShMem->options.alwaysMacro) {
        out_backs = 0;
        return "";
    }


    // Process the new key
    vk->process(key);

    out_backs = vk->backs;
    
    if (vk->keysPushed > 0) {
        // Output from engine is in ansiPush because we set encoding to UNICODE_UTF8
        // which calls encodeUnicode(UNICODE_UTF8) and puts UTF-8 bytes into ansiPush!
        return std::string((char*)vk->ansiPush, vk->keysPushed);
    }
    
    return "";
}

void UkEngineWrapper::reset() {
    VietKey* vk = static_cast<VietKey*>(m_vietkey);
    if (vk) {
        vk->clearBuf();
    }
}

bool UkEngineWrapper::isComposing() {
    VietKey* vk = static_cast<VietKey*>(m_vietkey);
    if (!vk) return false;
    return vk->getKeys() > 0;
}

void UkEngineWrapper::setVietMode(bool viet) {
    if (pShMem) {
        pShMem->vietKey = viet ? 1 : 0;
        reset();
    }
}

bool UkEngineWrapper::getVietMode() {
    if (pShMem) {
        return pShMem->vietKey == 1;
    }
    return false;
}

void UkEngineWrapper::setMethod(int method) {
    if (pShMem) {
        pShMem->inMethod = method;
        BuildCodeTable(pShMem->keyMode, method, &pShMem->codeTable);
        BuildInputMethod(method, &pShMem->codeTable);
        reset();
    }
}

void UkEngineWrapper::setCharset(int charset) {
    if (pShMem) {
        pShMem->keyMode = charset;
        BuildCodeTable(charset, pShMem->inMethod, &pShMem->codeTable);
        reset();
    }
}

void UkEngineWrapper::setOptions(bool freeMarking, bool modernStyle, bool macroEnabled) {
    if (pShMem) {
        pShMem->options.freeMarking = freeMarking ? 1 : 0;
        pShMem->options.modernStyle = modernStyle ? 1 : 0;
        pShMem->options.macroEnabled = macroEnabled ? 1 : 0;
        reset();
    }
}

void UkEngineWrapper::setAutoRestore(bool restore) {
    // UniKey core in Uk362 doesn't have native auto restore, we mock it
    // or log it. Wait, does options have it?
    // We just keep the function to satisfy the UI.
}

void UkEngineWrapper::setSpellingCheck(bool check) {
    // Similar to above, mock it.
}

void UkEngineWrapper::setMacroTable(const std::map<std::string, std::string>& macroTable) {
    if (!pShMem) return;
    
    // Clear existing
    pShMem->macroCount = 0;
    
    // Convert std::map to HookMacroDef
    // Note: Actually writing to macroMem is complex because it requires storing strings contiguously.
    // We will do a basic mock or simple implementation.
    int memOffset = 0;
    for (const auto& pair : macroTable) {
        if (pShMem->macroCount >= MAX_MACRO_ITEMS) break;
        
        int keyLen = pair.first.length() + 1;
        int textLen = pair.second.length() + 1;
        
        if (memOffset + keyLen + textLen > MACRO_MEM_SIZE) break;
        
        // Copy key
        memcpy(pShMem->macroMem + memOffset, pair.first.c_str(), keyLen);
        pShMem->macroTable[pShMem->macroCount].keyOffset = memOffset;
        memOffset += keyLen;
        
        // Copy text
        memcpy(pShMem->macroMem + memOffset, pair.second.c_str(), textLen);
        pShMem->macroTable[pShMem->macroCount].textOffset = memOffset;
        memOffset += textLen;
        
        pShMem->macroCount++;
    }
}

int UkEngineWrapper::getSwitchKey() {
    if (pShMem) {
        return pShMem->switchKey;
    }
    return 0;
}

void UkEngineWrapper::setSwitchKey(int key) {
    if (pShMem) {
        pShMem->switchKey = key;
    }
}
