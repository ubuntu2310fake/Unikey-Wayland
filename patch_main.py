import re

with open('wayland-client/src/main.cpp', 'r') as f:
    content = f.read()

# Replace macro surrounding text
macro_regex = r"int byte_backs = current_word\.length\(\);\s*if \(byte_backs > 0\) \{\s*zwp_input_method_context_v1_delete_surrounding_text\(state->context, -byte_backs, byte_backs\);\s*\}"

macro_replace = """int char_backs = 0;
                        size_t idx = 0;
                        while (idx < current_word.length()) {
                            char_backs++;
                            idx++;
                            while (idx < current_word.length() && (current_word[idx] & 0xC0) == 0x80) idx++;
                        }
                        for (int k = 0; k < char_backs; k++) {
                            zwp_input_method_context_v1_key(state->context, state->latest_serial, time, 14, 1);
                            zwp_input_method_context_v1_key(state->context, state->latest_serial, time, 14, 0);
                        }"""

content = re.sub(macro_regex, macro_replace, content, flags=re.MULTILINE)

# Replace surrounding text in normal typing
surrounding_regex = r"int byte_backs = state->composed_word\.length\(\) - common_bytes;\s*if \(byte_backs > 0\) \{.*?\}(?=\s*std::string suffix)"

surrounding_replace = """int byte_backs = state->composed_word.length() - common_bytes;
            if (byte_backs > 0) {
                int chars_to_delete = 0;
                size_t idx = common_bytes;
                while (idx < state->composed_word.length()) {
                    chars_to_delete++;
                    idx++;
                    while (idx < state->composed_word.length() && (state->composed_word[idx] & 0xC0) == 0x80) idx++;
                }
                for (int k = 0; k < chars_to_delete; k++) {
                    zwp_input_method_context_v1_key(state->context, state->latest_serial, time, 14, 1);
                    zwp_input_method_context_v1_key(state->context, state->latest_serial, time, 14, 0);
                }
            }"""

content = re.sub(surrounding_regex, surrounding_replace, content, flags=re.MULTILINE | re.DOTALL)

with open('wayland-client/src/main.cpp', 'w') as f:
    f.write(content)
