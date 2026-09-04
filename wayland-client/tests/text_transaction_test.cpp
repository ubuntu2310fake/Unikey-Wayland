#include "text_transaction.h"

#include <cassert>
#include <string>

int main() {
    const std::string word = "thê";
    assert(composition_matches_surrounding(word, word.size(), word.size(), word));
    assert(!composition_matches_surrounding("other", 5, 5, word));

    // Regression: "thê" -> "thể" must not split a UTF-8 codepoint.
    const std::string toned = "thể";
    const size_t common = utf8_common_prefix_bytes(word, toned);
    assert(common == 2);
    assert(word.size() - common == 2);
    assert(toned.substr(common) == "ể");
    const auto plain = make_surrounding_replacement(2, word, 4, 4);
    assert(plain.uses_surrounding);
    assert(plain.index == -2);
    assert(plain.length == 2);

    const std::string autocomplete = "thê suggestion";
    const auto forward = make_surrounding_replacement(2, autocomplete, 4, autocomplete.size());
    assert(forward.uses_surrounding);
    assert(forward.index == -2);
    assert(forward.length == autocomplete.size() - 2);

    const auto reverse = make_surrounding_replacement(2, autocomplete, autocomplete.size(), 4);
    assert(reverse.uses_surrounding);
    assert(reverse.index == 2 - static_cast<int32_t>(autocomplete.size()));
    assert(reverse.length == autocomplete.size() - 2);

    const std::string plain_word = "cach";
    const std::string accented_word = "cách";
    const size_t accent_common = utf8_common_prefix_bytes(plain_word, accented_word);
    const auto replace_accent = make_surrounding_replacement(
        plain_word.size() - accent_common,
        plain_word, plain_word.size(), plain_word.size());
    const auto expected = apply_surrounding_replacement(
        plain_word, plain_word.size(), plain_word.size(),
        replace_accent, accented_word.substr(accent_common));
    assert(expected.valid && expected.text == accented_word);

    // KWin reports the delete transaction before the following commit. That
    // intermediate snapshot must not replace the optimistic outgoing state.
    const auto after_delete = apply_surrounding_replacement(
        plain_word, plain_word.size(), plain_word.size(), replace_accent, "");
    assert(after_delete.valid && after_delete.text == "c");
    assert(!surrounding_matches(expected, after_delete.text,
                                after_delete.cursor, after_delete.anchor));
    assert(surrounding_matches(expected, accented_word,
                               accented_word.size(), accented_word.size()));

    // Electron may change or truncate the surrounding-text window, and may
    // include unrelated text after the caret. Match only the local prefix.
    const std::string shifted = "older context cách unrelated suffix";
    const auto shifted_cursor = shifted.find(" unrelated suffix");
    assert(surrounding_prefix_ends_with(
        shifted, shifted_cursor, shifted_cursor, "cách"));
    assert(!surrounding_prefix_ends_with(
        "older context cacách", std::string("older context cacách").size(),
        std::string("older context cacách").size(), " cách"));

    // A later commit must preserve both the successful and failed branches
    // until the client reports which transaction it actually applied.
    const std::string untoned = "noi";
    const std::string toned_word = "nói";
    const size_t tone_common = utf8_common_prefix_bytes(untoned, toned_word);
    const auto tone_replacement = make_surrounding_replacement(
        untoned.size() - tone_common,
        untoned, untoned.size(), untoned.size());
    const auto toned_expected = apply_surrounding_replacement(
        untoned, untoned.size(), untoned.size(),
        tone_replacement, toned_word.substr(tone_common));
    const auto insert_only = make_surrounding_replacement(
        0, untoned, untoned.size(), untoned.size());
    const auto missed_delete = apply_surrounding_replacement(
        untoned, untoned.size(), untoned.size(),
        insert_only, toned_word.substr(tone_common));
    assert(toned_expected.valid && toned_expected.text == toned_word);
    assert(missed_delete.valid && missed_delete.text == "noiói");

    const auto repair = make_surrounding_replacement(
        missed_delete.text.size(), missed_delete.text,
        missed_delete.cursor, missed_delete.anchor);
    const auto repaired = apply_surrounding_replacement(
        missed_delete.text, missed_delete.cursor, missed_delete.anchor,
        repair, toned_word);
    assert(repaired.valid && repaired.text == toned_word);

    const SurroundingSnapshot committed{"nói ", 5, 5, true};
    const auto extra_space = apply_forwarded_key(committed, ' ');
    assert(extra_space.valid && extra_space.text == "nói  ");
    const auto remove_space = apply_forwarded_key(extra_space, '\b');
    assert(remove_space.valid && remove_space.text == "nói ");
    const auto remove_multibyte = apply_forwarded_key(
        SurroundingSnapshot{"nói", 4, 4, true}, '\b');
    assert(remove_multibyte.valid && remove_multibyte.text == "nó");

    return 0;
}
