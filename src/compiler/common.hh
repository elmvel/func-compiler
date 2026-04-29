#ifndef COMMON_HH_
#define COMMON_HH_

#define TODOF()                                 \
    do {                                        \
        fmt::println("TODO: {}", __FUNCTION__); \
        exit(101);                              \
    } while (0)

#define UNUSED(x) ((void)(x))

#define TRY(dst, expr) dst = (expr); if (!(dst).is_ok()) return dst;

#define COMPILER_TERM() std::exit(EXIT_FAILURE)
#define COMPILER_ERROR_TERM(...)                \
    do {                                        \
        fmt::print(stderr, "error: ");          \
        fmt::println(stderr, __VA_ARGS__);      \
        COMPILER_TERM();                        \
    } while (0)
#define COMPILER_ERROR(...)                     \
    do {                                        \
        fmt::print(stderr, "error: ");          \
        fmt::println(stderr, __VA_ARGS__);      \
    } while (0)
#define COMPILER_NOTE(...)                      \
    do {                                        \
        fmt::print(stderr, "note: ");           \
        fmt::println(stderr, __VA_ARGS__);      \
    } while (0)
#define INTERNAL_ERROR(...)                     \
    do {                                        \
        fmt::print(stderr, "internal error: "); \
        fmt::println(stderr, __VA_ARGS__);      \
        COMPILER_TERM();                        \
    } while (0)

#include <variant>
#include <csignal>

#define BREAKPOINT std::raise(SIGINT);

#endif // COMMON_HH_
