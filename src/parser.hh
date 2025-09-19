#ifndef PARSER_H_
#define PARSER_H_

#include <functional>
#include <optional>

#include "scanner.hh"
#include "tree.hh"

class Parser
{
public:
    Parser(const Scanner& scanner)
        : m_scanner(scanner), m_head_token({})
    {}
    
    [[nodiscard]] TreeNode *parse_program();
    [[nodiscard]] TreeNode *parse_binding();
    [[nodiscard]] TreeNode *parse_param_list();
    [[nodiscard]] TreeNode *parse_param_seq();
    [[nodiscard]] TreeNode *parse_body();
    [[nodiscard]] TreeNode *parse_expr();
private:
    void match(TokenType type);
    void match(TokenType type, std::function<void(void)> closure);
    void next_token();
    bool has_token(TokenType type);
    void require_token(TokenType type);
private:
    Scanner m_scanner;
    std::optional<TokenType> m_head_token;
};

#endif // PARSER_H_
