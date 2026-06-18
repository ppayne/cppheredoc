#pragma once

#include <string_view>
#include <algorithm>
#include <cstddef>

// ============================================================================
// Stanza 1: Flat Inner Namespace for Implementation Details
// ============================================================================
namespace heredoc::detail {

    // Safely backtracks to the character immediately following the initial formatting newline
    constexpr std::string_view strip_leading_whitespace_and_newlines(std::string_view sv) {
        std::size_t first_nonwhitespace = sv.find_first_not_of(" \t\n");
        if (first_nonwhitespace == std::string_view::npos) {
            return ""; // String is completely empty or all whitespace
        }

        // Search backward from the first visible character to find the closest line break
        std::size_t previous_newline = sv.substr(0, first_nonwhitespace).find_last_of('\n');
        if (previous_newline == std::string_view::npos) {
            return sv; // No leading newlines exist (flat string format); preserve it entirely
        }

        return sv.substr(previous_newline + 1);
    }

    // Trims a whitespace-only final line but PRESERVES the trailing newline character
    constexpr std::string_view strip_trailing_whitespace_only_line(std::string_view sv) {
        if (sv.empty()) return sv;

        // find last non-whitespace character
        std::size_t last_non_whitespace = sv.find_last_not_of(" \t\n");
        if (last_non_whitespace == std::string_view::npos) {
            // no non-whitespace, just return empty string
            return "";
        }

        // find next newline character
        std::size_t next_nl = sv.find_first_of('\n', last_non_whitespace + 1);
        if (next_nl == std::string_view::npos) {
            // no trailing newline
            return sv;
        }

        return sv.substr(0, next_nl + 1);
    }

    // Computes the maximum common indentation found on all valid text lines in the block
    constexpr std::size_t calculate_common_indent(std::string_view sv) {
        std::size_t min_indent = static_cast<std::size_t>(-1); // Represents maximum size_t value
        std::string_view cursor = sv;

        while (!cursor.empty()) {
            std::size_t current_indent = 0;
            while (current_indent < cursor.size() &&
                  (cursor[current_indent] == ' ' || cursor[current_indent] == '\t')) {
                current_indent++;
            }

            std::size_t next_nl = cursor.find('\n');
            std::string_view line = (next_nl == std::string_view::npos) ? cursor : cursor.substr(0, next_nl);

            // Only factor in lines that contain actual text content (ignoring empty lines)
            if (line.find_first_not_of(" \t\n") != std::string_view::npos) {
                if (min_indent == static_cast<std::size_t>(-1) || current_indent < min_indent) {
                    min_indent = current_indent;
                }
            }

            if (next_nl == std::string_view::npos) break;
            cursor.remove_prefix(next_nl + 1);
        }

        return (min_indent == static_cast<std::size_t>(-1)) ? 0 : min_indent;
    }

    // Calculates exactly how many bytes the clean, un-indented multi-line string will take
    constexpr std::size_t calculate_indented_size(std::string_view sv, std::size_t common_indent) {
        std::size_t total_size = 0;
        std::string_view cursor = sv;

        while (!cursor.empty()) {
            std::size_t next_nl = cursor.find('\n');
            std::string_view line = (next_nl == std::string_view::npos) ? cursor : cursor.substr(0, next_nl);

            std::size_t leading_spaces = 0;
            while (leading_spaces < line.size() && (line[leading_spaces] == ' ' || line[leading_spaces] == '\t')) {
                leading_spaces++;
            }

            std::size_t strip_bytes = std::min(leading_spaces, common_indent);
            total_size += (line.size() - strip_bytes);

            if (next_nl == std::string_view::npos) break;
            total_size++; // Account for the newline char
            cursor.remove_prefix(next_nl + 1);
        }
        return total_size;
    }

    // Helper structural wrapper to accept raw literals directly into templates
    template <std::size_t N>
    struct StringLiteral {
        char value[N]; // Uninitialized array (safely overwritten by constructor)

        constexpr StringLiteral(const char (&str)[N]) {
            std::copy_n(str, N, value);
        }
    };

} // namespace heredoc::detail


// ============================================================================
// Stanza 2: Public Interface Namespace
// ============================================================================
namespace heredoc {

/**
 * @brief CleanString
 * Directly compatible native string representation wrapper.
 */
template <std::size_t N>
struct CleanString {
    char storage[N + 1];

    constexpr const char* c_str() const { return storage; }
    constexpr std::string_view view() const { return std::string_view(storage, N); }
    constexpr std::size_t size() const { return N; }

    constexpr operator const char*() const { return storage; }
};

/**
 * @brief PlainHeredoc
 * Strips layout wrapper artifacts but leaves internal indents untouched.
 */
template <detail::StringLiteral Literal>
consteval auto PlainHeredoc() {
    constexpr std::string_view full_sv(Literal.value, sizeof(Literal.value) - 1);
    constexpr std::string_view leading_stripped = detail::strip_leading_whitespace_and_newlines(full_sv);
    constexpr std::string_view fully_stripped = detail::strip_trailing_whitespace_only_line(leading_stripped);
    constexpr std::size_t TargetSize = fully_stripped.size();

    CleanString<TargetSize> result;
    std::copy_n(fully_stripped.data(), TargetSize, result.storage);
    result.storage[TargetSize] = '\0';
    return result;
}

/**
 * @brief IndentHeredoc
 * Computes, normalizes, and trims layout spacing block sizes completely at compile-time.
 */
template <detail::StringLiteral Literal>
consteval auto IndentHeredoc() {
    constexpr std::string_view full_sv(Literal.value, sizeof(Literal.value) - 1);
    constexpr std::string_view leading_stripped = detail::strip_leading_whitespace_and_newlines(full_sv);
    constexpr std::string_view sv = detail::strip_trailing_whitespace_only_line(leading_stripped);

    constexpr std::size_t common_indent = detail::calculate_common_indent(sv);
    constexpr std::size_t TargetSize = detail::calculate_indented_size(sv, common_indent);

    CleanString<TargetSize> result;
    std::size_t write_idx = 0;
    std::string_view cursor = sv;

    while (!cursor.empty()) {
        std::size_t next_nl = cursor.find('\n');
        std::string_view line = (next_nl == std::string_view::npos) ? cursor : cursor.substr(0, next_nl);

        std::size_t leading_spaces = 0;
        while (leading_spaces < line.size() && (line[leading_spaces] == ' ' || line[leading_spaces] == '\t')) {
            leading_spaces++;
        }

        std::size_t strip_bytes = std::min(leading_spaces, common_indent);
        std::size_t keep_bytes = line.size() - strip_bytes;

        for (std::size_t i = 0; i < keep_bytes; ++i) {
            result.storage[write_idx++] = line[strip_bytes + i];
        }

        if (next_nl == std::string_view::npos) break;
        result.storage[write_idx++] = '\n';
        cursor.remove_prefix(next_nl + 1);
    }

    if (write_idx > TargetSize) {
        throw "Heredoc memory tracking buffer overrun error!";
    } else if (write_idx != TargetSize) {
        throw "Heredoc memory calculations are incorrect!";
    }

    result.storage[TargetSize] = '\0';
    return result;
}

} // namespace heredoc
