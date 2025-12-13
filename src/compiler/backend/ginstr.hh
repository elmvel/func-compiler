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
};

using GInstrPtr = std::unique_ptr<GInstr>;

struct GInstrPushInt : GInstr
{
    GInstrPushInt(int value)
        : value(value)
    {}

    virtual void dump(int level);
    
    int value;
};

struct GInstrPushGlobal : GInstr
{
    GInstrPushGlobal(const std::string& name)
        : name(name)
    {}

    virtual void dump(int level);
    
    std::string name;
};

struct GInstrPush : GInstr
{
    GInstrPush(int offset)
        : offset(offset)
    {}

    virtual void dump(int level);
    
    int offset;
};

struct GInstrPop : GInstr
{
    GInstrPop(int n)
        : n(n)
    {}

    virtual void dump(int level);
    
    int n;
};


struct GInstrMkApp : GInstr
{
    GInstrMkApp()
    {}

    virtual void dump(int level);
};

struct GInstrUnwind : GInstr
{
    GInstrUnwind()
    {}

    virtual void dump(int level);
};

struct GInstrUpdate : GInstr
{
    GInstrUpdate(int offset)
        : offset(offset)
    {}

    virtual void dump(int level);
    
    int offset;
};

struct GInstrPack : GInstr
{
    GInstrPack(int tag, int n)
        : tag(tag), n(n)
    {}

    virtual void dump(int level);

    int tag, n;
};

struct GInstrSplit : GInstr
{
    GInstrSplit()
    {}

    virtual void dump(int level);
};

struct GInstrJump : GInstr
{
    GInstrJump()
        : branches(), tag_map()
    {}

    virtual void dump(int level);
    
    std::vector<std::vector<GInstrPtr>> branches;
    std::unordered_map<int, int> tag_map;
};

struct GInstrSlide : GInstr
{
    GInstrSlide(int n)
        : n(n)
    {}

    virtual void dump(int level);
    
    int n;
};

struct GInstrBinOp : GInstr
{
    GInstrBinOp(GBinop binop)
        : binop(binop)
    {}

    virtual void dump(int level);
    
    GBinop binop;
};

struct GInstrEval : GInstr
{
    GInstrEval()
    {}

    virtual void dump(int level);
};

struct GInstrAlloc : GInstr
{
    GInstrAlloc(int n)
        : n(n)
    {}

    virtual void dump(int level);
    
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
};

struct GInstrEnd : GInstr
{
    GInstrEnd()
    {}

    virtual void dump(int level);
};

struct GInstrPrint : GInstr
{
    GInstrPrint()
    {}

    virtual void dump(int level);
};

struct GInstrGlobStart : GInstr
{
    GInstrGlobStart(const std::string& name, int n)
        : name(name), n(n)
    {}

    virtual void dump(int level);

    std::string name;
    int n;
};

#endif // GINSTR_HH_
