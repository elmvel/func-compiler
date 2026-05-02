#include <fmt/core.h>
#include <fmt/std.h>

#include "./gcode_to_cxx.hh"

#include "../file_reading.hh"
#include "../common.hh"

#include <unistd.h>

int CompilerSCoMap::fetch_or_insert(const std::string& name)
{
    auto it = container.find(name);
    if (it != container.end()) {
        // Name already exists
        return it->second;
    } else {
        // Name does not already exist
        int id = counter++;
        container[name] = id;
        return id;
    }
}

void compile_prelude(std::string& code)
{
    auto file_le_opt = read_file("./src/compiler/vendor/leval.hpp");
    if (!file_le_opt.has_value()) {
        INTERNAL_ERROR("Could not find leval.hpp");
    }
    code.append(file_le_opt.value());

    auto file_gm_opt = read_file("./src/compiler/backend/gm.cc");
    if (!file_gm_opt.has_value()) {
        INTERNAL_ERROR("Could not find gm.cc");
    }
    code.append(file_gm_opt.value());
}

void GCodeToCXXCompiler::compile(const std::vector<GInstrPtr>& output, bool dump_gmachine)
{
    CompilerSCoMap scomap {};
    
    compile_prelude(code);
    
    for (int i = 0; i < (int)output.size(); ++i) {
        compile_instr(output[i], scomap);
    }

    code.append("}\n");

    if (dump_gmachine) {
        fmt::println("========================================");
        fmt::println("             G-Machine Impl.            ");
        fmt::println("========================================");
        fmt::println("{}", code);
    }
}

void GCodeToCXXCompiler::compile_instr(const GInstrPtr& instr, CompilerSCoMap& scomap)
{
    instr->compile(code, scomap);
}
