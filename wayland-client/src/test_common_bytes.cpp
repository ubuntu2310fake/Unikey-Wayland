#include <iostream>
#include <string>

int utf8_strlen(const std::string& str) {
    int len = 0;
    for (size_t i = 0; i < str.length(); i++) {
        if ((str[i] & 0xC0) != 0x80) len++;
    }
    return len;
}

int main() {
    std::string old_composed = "m\xE1\xBA\xB9"; // mẹ
    std::string new_composed = "me";
    
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
    
    std::cout << "old_composed len: " << old_composed.length() << "\n";
    std::cout << "common_bytes: " << common_bytes << "\n";
    std::cout << "char_backs: " << char_backs << "\n";
    std::cout << "to_insert: " << to_insert << "\n";
    
    return 0;
}
