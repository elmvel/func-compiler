#ifndef COMMON_H_
#define COMMON_H_

#define TODOF()                                 \
    do {                                        \
        fmt::println("TODO: {}", __FUNCTION__); \
        exit(101);                              \
    } while (0)

#endif // COMMON_H_
