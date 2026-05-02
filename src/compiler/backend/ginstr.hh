#ifndef GINSTR_HH_
#define GINSTR_HH_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// This is used more in gcode_to_cxx, but its easier to put here
struct CompilerSCoMap
{
    int fetch_or_insert(const std::string& name);
    
    // The GM implementation im referencing always has the first 6 ids
    // (0..5) mapped, so lets be safe and start at 10.
    int counter = 10;
    std::unordered_map<std::string, int> container {
        {"ADD", 0},
        {"SUB", 1},
        {"MUL", 3},
        {"DIV", 2},
    };
};

/*
  Binary operation supported by the G-Machine.
*/
enum class GBinop
{
    Add,
    Sub,
    Mul,
    Div,
};

void print_indentation(int level);
void print_binop(GBinop binop);

/*
  Instruction abstraction for the G-Machine.
*/
struct GInstr
{
    virtual ~GInstr()
    {}

    virtual void dump(int level) = 0;
    virtual void compile(std::string& output, CompilerSCoMap& scomap) = 0;
};

using GInstrPtr = std::unique_ptr<GInstr>;

struct GInstrPushInt : GInstr
{
    GInstrPushInt(int value)
        : value(value)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    int value;
};

struct GInstrPushGlobal : GInstr
{
    GInstrPushGlobal(const std::string& name)
        : name(name)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    std::string name;
};

struct GInstrPush : GInstr
{
    GInstrPush(int offset)
        : offset(offset)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    int offset;
};

struct GInstrPop : GInstr
{
    GInstrPop(int n)
        : n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    int n;
};


struct GInstrMkApp : GInstr
{
    GInstrMkApp()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
};

struct GInstrUnwind : GInstr
{
    GInstrUnwind()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
};

struct GInstrUpdate : GInstr
{
    GInstrUpdate(int offset)
        : offset(offset)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    int offset;
};

struct GInstrPack : GInstr
{
    GInstrPack(int tag, int n)
        : tag(tag), n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);

    int tag, n;
};

struct GInstrSplit : GInstr
{
    GInstrSplit()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
};

struct GInstrJump : GInstr
{
    GInstrJump()
        : branches(), tag_map()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    std::vector<std::vector<GInstrPtr>> branches;
    std::unordered_map<int, int> tag_map;
};

struct GInstrSlide : GInstr
{
    GInstrSlide(int n)
        : n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    int n;
};

struct GInstrBinOp : GInstr
{
    GInstrBinOp(GBinop binop)
        : binop(binop)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    GBinop binop;
};

struct GInstrEval : GInstr
{
    GInstrEval()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
};

struct GInstrAlloc : GInstr
{
    GInstrAlloc(int n)
        : n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
    
    int n;
};

/*
  Pseudo-Instructions.
*/

struct GInstrBegin : GInstr
{
    GInstrBegin()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
};

struct GInstrEnd : GInstr
{
    GInstrEnd()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
};

struct GInstrPrint : GInstr
{
    GInstrPrint()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);
};

struct GInstrGlobStart : GInstr
{
    GInstrGlobStart(const std::string& name, int n)
        : name(name), n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);

    std::string name;
    int n;
};

struct GInstrGlobEnd : GInstr
{
    GInstrGlobEnd(const std::string& name, int n)
        : name(name), n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output, CompilerSCoMap& scomap);

    std::string name;
    int n;
};

#endif // GINSTR_HH_
