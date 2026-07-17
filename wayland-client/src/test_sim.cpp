#include <iostream>
#include <string>
#include <vector>

std::string _composedWord = "";
std::vector<std::string> preedit_history;
int preedit_idx = 0;

int utf8_strlen(const std::string& str) {
    int len = 0;
    for (size_t i = 0; i < str.length(); i++) {
        if ((str[i] & 0xC0) != 0x80) len++;
    }
    return len;
}

void simulate_key(const std::string& preedit) {
    std::string old_composed = _composedWord;
    std::string new_composed = preedit;

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
    
    std::string to_insert = new_composed.substr(common_bytes);
    
    std::cout << "  char_backs: " << char_backs << ", to_insert: " << to_insert << "\n";
    _composedWord = new_composed;
}

int main() {
    std::cout << "Type 'm':\n";
    simulate_key("m");
    std::cout << "Type 'e':\n";
    simulate_key("me");
    std::cout << "Type 'j':\n";
    simulate_key("m\xE1\xBA\xB9");
    std::cout << "Type 'backspace':\n";
    simulate_key("me");
    return 0;
}
