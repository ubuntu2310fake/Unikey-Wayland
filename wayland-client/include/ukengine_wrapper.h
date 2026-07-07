#ifndef UKENGINE_WRAPPER_H
#define UKENGINE_WRAPPER_H

#include <string>
#include <map>

class UkEngineWrapper {
public:
    UkEngineWrapper();
    ~UkEngineWrapper();

    void init();
    
    // Process a key character. 
    // Returns the processed UTF-8 string to commit, or empty if nothing to commit.
    // 'out_backs' will contain the number of backspaces (characters to delete) needed before committing.
    std::string processKey(unsigned char key, int& out_backs);
    // Check if the engine is actively composing a word
    bool isComposing();

    // Reset the engine's internal buffer (e.g. when cursor moves or selection is deleted)
    void reset();

    // Configuration APIs for GUI
    void setVietMode(bool viet);
    bool getVietMode();
    void setMethod(int method);
    void setCharset(int charset);
    void setOptions(bool freeMarking, bool modernStyle, bool macroEnabled, bool alwaysMacro = false);
    
    // EVKey missing features mock / mapping
    void setAutoRestore(bool restore);
    void setSpellingCheck(bool check);
    void setMacroTable(const std::map<std::string, std::string>& macroTable);
    int getSwitchKey();
    void setSwitchKey(int key);

private:
    void* m_vietkey; // Opaque pointer to VietKey
};

#endif // UKENGINE_WRAPPER_H
