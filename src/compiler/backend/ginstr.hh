#ifndef GINSTR_HH_
#define GINSTR_HH_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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
    virtual void compile(std::string& output) = 0;
};

using GInstrPtr = std::unique_ptr<GInstr>;

struct GInstrPushInt : GInstr
{
    GInstrPushInt(int value)
        : value(value)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    int value;
};

struct GInstrPushGlobal : GInstr
{
    GInstrPushGlobal(const std::string& name)
        : name(name)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    std::string name;
};

struct GInstrPush : GInstr
{
    GInstrPush(int offset)
        : offset(offset)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    int offset;
};

struct GInstrPop : GInstr
{
    GInstrPop(int n)
        : n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    int n;
};


struct GInstrMkApp : GInstr
{
    GInstrMkApp()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
};

struct GInstrUnwind : GInstr
{
    GInstrUnwind()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
};

struct GInstrUpdate : GInstr
{
    GInstrUpdate(int offset)
        : offset(offset)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    int offset;
};

struct GInstrPack : GInstr
{
    GInstrPack(int tag, int n)
        : tag(tag), n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);

    int tag, n;
};

struct GInstrSplit : GInstr
{
    GInstrSplit()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
};

struct GInstrJump : GInstr
{
    GInstrJump()
        : branches(), tag_map()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    std::vector<std::vector<GInstrPtr>> branches;
    std::unordered_map<int, int> tag_map;
};

struct GInstrSlide : GInstr
{
    GInstrSlide(int n)
        : n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    int n;
};

struct GInstrBinOp : GInstr
{
    GInstrBinOp(GBinop binop)
        : binop(binop)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
    GBinop binop;
};

struct GInstrEval : GInstr
{
    GInstrEval()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
};

struct GInstrAlloc : GInstr
{
    GInstrAlloc(int n)
        : n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
    
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
    virtual void compile(std::string& output);
};

struct GInstrEnd : GInstr
{
    GInstrEnd()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
};

struct GInstrPrint : GInstr
{
    GInstrPrint()
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);
};

struct GInstrGlobStart : GInstr
{
    GInstrGlobStart(const std::string& name, int n)
        : name(name), n(n)
    {}

    virtual void dump(int level);
    virtual void compile(std::string& output);

    std::string name;
    int n;
};

#endif // GINSTR_HH_
