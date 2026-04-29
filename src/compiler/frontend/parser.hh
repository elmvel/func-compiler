#ifndef PARSER_HH_
#define PARSER_HH_

#include <functional>
#include <optional>

#include "scanner.hh"
#include "tree.hh"

/*
  Parser::match is overloaded to support calling complex closures to retrieve
  information from the scanner before advancing it. For now, to support
  closures that do not return anything, return VOID_MATCH to avoid a
  compiletime error.
 */
#define VOID_MATCH 0

class Parser
{
public:
    Parser(const Scanner& scanner)
        : m_scanner(scanner), m_head_token({})
    {}
    
    [[nodiscard]] std::unique_ptr<TreeNode> parse_program();
    [[nodiscard]] std::unique_ptr<TreeNode> parse_binding();
    [[nodiscard]] std::unique_ptr<TreeNode> parse_param_list();
    [[nodiscard]] std::unique_ptr<TreeNode> parse_param_seq();
    [[nodiscard]] std::unique_ptr<TreeNode> parse_body();
    [[nodiscard]] std::unique_ptr<TreeNode> parse_expr(int precedence);
    [[nodiscard]] std::unique_ptr<TreeNode> parse_primary_expr(bool apply);
    [[nodiscard]] TypePtr parse_type();
    [[nodiscard]] TypePtr parse_type_primitive();
private:
    void match(TokenType type);
    void next_token();
    bool has_token(TokenType type);
    void require_token(TokenType type);
    bool could_start_primary();

    template <class F>
    auto match(TokenType type, F&& closure)
    {
        this->require_token(type);
        auto result = std::forward<F>(closure)();
        this->match(type);
        return result;
    }
private:
    Scanner m_scanner;
    std::optional<TokenType> m_head_token;
};

#endif // PARSER_HH_
