#include <functional>

#include <fmt/core.h>
#include <fmt/std.h>

#include "common.hh"
#include "parser.hh"

struct TreeNodeSeq
{
    TreeNodeSeq()
        : root(nullptr), head(nullptr)
    {}
    
    TreeNode *root;
    TreeNode *head;

    void append(TreeNode *node)
    {
        if (root == nullptr) {
            root = node;
            head = root;
        } else {
            head->next() = node;
            head = head->next();
        }
    }
};

TreeNode *Parser::parse_program()
{
    TreeNodeSeq seq{};
    m_head_token = m_scanner.next_token();
    
    while (m_head_token.has_value()) {
        TreeNode *binding = parse_binding();
        seq.append(binding);
    }

    return seq.root;
}

/*
<binding> ::= LET <id> <param-list> ":=" <body> END
            | LET <id> ":=" <expr>
 */
TreeNode *Parser::parse_binding()
{
    bool require_end = false;
    
    match(TokenType::Let);

    require_token(TokenType::Id);
    std::string name = m_scanner.lexeme();
    match(TokenType::Id);

    TreeNode *params = NULL;
    if (has_token(TokenType::LBracket)) {
        // Get parameter list
        params = parse_param_list();
        require_end = true;
    }
    match(TokenType::Assign);
    TreeNode *body = parse_body();
    if (require_end) match(TokenType::End);

    return new_binding_node(name, body, params);
}

/*
<param-list> ::= "[" <param-seq> "]"
 */
TreeNode *Parser::parse_param_list()
{
    match(TokenType::LBracket);
    TreeNode *params = parse_param_seq();
    match(TokenType::RBracket);
    return params;
}

/*
<param-seq>  ::= ID "," <param-seq> | ID
 */
TreeNode *Parser::parse_param_seq()
{
    TreeNodeSeq seq{};
    
    // TODO: find a way to encapsulate
    // require_token(TokenType::Id);
    // TreeNode *ident = new_expr_node(ExprKind::Ident, m_scanner.lexeme());
    // seq.append(ident);
    // match(TokenType::Id);
    match(TokenType::Id, [&](){
        TreeNode *ident = new_expr_node(ExprKind::Ident, m_scanner.lexeme());
        seq.append(ident);
    });

    while (has_token(TokenType::Comma)) {
        match(m_head_token.value());

        match(TokenType::Id, [&](){
            TreeNode *ident = new_expr_node(ExprKind::Ident, m_scanner.lexeme());
            seq.append(ident);
        });
    }

    return seq.root;
}

/*
<body> ::= <binding> "," <body> | <expr>
 */
TreeNode *Parser::parse_body()
{
    // Ad Hoc, we cheat by checking for 'Let'
    if (has_token(TokenType::Let)) {
        TreeNode *binding = parse_binding();
        match(TokenType::Comma);
        TreeNode *body = parse_body();

        binding->next() = body;
        
        return binding;
    } else {
        return parse_expr();
    }
}

TreeNode *Parser::parse_expr()
{
    // TreeNode *expr = match_ret(TokenType::Integer, [&](){
    //     return new_expr_node(ExprKind::Integer, m_scanner.number());
    // });
    require_token(TokenType::Integer);
    TreeNode *expr = new_expr_node(ExprKind::Integer, m_scanner.number());
    match(TokenType::Integer);
    return expr;
    // TODOF();
}

// Private Functions

void Parser::match(TokenType type)
{
    if (m_head_token.has_value()) {
        if (m_head_token.value() == type) {
            next_token();
        } else {
            fmt::println("ERROR: Parser wanted to match a token but got {}.", type);
            fmt::println("TODO: Improve error handling within parser.");
            abort();
        }
    } else {
        fmt::println("ERROR: Parser wanted to match a token but got EOF.");
        fmt::println("TODO: Improve error handling within parser.");
        abort();
    }
}

void Parser::match(TokenType type, std::function<void(void)> closure)
{
    require_token(type);
    closure();
    match(type);
}

void Parser::next_token()
{
    m_head_token = m_scanner.next_token();
}

bool Parser::has_token(TokenType type)
{
    if (!m_head_token.has_value()) return false;
    return *m_head_token == type;
}

void Parser::require_token(TokenType type)
{
    if (!m_head_token.has_value()) {
        fmt::println("error: Expected token type {}, but got EOF", type);
        abort();
    }
    if (!has_token(type)) {
        fmt::println("error: Expected token type {}, but got {}", type, *m_head_token);
        abort();
    }
}
