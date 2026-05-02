#ifndef GCODE_TO_CXX_HH_
#define GCODE_TO_CXX_HH_

#include <vector>

#include "./ginstr.hh"

struct GCodeToCXXCompiler
{
    void compile(const std::vector<GInstrPtr>& output, bool dump_gmachine);
    void compile_instr(const GInstrPtr& instr, CompilerSCoMap& scomap);

    std::string code;
};

#endif // GCODE_TO_CXX_HH_
