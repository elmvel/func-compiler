#include <fmt/core.h>
#include <fmt/std.h>

#include "./ginstr.hh"
#include "../common.hh"

std::string_view binop_to_name(GBinop binop)
{
    switch (binop) {
        case GBinop::Add: return "Add";
        case GBinop::Sub: return "Sub";
        case GBinop::Mul: return "Mul";
        case GBinop::Div: return "Div";
        default: INTERNAL_ERROR("Unreachable binop name.");
    }
}

std::string_view binop_to_vendor(GBinop binop)
{
    switch (binop) {
        case GBinop::Add: return "_int::ADD_M";
        case GBinop::Sub: return "_int::SUB_M";
        case GBinop::Mul: return "_int::MUL_M";
        case GBinop::Div: return "_int::DIV_M";
        default: INTERNAL_ERROR("Unreachable binop name.");
    }
}

void print_indentation(int level)
{
    for (int i = 0; i < level; ++i)
        fmt::print("  ");
}

void GInstrPushInt::dump(int level)
{
    print_indentation(level);
    fmt::println("PushInt({})", value);
}

void GInstrPushGlobal::dump(int level)
{
    print_indentation(level);
    fmt::println("PushGlobal({})", name);
}

void GInstrPush::dump(int level)
{
    print_indentation(level);
    fmt::println("Push({})", offset);
}

void GInstrPop::dump(int level)
{
    print_indentation(level);
    fmt::println("Pop({})", n);
}

void GInstrMkApp::dump(int level)
{
    print_indentation(level);
    fmt::println("MkApp()");
}

void GInstrUnwind::dump(int level)
{
    print_indentation(level);
    fmt::println("Unwind()");
}

void GInstrUpdate::dump(int level)
{
    print_indentation(level);
    fmt::println("Update({})", offset);
}

void GInstrPack::dump(int level)
{
    print_indentation(level);
    fmt::println("Pack({}, {})", tag, n);
}

void GInstrSplit::dump(int level)
{
    print_indentation(level);
    fmt::println("Split()");
}

void GInstrJump::dump(int level)
{
    print_indentation(level);
    // TODO: implement debug printing for GInstrJump
    fmt::println("Jump(...<printing unimplemented>...)");
}

void GInstrSlide::dump(int level)
{
    print_indentation(level);
    fmt::println("Slide({})", n);
}

void GInstrBinOp::dump(int level)
{
    print_indentation(level);
    fmt::print("BinOp(");
    fmt::print("{}", binop_to_name(this->binop));
    fmt::println(")");
}

void GInstrEval::dump(int level)
{
    print_indentation(level);
    fmt::println("Eval()");
}

void GInstrAlloc::dump(int level)
{
    print_indentation(level);
    fmt::println("Alloc({})", n);
}

void GInstrBegin::dump(int level)
{
    print_indentation(level);
    fmt::println("Begin()");
}

void GInstrEnd::dump(int level)
{
    print_indentation(level);
    fmt::println("End()");
}

void GInstrPrint::dump(int level)
{
    print_indentation(level);
    fmt::println("Print()");
}

void GInstrGlobStart::dump(int level)
{
    print_indentation(level);
    fmt::println("GlobStart({}, {})", name, n);
}

void GInstrGlobEnd::dump(int level)
{
    print_indentation(level);
    fmt::println("GlobEnd({}, {})", name, n);
}

/*
  Compilation
*/
void GInstrPushInt::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrPushInt\n");
    output.append(fmt::format("    code.push_back({{ _int::PUSH, std::pair(_int::VALUE, {}) }});\n", value));
}

void GInstrPushGlobal::compile(std::string& output, CompilerSCoMap& scomap)
{
    int scoid = scomap.fetch_or_insert(name);
    // Specifically we use push global within the main body,
    // so we have access to init_code.
    output.append("    // GInstrPushGlobal\n");
    output.append(fmt::format("    code.push_back({{ _int::PUSH, std::pair(_int::GLOB, {}) }});\n", scoid));
}

void GInstrPush::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrPush\n");
    // @check This might be correct?
    if (is_param)
    {
        output.append(fmt::format("    code.push_back({{ _int::PUSH, std::pair(_int::ARG, {}) }});\n", offset));
    }
    else
    {
        output.append(fmt::format("    code.push_back({{ _int::PUSH, std::pair(_int::LOCAL, {}) }});\n", offset));
    }
}

void GInstrPop::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrPop\n");
    output.append(fmt::format("    code.push_back({{ _int::POP, {} }});\n", n));
}

void GInstrMkApp::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrMkApp\n");
    // 
    /*
      For some reason, the vendor implementation ordered it
        n0 n1 ... bot : Stack, G[]
        MKAP
          -> n ... bot : Stack, G[n = App n1 n0]

      Peyton Jones orders it the other way, so we will keep with this convention.
     */
    output.append("    code.push_back({_int::MKAPSPJ, std::monostate()});\n");
}

void GInstrUnwind::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrUnwind\n");
    output.append("    code.push_back({_int::UNWIND, std::monostate()});\n");
}

void GInstrUpdate::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrUpdate\n");
    output.append(fmt::format("    code.push_back({{ _int::UPDATE, {} }});\n", offset));
}

void GInstrPack::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(output);
    UNUSED(scomap);
    INTERNAL_ERROR("TODO: GInstrPack");
}

void GInstrSplit::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(output);
    UNUSED(scomap);
    INTERNAL_ERROR("TODO: GInstrSplit");
}

void GInstrJump::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(output);
    UNUSED(scomap);
    INTERNAL_ERROR("TODO: GInstrJump");
}

void GInstrSlide::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrSlide\n");
    output.append(fmt::format("    code.push_back({{ _int::SLIDE, {} }});\n", n));
}

void GInstrBinOp::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    std::string_view name = binop_to_name(binop);
    std::string_view vname = binop_to_vendor(binop);
    output.append(fmt::format("    // GInstrBinOp({})\n", name));
    output.append(fmt::format("    code.push_back({{ _int::PUSH, std::pair(_int::GLOB, {}) }});\n", vname));
    output.append("    code.push_back({_int::MKAPSPJ, std::monostate()});\n");
    output.append("    code.push_back({_int::MKAPSPJ, std::monostate()});\n");
    // @check Might need more work
}

void GInstrEval::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrEval\n");
    output.append("    code.push_back({ _int::EVAL, std::monostate() });\n");
}

void GInstrAlloc::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(output);
    UNUSED(scomap);
    INTERNAL_ERROR("TODO: GInstrAlloc");
}

void GInstrBegin::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("// GInstrBegin\n");
    output.append(
        "int compiled_program(GMSCoMap* globals) {\n"
        "    std::vector<_int::GmInstr> code{};\n"
        );
}

void GInstrEnd::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrEnd\n");
    output.append(
        "    std::reverse(code.begin(), code.end());\n"
        "    _int::i_stack<_int::GmInstr> init{};\n"
        "    init.insert(std::move(code));\n"
        "    _int::G_machine g = {globals, _int::stack_i{}, _int::dump_t{}, std::move(init)};\n"
        "    return g.eval();\n"
        "}\n"
        "void register_supercombinators(GMSCoMap* globals) {\n"
        "    std::vector<_int::GmInstr> code{};\n"
        );
}

void GInstrPrint::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(output);
    UNUSED(scomap);
    // I think it's easiest I just leave this to the runtime, we'll omit this.
}

void GInstrGlobStart::compile(std::string& output, CompilerSCoMap& scomap)
{
    UNUSED(scomap);
    output.append("    // GInstrGlobStart\n");
    // We reuse the code block for every supercombinator.
    output.append("    code = {};\n");
}

void GInstrGlobEnd::compile(std::string& output, CompilerSCoMap& scomap)
{
    int scoid = scomap.fetch_or_insert(name);

    output.append("    // GInstrGlobEnd\n");
    output.append("    std::reverse(code.begin(), code.end());\n");
    output.append(fmt::format("    (*globals)[{}] = {{ {}, std::move(code) }};\n", scoid, n));
}
