#include "sema.hh"

#include <fmt/core.h>
#include <fmt/std.h>

auto fmt::formatter<Type>::format(Type c, fmt::v10::format_context& ctx) const
    -> format_context::iterator {
  string_view name = "<unknown>";
  switch (c) {
  case Type::Integer:  name="Integer";  break;
  case Type::String:   name="String";   break;
  }
  return formatter<std::string_view>::format(name, ctx);
}
