#include <cassert>
#include <fstream>
#include <sstream>
#include <string>
#include <optional>

#include <fmt/core.h>
#include <fmt/std.h>

#include "vendor/CLI11.hpp"

#include "file_reading.hh"
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
#include "backend/lambda_lifting.hh"
#include "backend/sc_to_gcode.hh"
// #include "backend/gcode_to_c.hh"
#include "backend/gcode_to_cxx.hh"

#define CLIARG(type, name, def, cli_name, cli_desc, req)   \
    type name = def;                                       \
    app.add_option(cli_name, name, cli_desc)->required(req);

#define CLIFLAG(type, name, def, cli_name, cli_desc, req)  \
    type name = def;                                       \
    app.add_flag(cli_name, name, cli_desc)->required(req);

LiftedProgram perform_lambda_lifting(LCNodePtr prog, bool print)
{
    LCTraceVisitor visitor_trace;
    LCSupercombinatorVisitor visitor_lambda_lifting;
    LCNodePtr lifted = prog;
    int rounds = 0;
    while (lifted->count_inner_lambdas() > 0) {
        if (print) fmt::println("==================================");
        lifted = lifted->accept(&visitor_lambda_lifting);
        ++rounds;

        if (print) {
            lifted->accept(&visitor_trace);
            for (auto& [name, elc] : visitor_lambda_lifting.supercombinators) {
                LCTraceVisitor vet;
                elc->accept(&vet);
                fmt::println("  $ {} = {}", name, vet.v_text.read_asserted());
            }
            fmt::println("Round {} of Lifting: {}", rounds, visitor_trace.v_text.read_asserted());
        }
    }

    // Recalculate let(rec) levels
    FreeVarsEnv _unused;
    lifted->find_free_vars(_unused);
    // Lift the toplevel let(rec)
    lifted = lifted->accept(&visitor_lambda_lifting);
    if (print) {
        fmt::println("========== Lifting defs in toplevel letrec =============");
        lifted->accept(&visitor_trace);
        for (auto& [name, elc] : visitor_lambda_lifting.supercombinators) {
            LCTraceVisitor vet;
            elc->accept(&vet);
            fmt::println("  $ {} = {}", name, vet.v_text.read_asserted());
        }
        fmt::println("Round {} of Lifting: {}", rounds, visitor_trace.v_text.read_asserted());
    }

    // Optimizations
    if (print) fmt::println("========== Optimizing with eta-reduction =============");

    // Perform Eta-Reductions a handful of times
    for (int i = 0; i < 10; ++i) {
        std::vector<std::pair<std::string, LCNodePtr>> to_replace;
        for (auto& [sc_name, sc] : visitor_lambda_lifting.supercombinators) {
            auto opt_elc = eta_reduction(visitor_lambda_lifting.supercombinators, sc);
            if (opt_elc.has_value()) {
                to_replace.push_back({sc_name, opt_elc.value()});
            }
        }
        for (auto& [sc_name, eta_reduced] : to_replace) {
            visitor_lambda_lifting.supercombinators[sc_name] = eta_reduced;
        }
    }

    // Eliminate all redundant definitions
    while (1) {
        std::optional<std::pair<std::string, std::string>> to_replace {};
        for (auto& [sc_name, sc] : visitor_lambda_lifting.supercombinators) {
            LCConstantNode *constant = dynamic_cast<LCConstantNode *>(sc.get());
            if (constant == nullptr) continue;
            to_replace = {sc_name, constant->name};
        }
        if (!to_replace.has_value()) break;
        auto& [from, to] = to_replace.value();
        visitor_lambda_lifting.supercombinators.erase(from);
        lifted->sc_rewrite(from, to);

        // Remove any 'sc_name = from' supercombinator definitions.
        for (auto& [sc_name, sc] : visitor_lambda_lifting.supercombinators) {
            LCConstantNode *constant = dynamic_cast<LCConstantNode *>(sc.get());
            if (constant == nullptr) continue;
            if (constant->name == from) {
                constant->name = to;
            }
        }
    }

    // Finally, lift main body into its own PROG supercombinator
    visitor_lambda_lifting.supercombinators["__PROG"] = lifted;
    lifted = std::make_shared<LCConstantNode>("__PROG");

    if (print) {
        for (auto& [name, elc] : visitor_lambda_lifting.supercombinators) {
            LCTraceVisitor vet;
            elc->accept(&vet);
            fmt::println("  $ {} = {}", name, vet.v_text.read_asserted());
        }
        lifted->accept(&visitor_trace);
        fmt::println("Final Result: {}", visitor_trace.v_text.read_asserted());
    }
    return {visitor_lambda_lifting, lifted};
}

bool finish_compilation(GCodeToCXXCompiler& gcode_to_cxx)
{
    if (!write_file("./out.cc", gcode_to_cxx.code)) return false;

    system("g++ -std=c++20 -o ./f.out ./out.cc");

    return true;
}

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
    CLIFLAG(bool, dump_lifting, false, "-L,--dump-lifting", "Debug the lambda lifting process.", false);
    CLIFLAG(bool, dump_gcode, false, "--gc,--dump-gcode", "Dump the compiled G-Machine code.", false);
    CLIFLAG(bool, dump_gmachine, false, "--gm,--dump-gmachine", "Dump the C++ G-Machine implementation code.", false);
    CLIFLAG(bool, no_compilation, false, "--nc,--no-comp", "Exit before compiling G-Machine code.", false);

    CLI11_PARSE(app, argc, argv);

    auto file_content = read_file(file_path);
    if (!file_content.has_value()) return EXIT_FAILURE;

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
        return EXIT_SUCCESS;
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
        // Unsafe reading, so assert first
        assert(visitor_elc.v_elc.is_valid());
        LCNodePtr prog = visitor_elc.v_elc.value;
        prog->accept(&visitor_elc_trace);
        fmt::println("{}", visitor_elc_trace.v_text.read_asserted());
    }

    LCNodePtr elc_prog = visitor_elc.v_elc.read_asserted();
    LiftedProgram lp = perform_lambda_lifting(elc_prog, dump_lifting);

    SCToGCodeCompiler sc_to_gcode;
    sc_to_gcode.compile(lp);

    if (dump_gcode) {
        fmt::println("GCode Program:");
        for (auto& instr : sc_to_gcode.output) {
            instr->dump(0);
        }
    }
    
    // Not in use.
    // GCodeToCCompiler gcode_to_c;
    // gcode_to_c.compile(sc_to_gcode.output);
    GCodeToCXXCompiler gcode_to_cxx;
    gcode_to_cxx.compile(sc_to_gcode.output, dump_gmachine);

    if (no_compilation)
    {
        return EXIT_SUCCESS;
    }

    if (!finish_compilation(gcode_to_cxx))
    {
        fmt::println("An error occurred when trying to compile output program.");
        return EXIT_FAILURE;
    }
}
