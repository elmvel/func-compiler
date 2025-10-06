#ifndef TYPES_HH_
#define TYPES_HH_

#include <cassert>
#include <memory>
#include <variant>

// helper type for the visitor
template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };

// explicit deduction guide (not needed as of C++20)
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

struct Type;

enum class TypePrimitive
{
    Integer,
    String,
};

struct TypeFunction
{
    TypeFunction(Type *input, Type *output)
        : input(input), output(output)
    {
        assert(input != nullptr);
        assert(output != nullptr);
    }
    
    Type *input;
    Type *output;
};

struct Type
{
    Type(TypePrimitive kind)
        : storage(kind)
    {}

    Type(TypeFunction fn)
        : storage(fn)
    {}

    std::optional<Type *> get_input()
    {
        const TypeFunction *func = std::get_if<TypeFunction>(&storage);
        if (func == nullptr) return {};

        return func->input;
    }

    std::optional<Type *> get_output()
    {
        const TypeFunction *func = std::get_if<TypeFunction>(&storage);
        if (func == nullptr) return {};

        return func->output;
    }

    // Not the prettiest...
    bool operator==(const Type& other) const
    {
        const TypePrimitive *this_prim = std::get_if<TypePrimitive>(&storage);
        const TypePrimitive *other_prim = std::get_if<TypePrimitive>(&other.storage);
        if (this_prim != nullptr && other_prim != nullptr) {
            return *this_prim == *other_prim;
        } else {
            const TypeFunction *this_func = std::get_if<TypeFunction>(&storage);
            const TypeFunction *other_func = std::get_if<TypeFunction>(&other.storage);
            if (this_func != nullptr && other_func != nullptr) {
                bool a = *this_func->input == *other_func->input;
                bool b = *this_func->output == *other_func->output;
                return a && b;
            } else {
                return false;
            }
        }
    }

    bool operator!=(const Type& other) const
    {
        return !(*this == other);
    }

    // TODO: fix allocation strategy with types
    // ~Type()
    // {
    //     std::visit(overloaded {
    //         [](TypePrimitive prim)
    //         {
    //             (void)prim;
    //         },
    //         [](TypeFunction func)
    //         {
    //             delete func.input;
    //             delete func.output;
    //         },
    //     }, storage);
    // }
    
    std::variant<TypePrimitive, TypeFunction> storage;
};

template <> struct fmt::formatter<Type>: formatter<std::string> {
  auto format(Type c, format_context& ctx) const
    -> format_context::iterator;
};

struct TreeParamsNode;

std::pair<Type *, size_t> make_function_type(TreeParamsNode *node, Type *ret_type);

#endif // TYPES_HH_
