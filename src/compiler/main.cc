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
#include "frontend/scanner_debug.hh"
#include "frontend/parser.hh"
#include "frontend/symtab.hh"
#include "frontend/sema.hh"
#include "frontend/tree_debug.hh"
#include "backend/lambda.hh"
#include "backend/lambda_debug.hh"
#include "backend/high_to_elc.hh"

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

#define CLIARG(type, name, def, cli_name, cli_desc, req)   \
    type name = def;                                       \
    app.add_option(cli_name, name, cli_desc)->required(req);

#define CLIFLAG(type, name, def, cli_name, cli_desc, req)  \
    type name = def;                                       \
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
