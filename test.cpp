#include <iostream>
#include <string_view>
#include "heredoc.hpp"

void test1() {
    static constexpr auto clean_query = heredoc::IndentHeredoc< R"(
        SELECT username
        FROM account_credentials;
    )" >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    // Build-Time Validation Asserts
    // 1. Line 1 starts with 'S'
    static_assert(clean_query.view()[0] == 'S');

    // 2. Line 2 starts with 'F' after the first newline
    constexpr std::size_t first_nl = clean_query.view().find('\n');
    static_assert(clean_query.view()[first_nl + 1] == 'F');

    // 3. String contains exactly 2 newlines and ends immediately after the second one
    static_assert(clean_query.view().ends_with('\n'));
    static_assert(clean_query.view() == static_cast<std::string_view>( "SELECT username\nFROM account_credentials;\n" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

void test2() {
    static constexpr auto clean_query = heredoc::IndentHeredoc< R"(

        SELECT
            username
        FROM
            account_credentials;

    )" >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    // Build-Time Validation Asserts
    // 1. Line 1 starts with 'S'
    static_assert(clean_query.view()[0] == 'S');

    // 2. Line 2 starts with 'F' after the first newline
    constexpr std::size_t first_nl = clean_query.view().find('\n');
    constexpr std::size_t second_nl = clean_query.view().find('\n', first_nl + 1);
    static_assert(clean_query.view()[second_nl + 1] == 'F');

    // 3. String contains exactly 2 newlines and ends immediately after the second one
    static_assert(clean_query.view().ends_with('\n'));
    static_assert(clean_query.view() == static_cast<std::string_view>( "SELECT\n    username\nFROM\n    account_credentials;\n" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

void test3() {
    static constexpr auto clean_query = heredoc::PlainHeredoc< R"(

    Line 1: No junk newline before me!
    Line 2: Indentation is explicitly kept.

    )" >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    // Build-Time Validation Asserts
    // 1. Line 1 starts with ' '
    static_assert(clean_query.view()[0] == ' ');

    // 2. String contains exactly 2 newlines and ends immediately after the second one
    static_assert(clean_query.view().ends_with('\n'));
    static_assert(clean_query.view() == static_cast<std::string_view>( "    Line 1: No junk newline before me!\n    Line 2: Indentation is explicitly kept.\n" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

void test4() {
    static constexpr auto clean_query = heredoc::PlainHeredoc< R"(
    Line 1: No junk newline before me!)" >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    // Build-Time Validation Asserts
    // 1. Line 1 starts with ' '
    static_assert(clean_query.view()[0] == ' ');

    // 2. String contains exactly 2 newlines and ends immediately after the second one
    static_assert(clean_query.view().ends_with('!'));
    static_assert(clean_query.view() == static_cast<std::string_view>( "    Line 1: No junk newline before me!" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

void test5() {
    static constexpr auto clean_query = heredoc::IndentHeredoc< R"(
    Line 1: No junk newline before me!)" >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    // Build-Time Validation Asserts
    // 1. Line 1 starts with ' '
    static_assert(clean_query.view()[0] == 'L');

    // 2. String contains exactly 2 newlines and ends immediately after the second one
    static_assert(clean_query.view().ends_with('!'));
    static_assert(clean_query.view() == static_cast<std::string_view>( "Line 1: No junk newline before me!" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

void test6() {
    static constexpr auto clean_query = heredoc::PlainHeredoc<
        "\r\n"
        "Test string with embedded carriage returns\r\n"
        "And trailing whitespace\r\n"
        " "
    >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    static_assert(clean_query.view()[0] == 'T');
    static_assert(clean_query.view() == static_cast<std::string_view>( "Test string with embedded carriage returns\r\nAnd trailing whitespace\r\n" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

void test7() {
    static constexpr auto clean_query = heredoc::PlainHeredoc<
        "  \n\r"
        "Test string with embedded reverse carriage returns\n\r"
        "And trailing whitespace\n\r"
        " "
    >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    static_assert(clean_query.view()[0] == 'T');
    static_assert(clean_query.view().ends_with('\r'));
    static_assert(clean_query.view() == static_cast<std::string_view>( "Test string with embedded reverse carriage returns\n\rAnd trailing whitespace\n\r" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

void test8() {
    static constexpr auto clean_query = heredoc::IndentHeredoc<
        "  \n\r"
        "  Test string with embedded reverse carriage returns\n\r"
        "  And trailing whitespace\n\r"
        " "
    >();

    std::cout << "--- Verified Output Begin ---\n";
    std::cout << clean_query;
    std::cout << "--- Verified Output End ---\n";

    static_assert(clean_query.view()[0] == 'T');
    static_assert(clean_query.view().ends_with('\r'));
    static_assert(clean_query.view() == static_cast<std::string_view>( "Test string with embedded reverse carriage returns\n\rAnd trailing whitespace\n\r" ));

    std::cout << "\nAll conditions passed successfully!\n";
}

int main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test8();
}
