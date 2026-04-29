#ifndef SC_TO_GCODE_HH_
#define SC_TO_GCODE_HH_

#include <vector>
#include <unordered_map>

#include "./ginstr.hh"
#include "./lambda_lifting.hh"

struct GCodeEnv
{
    GCodeEnv(int depth)
        : offset_map(), depth(depth)
    {}
    
    std::unordered_map<std::string, int> offset_map;
    int depth;
};

struct SCToGCodeCompiler
{
    void compile(LiftedProgram& lp);
    void compile_supercombinator(const std::string& sc_name, LCNodePtr lcbody);
    void compile_supercombinator_body(GCodeEnv& env, LCNodePtr body);

    std::vector<GInstrPtr> output;
};

void compile_construct_instance(std::vector<GInstrPtr> *output, const GCodeEnv *env, LCNodePtr body);

struct LCConstructInstanceVisitor : ILCVisitor
{
    LCConstructInstanceVisitor(std::vector<GInstrPtr> *output, const GCodeEnv *env)
        : output(output), env(env)
    {}
    
    virtual void visit(LCApplyNode *node);
    virtual void visit(LCLambdaNode *node);
    virtual void visit(LCDefNode *node);
    virtual void visit(LCLetNode *node);
    virtual void visit(LCCaseNode *node);
    virtual void visit(LCCaseArmNode *node);
    virtual void visit(LCIntNode *node);
    virtual void visit(LCBoolNode *node);
    virtual void visit(LCConstantNode *node);
    virtual void visit(LCDummyNode *node);

    std::vector<GInstrPtr> *output;
    const GCodeEnv *env;
};

#endif // SC_TO_GCODE_HH_
