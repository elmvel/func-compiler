#include <functional>
#include <map>

#include <fmt/core.h>
#include <fmt/std.h>

#include "common.hh"
#include "parser.hh"

enum class Assoc {
    Left, Right
};

std::map<TokenType, std::pair<int, Assoc>> g_binary_ops = {
    {TokenType::Add, {0, Assoc::Left}},
    {TokenType::Sub, {0, Assoc::Left}},
    {TokenType::Mul, {1, Assoc::Left}},
    {TokenType::Div, {1, Assoc::Left}},
};

TreeNode *Parser::parse_program()
{
    std::vector<TreeNode *> nodes;
    m_head_token = m_scanner.next_token();
    
    while (m_head_token.has_value()) {
        TreeNode *binding = parse_binding();
        nodes.push_back(binding);
    }

    return new TreeSeqNode(nodes);
}

/*
<binding> ::= LET <id> <param-list> ":=" <body> END
            | LET <id> ":=" <expr>
 */
TreeNode *Parser::parse_binding()
{
    bool require_end = false;
    
    match(TokenType::Let);

    std::string name = match(TokenType::Id, [&](){
        return m_scanner.lexeme();
    });

    TreeNode *params = NULL;
    if (has_token(TokenType::LBracket)) {
        // Get parameter list
        params = parse_param_list();
        require_end = true;
    }

    match(TokenType::Assign);
    TreeNode *body = parse_body();
    if (require_end) match(TokenType::End);

    return new TreeBindingNode(name, body, params);
}

/*
<param-list> ::= "[" <param-seq> "]"
 */
TreeNode *Parser::parse_param_list()
{
    match(TokenType::LBracket);

    // Early return for empty params
    if (has_token(TokenType::RBracket)) {
        match(m_head_token.value());
        return new TreeListNode({});
    }
    
    TreeNode *params = parse_param_seq();
    match(TokenType::RBracket);
    return params;
}

/*
<param-seq>  ::= ID "," <param-seq> | ID
 */
TreeNode *Parser::parse_param_seq()
{
    std::vector<TreeNode *> nodes;
    
    match(TokenType::Id, [&](){
        TreeNode *ident = new TreeIdentNode(m_scanner.lexeme());
        nodes.push_back(ident);
        return VOID_MATCH;
    });

    while (has_token(TokenType::Comma)) {
        match(m_head_token.value());

        match(TokenType::Id, [&](){
            TreeNode *ident = new TreeIdentNode(m_scanner.lexeme());
            nodes.push_back(ident);
            return VOID_MATCH;
        });
    }

    return new TreeListNode(nodes);
}

/*
<body> ::= <binding> "," <body> | <expr>
 */
TreeNode *Parser::parse_body()
{
    // Ad Hoc, we cheat by checking for 'Let'
    if (has_token(TokenType::Let)) {
        TreeBindingNode *binding = static_cast<TreeBindingNode *>(parse_binding());
        match(TokenType::Comma);
        TreeNode *body = parse_body();

        binding->next = body;
        
        return binding;
    } else {
        return parse_expr(0);
    }
}

TreeNode *Parser::parse_expr(int precedence)
{
    TreeNode *lhs = parse_primary_expr(true);

    // While the next token is a binary operator
    while (m_head_token.has_value()) {
        auto it = g_binary_ops.find(*m_head_token);
        if (it == g_binary_ops.end()) break;
        if (it->second.first < precedence) break;

        TokenType op = *m_head_token;
        match(op);

        int next_prec = g_binary_ops[op].first;
        if (g_binary_ops[op].second == Assoc::Left) {
            ++next_prec;
        }

        TreeNode *rhs = parse_expr(next_prec);
        lhs = new TreeBinopNode(op, lhs, rhs);
    }

    return lhs;
}

// IMPORTANT: This needs to mirror parse_primary_expr exactly
bool Parser::could_start_primary() {
    if (has_token(TokenType::LParen)) return true;
    if (has_token(TokenType::Id)) return true;
    if (has_token(TokenType::Integer)) return true;
    if (has_token(TokenType::String)) return true;
    return false;
}

TreeNode *Parser::parse_primary_expr(bool apply)
{
    if (has_token(TokenType::LParen)) {
        match(m_head_token.value());
        TreeNode *expr = parse_expr(0);
        match(TokenType::RParen);
        return expr;
    } else if (has_token(TokenType::Id)) {
        TreeNode *lhs = match(TokenType::Id, [&](){
            return new TreeIdentNode(m_scanner.lexeme());
        });

        // IMPORTANT: This could be a function application
        if (apply) {
            while (could_start_primary()) {
                TreeNode *rhs = parse_primary_expr(false);
                lhs = new TreeApplyNode(lhs, rhs);
            }
        }

        return lhs;
    } else if (has_token(TokenType::Integer)) {
        // Int Literal
        return match(TokenType::Integer, [&](){
            return new TreeIntegerNode(m_scanner.number());
        });
    } else if (has_token(TokenType::String)) {
        // String Literal
        return match(TokenType::String, [&](){
            return new TreeStringNode(m_scanner.lexeme());
        });
    } else {
        if (!m_head_token.has_value()) {
            fmt::println("Could not parse an expression: EOF.");
            abort();
        } else {
            fmt::println("Could not parse an expression: Head token = {}", *m_head_token);
            abort();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//                              Private Functions
////////////////////////////////////////////////////////////////////////////////

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
