#ifndef GCODE_TO_C_HH_
#define GCODE_TO_C_HH_

#include <vector>

#include "./ginstr.hh"

struct GCodeToCCompiler
{
    void compile(const std::vector<GInstrPtr>& output);
    void compile_instr(const GInstrPtr& instr);

    std::string code;
};

#endif // GCODE_TO_C_HH_
