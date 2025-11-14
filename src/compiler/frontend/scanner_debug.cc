#include "scanner_debug.hh"

void trace_token(Scanner& scanner, const std::optional<TokenType>& token)
{
    if (token.has_value()) {
        if (*token == TokenType::Integer) {
            fmt::println("Got Token: [{}] with number `{}`", *token, scanner.number());
        } else if (scanner.lexeme().empty()) {
            fmt::println("Got Token: [{}]", *token);
        } else {
            fmt::println("Got Token: [{}] with lexeme `{}`", *token, scanner.lexeme());
        }
    } else {
        fmt::println("Unreachable");
    }
}
