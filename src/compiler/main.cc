#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>

#include <fmt/core.h>
#include <fmt/std.h>

#include "CLI11.hpp"

#include "common.hh"
#include "frontend/scanner.hh"
#include "frontend/parser.hh"
#include "frontend/symtab.hh"
#include "frontend/sema.hh"
#include "backend/lambda.hh"
#include "backend/lambda_debug.hh"
#include "backend/high_to_elc.hh"

// #define ONLY_SCAN
// #define ONLY_PARSE
// #define ONLY_SEMA
#define ONLY_ELC_GEN

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

// Smart pointers
// Visitor pattern

struct TreeTraceVisitor : ITreeVisitor
{
    virtual void visit(TreeSeqNode *node)
    {
        std::string local_text = "";
        for (auto& child : node->children) {
            child->accept(this);
            local_text.append(text);
            local_text.append("\n\n");
        }
        text = local_text;
    }

    virtual void visit(TreeParamsNode *node)
    {
        std::string local_text = "";
        local_text.push_back('[');
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (i != 0) {
                local_text.append(", ");
            }
            node->children[i]->accept(this);
            local_text.append(text);
        }
        local_text.push_back(']');
        text = local_text;
    }

    virtual void visit(TreeListNode *node)
    {
        std::string local_text = "";
        local_text.push_back('[');
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (i != 0) {
                local_text.append(", ");
            }
            node->children[i]->accept(this);
            local_text.append(text);
        }
        local_text.push_back(']');
        text = local_text;
    }

    virtual void visit(TreeBindingNode *node)
    {
        std::string local_text = "Let(";

        local_text.append(node->id);
        if (node->params != nullptr) {
            local_text.append(", ");
            node->params->accept(this);
            local_text.append(text);
        }
        local_text.append(")\n");

        level += 1;
        for (int i = 0; i < level; ++i) {
            local_text.append("    ");
        }
        node->body->accept(this);
        local_text.append(text);
        level -= 1;

        if (node->next != nullptr) {
            local_text.append(",\n");
            for (int i = 0; i < level; ++i) {
                local_text.append("    ");
            }
            node->next->accept(this);
            local_text.append(text);
        }

        text = local_text;
    }

    virtual void visit(TreeBinopNode *node)
    {
        std::string local_text = fmt::format("{}(", node->op);
        node->lhs->accept(this);
        local_text.append(text);
        local_text.append(", ");
        node->rhs->accept(this);
        local_text.append(text);
        local_text.append(")");
        
        text = local_text;
    }

    virtual void visit(TreeApplyNode *node)
    {
        std::string local_text = "Apply(";

        node->func->accept(this);
        local_text.append(text);
        local_text.append(", ");
        node->arg->accept(this);
        local_text.append(text);
        local_text.append(")");
        
        text = local_text;
    }

    virtual void visit(TreeMatchNode *node)
    {
        std::string local_text = "Match(";

        node->expr->accept(this);
        local_text.append(text);
        for (auto& node : node->arms) {
            local_text.append(", ");
            node->accept(this);
            local_text.append(text);
        }
        local_text.append(")");
        

        text = local_text;
    }

    virtual void visit(TreeMatchArmNode *node)
    {
        std::string local_text = "";
        
        node->pattern->accept(this);
        local_text.append(text);
        local_text.append(" => ");
        node->body->accept(this);
        local_text.append(text);

        text = local_text;
    }

    virtual void visit(TreeIdentNode *node)
    {
        text = node->name;
    }

    virtual void visit(TreeIntegerNode *node)
    {
        std::string local_text = fmt::format("{}", node->value);
        text = local_text;
    }

    virtual void visit(TreeStringNode *node)
    {
        text = fmt::format("\"{}\"", node->text);
    }

    std::string text;
    int level = 0;
};

#define CLIARG(type, name, def, cli_name, cli_desc, req)       \
    type name = def;                                           \
    app.add_option(cli_name, name, cli_desc)->required(req);

#define CLIFLAG(type, name, def, cli_name, cli_desc, req)       \
    type name = def;                                           \
    app.add_flag(cli_name, name, cli_desc)->required(req);

int main(int argc, char *argv[])
{
    CLI::App app {"A Functional Compiler"};
    argv = app.ensure_utf8(argv);

    CLIARG(std::string, file_path, "", "file-path", "The path of the fc to compile.", true);
    CLIFLAG(bool, dump_file, false, "-d,--dump-file", "Output the input file.", false);
    CLIFLAG(bool, dump_tokens, false, "-t,--dump-tokens", "Output scanned tokens.", false);
    CLIFLAG(bool, dump_ast, false, "--ast,--dump-ast", "Output parse tree.", false);
    CLIFLAG(bool, no_sema, false, "--ns,--no-sema", "Exit before semantic analysis.", false);
    CLIFLAG(bool, dump_elc, false, "-E,--dump-elc", "Output the lambda calculus.", false);

    CLI11_PARSE(app, argc, argv);

    auto file_content = read_file(file_path);
    if (!file_content.has_value()) return 1;

    if (dump_file) {
        fmt::println("========================================");
        fmt::println("                Raw Data                ");
        fmt::println("========================================");
        fmt::println("File Content:\n{}", *file_content);
    }

    Scanner scanner {*file_content};

    if (dump_tokens) {
        fmt::println("========================================");
        fmt::println("                 Tokens                 ");
        fmt::println("========================================");
        Scanner scannerTrace {*file_content};

        std::optional<TokenType> token = {};
        while ((token = scannerTrace.next_token()).has_value()) {
            trace_token(scannerTrace, token);
        }
    }
    
    Parser parser {scanner};
    std::unique_ptr<TreeNode> root = parser.parse_program();

    if (dump_ast) {
        fmt::println("========================================");
        fmt::println("                  AST                   ");
        fmt::println("========================================");
        TreeTraceVisitor visitor;
        root->accept(&visitor);
        fmt::println("{}", visitor.text);
    }

    if (no_sema) {
        fmt::println("Skipping semantic analysis, exiting...");
        return 0;
    }
    
    // Symbol Table Pass
    TreeSymtabVisitor visitor_symtab;
    root->accept(&visitor_symtab);
    
    // Semantic Analysis Pass
    TreeSemaVisitor visitor_sema(visitor_symtab.table);
    root->accept(&visitor_sema);
    
    if (!visitor_sema.valid) COMPILER_TERM();
    
    TreeToELCVisitor visitor_elc;
    root->accept(&visitor_elc);

    if (dump_elc) {
        fmt::println("========================================");
        fmt::println("                  ELC                   ");
        fmt::println("========================================");

        LCTraceVisitor visitor_elc_trace;
        LCNodePtr prog = visitor_elc.v_elc.read_asserted();
        prog->accept(&visitor_elc_trace);
        fmt::println("{}", visitor_elc_trace.v_text.read_asserted());
    }
    
    fmt::println("Finished Pass: lowering to ELC.");
}
