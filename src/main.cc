#include <fstream>
#include <sstream>
#include <string>
#include <optional>

#include <fmt/core.h>
#include <fmt/std.h>

#include "scanner.hh"

// Fmt support for TokenType

template <> struct fmt::formatter<TokenType>: formatter<std::string_view> {
  // parse is inherited from formatter<string_view>.

  auto format(TokenType c, format_context& ctx) const
    -> format_context::iterator;
};

// Possible X-Macro?
// Might ruin "LSP"
auto fmt::formatter<TokenType>::format(TokenType c, fmt::v10::format_context& ctx) const
    -> format_context::iterator {
  string_view name = "unknown";
  switch (c) {
  case TokenType::Id:       name="Id";       break;
  case TokenType::Integer:  name="Integer";  break;
  case TokenType::String:   name="String";   break;
  case TokenType::LParen:   name="LParen";   break;
  case TokenType::RParen:   name="RParen";   break;
  case TokenType::LBracket: name="LBracket"; break;
  case TokenType::RBracket: name="RBracket"; break;
  case TokenType::Comma:    name="Comma";    break;
  case TokenType::Assign:   name="Assign";   break;
  case TokenType::Add:      name="Add";      break;
  case TokenType::Sub:      name="Sub";      break;
  case TokenType::Mul:      name="Mul";      break;
  case TokenType::Div:      name="Div";      break;
  case TokenType::Arrow:    name="Arrow";    break;
  case TokenType::And:      name="And";      break;
  case TokenType::Dot:      name="Dot";      break;
  case TokenType::End:      name="End";      break;
  case TokenType::If:       name="If";       break;
  case TokenType::Then:     name="Then";     break;
  case TokenType::Else:     name="Else";     break;
  case TokenType::Match:    name="Match";    break;
  }
  return formatter<std::string_view>::format(name, ctx);
}

std::optional<std::string> read_file(const std::string& file_path)
{
    std::ifstream file(file_path);

    if (!file.is_open()) {
        fmt::println(stderr, "error: Could not read {}.", file_path);
        return {};
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    return content;
}

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

int main()
{
    std::string file_path = "examples/test.fc";
    auto file_content = read_file(file_path);
    if (!file_content.has_value()) return 1;

    fmt::println("========================================");
    fmt::println("                Raw Data                ");
    fmt::println("========================================");
    fmt::println("File Content:\n{}", *file_content);

    fmt::println("========================================");
    fmt::println("                Scanning                ");
    fmt::println("========================================");
    {
        Scanner scanner {*file_content};
        std::optional<TokenType> token = {};
        while ((token = scanner.next_token()).has_value()) {
            trace_token(scanner, token);
        }
    }
    fmt::println("========================================");
    fmt::println("             Save & Restore             ");
    fmt::println("========================================");
    {
        Scanner scanner {*file_content};
        std::optional<TokenType> token = {};
        size_t point = scanner.save();

        fmt::print("1: ");
        token = scanner.next_token();
        trace_token(scanner, token);

        fmt::print("2: ");
        token = scanner.next_token();
        trace_token(scanner, token);

        fmt::print("3: ");
        token = scanner.next_token();
        trace_token(scanner, token);

        fmt::println("Restoring...");
        scanner.restore(point);

        fmt::print("4: ");
        token = scanner.next_token();
        trace_token(scanner, token);
    }
}
