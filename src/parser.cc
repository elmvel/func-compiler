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

std::unique_ptr<TreeNode> Parser::parse_program()
{
    std::vector<std::unique_ptr<TreeNode>> nodes;
    m_head_token = m_scanner.next_token();
    
    while (m_head_token.has_value()) {
        std::unique_ptr<TreeNode> binding = parse_binding();
        nodes.push_back(std::move(binding));
    }

    return std::make_unique<TreeSeqNode>(std::move(nodes));
}

/*
<binding> ::= LET <id> [":" <type>] <param-list> ":=" <body> END
            | LET <id> [":" <type>] ":=" <expr>
 */
std::unique_ptr<TreeNode> Parser::parse_binding()
{
    bool require_end = false;
    
    match(TokenType::Let);

    std::string name = match(TokenType::Id, [&](){
        return m_scanner.lexeme();
    });

    std::optional<Type *> type_hint;
    if (has_token(TokenType::Colon)) {
        next_token();

        type_hint = parse_type();
    }

    std::unique_ptr<TreeNode> params {};
    if (has_token(TokenType::LBracket)) {
        // Get parameter list
        params = parse_param_list();
        require_end = true;
    }

    match(TokenType::Assign);
    std::unique_ptr<TreeNode> body = parse_body();
    if (require_end) match(TokenType::End);

    auto node = std::make_unique<TreeBindingNode>(name, body, params);
    if (type_hint.has_value()) {
        node->attr_type = *type_hint;
    }
    return node;
}

/*
<param-list> ::= "[" <param-seq> "]"
 */
std::unique_ptr<TreeNode> Parser::parse_param_list()
{
    match(TokenType::LBracket);

    // Early return for empty params
    if (has_token(TokenType::RBracket)) {
        match(m_head_token.value());
        return std::make_unique<TreeParamsNode>(std::vector<std::unique_ptr<TreeNode>> {});
    }
    
    std::unique_ptr<TreeNode> params = parse_param_seq();
    match(TokenType::RBracket);
    return params;
}

/*
<param-seq>  ::= ID [":" <type>] "," <param-seq> | ID
 */
std::unique_ptr<TreeNode> Parser::parse_param_seq()
{
    std::vector<std::unique_ptr<TreeNode>> nodes;
    
    std::unique_ptr<TreeNode> ident = match(TokenType::Id, [&](){
        return std::make_unique<TreeIdentNode>(m_scanner.lexeme());
    });

    if (has_token(TokenType::Colon)) {
        next_token();

        Type *type = parse_type();
        static_cast<TreeIdentNode *>(ident.get())->attr_type = type;
    }

    nodes.push_back(std::move(ident));

    while (has_token(TokenType::Comma)) {
        next_token();

        std::unique_ptr<TreeNode> ident = match(TokenType::Id, [&](){
            return std::make_unique<TreeIdentNode>(m_scanner.lexeme());
        });

        if (has_token(TokenType::Colon)) {
            next_token();

            Type *type = parse_type();
            static_cast<TreeIdentNode *>(ident.get())->attr_type = type;
        }

        nodes.push_back(std::move(ident));
    }

    return std::make_unique<TreeParamsNode>(std::move(nodes));
}

/*
<body> ::= <binding> "," <body> | <expr>
 */
std::unique_ptr<TreeNode> Parser::parse_body()
{
    // Ad Hoc, we cheat by checking for 'Let'
    if (has_token(TokenType::Let)) {
        std::unique_ptr<TreeNode> binding = parse_binding();
        TreeBindingNode *alias_binding = static_cast<TreeBindingNode *>(binding.get());
        
        match(TokenType::Comma);
        std::unique_ptr<TreeNode> body = parse_body();

        alias_binding->next = std::move(body);
        
        return binding;
    } else {
        return parse_expr(0);
    }
}

std::unique_ptr<TreeNode> Parser::parse_expr(int precedence)
{
    std::unique_ptr<TreeNode> lhs = parse_primary_expr(true);

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

        std::unique_ptr<TreeNode> rhs = parse_expr(next_prec);
        lhs = std::make_unique<TreeBinopNode>(op, lhs, rhs);
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

std::unique_ptr<TreeNode> Parser::parse_primary_expr(bool apply)
{
    if (has_token(TokenType::LParen)) {
        match(m_head_token.value());
        std::unique_ptr<TreeNode> expr = parse_expr(0);
        match(TokenType::RParen);
        return expr;
    } else if (has_token(TokenType::Id)) {
        std::unique_ptr<TreeNode> lhs = match(TokenType::Id, [&](){
            return std::make_unique<TreeIdentNode>(m_scanner.lexeme());
        });

        // IMPORTANT: This could be a function application
        if (apply) {
            while (could_start_primary()) {
                std::unique_ptr<TreeNode> rhs = parse_primary_expr(false);
                lhs = std::make_unique<TreeApplyNode>(std::move(lhs), std::move(rhs));
            }
        }

        return lhs;
    } else if (has_token(TokenType::Integer)) {
        // Int Literal
        return match(TokenType::Integer, [&](){
            return std::make_unique<TreeIntegerNode>(m_scanner.number());
        });
    } else if (has_token(TokenType::String)) {
        // String Literal
        return match(TokenType::String, [&](){
            return std::make_unique<TreeStringNode>(m_scanner.lexeme());
        });
    } else {
        if (!m_head_token.has_value()) {
            COMPILER_ERROR_TERM("Could not parse an expression: EOF.");
        } else {
            COMPILER_ERROR_TERM("Could not parse an expression: Head token = {}", *m_head_token);
        }
    }
}

Type *Parser::parse_type_primitive()
{
    if (has_token(TokenType::LParen)) {
        next_token();
        
        Type *type = parse_type();
        match(TokenType::RParen);
        return type;
    } else if (has_token(TokenType::TyInt)) {
        next_token();

        return new Type(TypePrimitive::Integer);
    } else if (has_token(TokenType::TyString)) {
        next_token();

        return new Type(TypePrimitive::String);
    } else {
        if (!m_head_token.has_value()) {
            COMPILER_ERROR_TERM("Could not parse a type: EOF.");
        } else {
            COMPILER_ERROR_TERM("Could not parse a type: Head token = {}", *m_head_token);
        }
    }
}

Type *Parser::parse_type()
{
    Type *lhs = parse_type_primitive();
    if (has_token(TokenType::Arrow)) {
        next_token();

        Type *rhs = parse_type();
        lhs = new Type(TypeFunction(lhs, rhs));
    }
    return lhs;
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
            // TODO: allow for error recovery in some cases if time allows
            COMPILER_ERROR_TERM("ERROR: Parser wanted to match a token but got {}.", type);
        }
    } else {
        COMPILER_ERROR_TERM("ERROR: Parser wanted to match a token but got EOF.");
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
        COMPILER_ERROR_TERM("error: Expected token type {}, but got EOF", type);
    }
    if (!has_token(type)) {
        COMPILER_ERROR_TERM("error: Expected token type {}, but got {}", type, *m_head_token);
    }
}
