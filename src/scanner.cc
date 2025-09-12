#include <vector>

#include "scanner.hh"

std::vector<std::pair<std::string, TokenType>> g_token_symbols = {
    {":=", TokenType::Assign},
    {"->", TokenType::Arrow},
    {"(", TokenType::LParen},
    {")", TokenType::RParen},
    {"[", TokenType::LBracket},
    {"]", TokenType::RBracket},
    {",", TokenType::Comma},
    {"+", TokenType::Add},
    {"-", TokenType::Sub},
    {"*", TokenType::Mul},
    {"/", TokenType::Div},
    {"&", TokenType::And},
    {".", TokenType::Dot},
};

std::vector<std::pair<std::string, TokenType>> g_token_keywords = {
    {"end", TokenType::End},
    {"if", TokenType::If},
    {"then", TokenType::Then},
    {"else", TokenType::Else},
    {"match", TokenType::Match},
};

Scanner::Scanner(const std::string& content)
    : m_state(ScanState::Start), m_content(content), m_cursor(0), m_lexeme({}), m_number(0)
{}

std::optional<TokenType> Scanner::next_token()
{
    std::optional<TokenType> token {};
    m_state = ScanState::Start;
    m_lexeme.clear();
    while (m_state != ScanState::Done) {
        switch (m_state) {
            case ScanState::Start: {
                if (isalpha(m_content[m_cursor])) {
                    m_start = m_cursor;
                    m_state = ScanState::LexIdent;
                } else if (isdigit(m_content[m_cursor])) {
                    m_start = m_cursor;
                    m_state = ScanState::LexInt;
                } else if (m_content[m_cursor] == '"') {
                    // Skip the quote
                    ++m_cursor;
                    m_start = m_cursor;
                    m_state = ScanState::LexString;
                } else if (isspace(m_content[m_cursor])) {   
                    // Skip whitespace
                    while (m_cursor < m_content.size() && isspace(m_content[m_cursor])) {
                        ++m_cursor;
                    }
                    break;
                } else if (m_content.size() - m_cursor >= 2 && m_content.compare(m_cursor, 2, "(*") == 0) {   
                    // This is a comment
                    m_state = ScanState::LexComment;
                } else {
                    if (m_cursor >= m_content.size()) {
                        // End of File
                        token = {};
                        m_state = ScanState::Done;
                        break;
                    }
                    for (auto& [sym, tok] : g_token_symbols) {
                        // Skip symbols that are too long
                        if (m_content.size() - m_cursor < sym.size()) continue;
                        if (m_content.compare(m_cursor, sym.size(), sym) == 0) {
                            m_cursor += sym.size();
                            token = tok;
                            m_state = ScanState::Done;
                            break;
                        }
                    }

                    // Needed for Loop
                    if (m_state == ScanState::Done) break;
                    
                    // Unrecognized text
                    m_state = ScanState::Error;
                }
            } break;
            case ScanState::LexIdent: {
                ++m_cursor;
                if (!isalpha(m_content[m_cursor])) {
                    m_lexeme.append(&m_content[m_start], m_cursor - m_start);
                    m_state = ScanState::Done;
                    token = TokenType::Id;
                }
            } break;
            case ScanState::LexInt: {
                ++m_cursor;
                if (!isdigit(m_content[m_cursor])) {
                    m_lexeme.append(&m_content[m_start], m_cursor - m_start);
                    m_number = std::stoi(m_lexeme);
                    m_state = ScanState::Done;
                    token = TokenType::Integer;
                }
            } break;
            case ScanState::LexString: {
                if (m_content[m_cursor] == '"') {
                    m_lexeme.append(&m_content[m_start], m_cursor - m_start);
                    m_state = ScanState::Done;
                    token = TokenType::String;
                }
                ++m_cursor;
            } break;
            case ScanState::LexComment: {
                ++m_cursor;
                if (m_content.size() - m_cursor < 2) {
                    // We ran out of file contents, unending comment found
                    fmt::println("Unclosed comment found.");
                    m_state = ScanState::Error;
                } else if (m_content.compare(m_cursor, 2, "*)") == 0) {
                    m_cursor += 2;
                    m_state = ScanState::Start;
                }
            } break;
            case ScanState::Done:
            case ScanState::Error:
            default: {
                fmt::println("Error ocurred while scanning. Invalid text found:");
                fmt::println("  `{}`", m_content[m_cursor]);
                abort();
            } break;
        }
        if (m_state == ScanState::Done) {
            // Lookup keywords if Identifier
            for (auto& [kwd, tok] : g_token_keywords) {
                if (m_lexeme == kwd) {
                    token = tok;
                }
            }
        }
    }

    return token;
}

// Used to save progress of the scanner
size_t Scanner::save()
{
    return m_cursor;
}

// Used to restore the scanner
void Scanner::restore(size_t point)
{
    m_cursor = point;
}

std::string& Scanner::lexeme()
{
    return m_lexeme;
}

int Scanner::number()
{
    return m_number;
}
