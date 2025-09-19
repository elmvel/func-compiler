#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>

#include <fmt/core.h>
#include <fmt/std.h>

#include "common.hh"
#include "scanner.hh"
#include "parser.hh"

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

template <typename... Args>
void print_indented(int level, std::string_view format_str, Args&&... args) {
    fmt::print("{:{}}", "", level * 4);
    fmt::print(format_str, std::forward<Args>(args)...);
}

std::string trace_params(TreeNode *params)
{
    std::string text = "[";
    TreeNode *first = params;
    while (first != nullptr) {
        text.append(std::get<std::string>(first->attr));
        if (first->next() != nullptr) {
            text.append(", ");
        }
        first = first->next();
    }
    text.append("]");

    return text;
}

// Questionable Performance
void trace_tree(TreeNode *root, int level = 0)
{
    if (!root) return;
    if (root->kind == NodeKind::Binding) {
        if (root->right() != nullptr) {
            std::string params = trace_params(root->right());
            std::string text = fmt::format("Let({}, {})\n", std::get<std::string>(root->attr), params);
            print_indented(level, text);
        } else {
            std::string text = fmt::format("Let({})\n", std::get<std::string>(root->attr));
            print_indented(level, text);
        }
        trace_tree(root->left(), level + 1);
        fmt::println("");
        trace_tree(root->next());
    } else {
        assert(root->expr_kind.has_value());
        switch (root->expr_kind.value()) {
            case ExprKind::Integer: {
                std::string text = fmt::format("{}", std::get<int>(root->attr));
                print_indented(level, text);
            } break;
            default: {
                fmt::println("error: unsupported trace kind");
                abort();
            } break;
        }
    }
}

// Smart pointers
// Visitor pattern

int main()
{
    std::string file_path = "examples/test.fc";
    auto file_content = read_file(file_path);
    if (!file_content.has_value()) return 1;

    fmt::println("========================================");
    fmt::println("                Raw Data                ");
    fmt::println("========================================");
    fmt::println("File Content:\n{}", *file_content);

#if 0
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
#endif

    fmt::println("========================================");
    fmt::println("                 Parsing                ");
    fmt::println("========================================");
    {
        Scanner scanner {*file_content};
        Parser parser {scanner};
        TreeNode *root = parser.parse_program();
        trace_tree(root);
    }
}

// compilers
// cs435!
