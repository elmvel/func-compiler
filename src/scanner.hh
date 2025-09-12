#ifndef SCANNER_H_
#define SCANNER_H_

#include <optional>
#include <string>

#include <fmt/core.h>
#include <fmt/std.h>

enum class TokenType
{
    // Core
    Id,
    Integer,
    String,

    // Symbols
    LParen,
    RParen,
    LBracket,
    RBracket,
    Comma,
    Assign,
    Add,
    Sub,
    Mul,
    Div,
    Arrow,
    And,
    Dot,

    // Keywords
    End,
    If,
    Then,
    Else,
    Match,
};

enum class ScanState
{
    Start, LexIdent, LexInt, LexString, LexComment, Done, Error
};

class Scanner
{
public:
    Scanner(const std::string& content);
    std::optional<TokenType> next_token();
    // Used to save progress of the scanner
    size_t save();
    // Used to restore the scanner
    void restore(size_t point);
    std::string& lexeme();
    int number();
private:
    ScanState m_state;
    std::string m_content;
    size_t m_start;
    size_t m_cursor;
    std::string m_lexeme;
    int m_number;
};

#endif // SCANNER_H_
