#include <fmt/core.h>
#include <fmt/std.h>

#include "./gcode_to_c.hh"

#include "../file_reading.hh"
#include "../common.hh"

#include <unistd.h>

void compile_prelude(std::string& code)
{
    auto file_opt = read_file("./src/compiler/backend/gm.c");
    if (!file_opt.has_value()) {
        INTERNAL_ERROR("Could not find gm.c");
    }
    code.append(file_opt.value());
}

void GCodeToCCompiler::compile(const std::vector<GInstrPtr>& output)
{
    compile_prelude(code);
    for (int i = 0; i < (int)output.size(); ++i) {
        compile_instr(output[i]);
    }

    fmt::println("C Code:\n{}", code);
    
    INTERNAL_ERROR("Sanity check");
}

void GCodeToCCompiler::compile_instr(const GInstrPtr& instr)
{
    instr->compile(code);
}
