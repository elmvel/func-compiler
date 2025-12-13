#include <fmt/core.h>
#include <fmt/std.h>

#include "./ginstr.hh"

void print_binop(GBinop binop)
{
    switch (binop) {
        case GBinop::Add: fmt::print("Add"); break; 
        case GBinop::Sub: fmt::print("Sub"); break; 
        case GBinop::Mul: fmt::print("Mul"); break; 
        case GBinop::Div: fmt::print("Div"); break;
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
    print_binop(binop);
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
