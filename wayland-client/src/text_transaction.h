#ifndef TEXT_TRANSACTION_H
#define TEXT_TRANSACTION_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

struct SurroundingReplacement {
    int32_t index = 0;
    uint32_t length = 0;
    size_t start = 0;
    size_t end = 0;
    bool uses_surrounding = false;
};

struct SurroundingSnapshot {
    std::string text;
    uint32_t cursor = 0;
    uint32_t anchor = 0;
    bool valid = false;
};

inline size_t utf8_common_prefix_bytes(const std::string& left, const std::string& right) {
    size_t common = 0;
    while (common < left.size() && common < right.size() && left[common] == right[common]) {
        ++common;
    }
    while (common > 0 && common < left.size() &&
           (static_cast<unsigned char>(left[common]) & 0xC0) == 0x80) {
        --common;
    }
    return common;
}

inline bool composition_matches_surrounding(const std::string& text,
                                            uint32_t cursor,
                                            uint32_t anchor,
                                            const std::string& composition) {
    if (composition.empty()) return true;
    const size_t insertion = std::min<size_t>(cursor, anchor);
    if (cursor > text.size() || anchor > text.size() || insertion < composition.size()) {
        return false;
    }
    return text.compare(insertion - composition.size(), composition.size(), composition) == 0;
}

// The delete range must cover the current selection.
inline SurroundingReplacement make_surrounding_replacement(size_t old_tail_bytes,
                                                            const std::string& text,
                                                            uint32_t cursor,
                                                            uint32_t anchor) {
    SurroundingReplacement result;
    result.index = -static_cast<int32_t>(old_tail_bytes);
    result.length = static_cast<uint32_t>(old_tail_bytes);

    if (cursor > text.size() || anchor > text.size()) return result;
    const size_t selection_start = std::min<size_t>(cursor, anchor);
    const size_t selection_end = std::max<size_t>(cursor, anchor);
    if (old_tail_bytes > selection_start) return result;

    const size_t start = selection_start - old_tail_bytes;
    const size_t length = selection_end - start;
    const int64_t index = static_cast<int64_t>(start) - static_cast<int64_t>(cursor);
    if (index < std::numeric_limits<int32_t>::min() ||
        index > std::numeric_limits<int32_t>::max() ||
        length > std::numeric_limits<uint32_t>::max()) {
        return result;
    }

    result.index = static_cast<int32_t>(index);
    result.length = static_cast<uint32_t>(length);
    result.start = start;
    result.end = selection_end;
    result.uses_surrounding = true;
    return result;
}

inline SurroundingSnapshot apply_surrounding_replacement(
    const std::string& text,
    uint32_t cursor,
    uint32_t anchor,
    const SurroundingReplacement& replacement,
    const std::string& inserted_text) {
    SurroundingSnapshot result;
    if (!replacement.uses_surrounding || replacement.start > replacement.end ||
        replacement.end > text.size()) {
        return result;
    }

    result.text = text;
    result.text.replace(replacement.start,
                        replacement.end - replacement.start,
                        inserted_text);
    result.cursor = static_cast<uint32_t>(replacement.start + inserted_text.size());
    result.anchor = result.cursor;
    result.valid = true;
    return result;
}

inline bool surrounding_matches(const SurroundingSnapshot& expected,
                                const std::string& text,
                                uint32_t cursor,
                                uint32_t anchor) {
    return expected.valid && expected.text == text &&
           expected.cursor == cursor && expected.anchor == anchor;
}

inline bool surrounding_prefix_ends_with(const std::string& text,
                                         uint32_t cursor,
                                         uint32_t anchor,
                                         const std::string& suffix) {
    if (cursor > text.size() || anchor > text.size()) return false;
    const size_t insertion = std::min<size_t>(cursor, anchor);
    return suffix.size() <= insertion &&
           text.compare(insertion - suffix.size(), suffix.size(), suffix) == 0;
}

inline SurroundingSnapshot apply_forwarded_key(const SurroundingSnapshot& snapshot,
                                               char key) {
    if (!snapshot.valid || snapshot.cursor > snapshot.text.size() ||
        snapshot.anchor > snapshot.text.size()) {
        return {};
    }

    size_t old_tail_bytes = 0;
    std::string inserted_text(1, key);
    const size_t insertion = std::min<size_t>(snapshot.cursor, snapshot.anchor);
    if (key == '\b') {
        inserted_text.clear();
        if (snapshot.cursor == snapshot.anchor && insertion > 0) {
            size_t previous = insertion - 1;
            while (previous > 0 &&
                   (static_cast<unsigned char>(snapshot.text[previous]) & 0xC0) == 0x80) {
                --previous;
            }
            old_tail_bytes = insertion - previous;
        }
    }

    const auto replacement = make_surrounding_replacement(
        old_tail_bytes, snapshot.text, snapshot.cursor, snapshot.anchor);
    return apply_surrounding_replacement(
        snapshot.text, snapshot.cursor, snapshot.anchor, replacement, inserted_text);
}

#endif // TEXT_TRANSACTION_H
