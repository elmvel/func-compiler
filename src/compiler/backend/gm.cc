#include <iostream>
#include <vector>
#include <map>

using namespace LE;

using GMSCoMap = std::map<int, _int::SCo>;

int compiled_program(GMSCoMap* globals);
void register_supercombinators(GMSCoMap* globals);

int main()
{
    GMSCoMap* globals = &_int::function_library;

    register_supercombinators(globals);
    int result = compiled_program(globals);
    std::cout << "GM Output: " << result << "\n";
    return EXIT_SUCCESS;
}
