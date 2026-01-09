#pragma once
#include <stdio.h>

/* from cpp ref */
// Uses compiler specific extensions if possible.
#ifdef __GNUC__ // GCC, Clang, ICC
 
#define unreachable() (__builtin_unreachable())
 
#elifdef _MSC_VER // MSVC
 
#define unreachable() (__assume(false))
 
#else
// Even if no extension is used, undefined behavior is still raised by
// the empty function body and the noreturn attribute.
 
// The external definition of unreachable_impl must be emitted in a separated TU
// due to the rule for inline functions in C.
 
[[noreturn]] inline void unreachable_impl() {}
#define unreachable() (unreachable_impl())
 
#endif

/**
 * @brief Write to VA_ARGS to stderr and evaluates to RC
 */
#define printf_err(__rc, ...) \
    (fprintf(stderr, __VA_ARGS__), (__rc))

/**
 * @brief Print out of memory to stderr and evaluates to RC
 */
#define enomem(__rc) \
    (printf_err(__rc, "out of memory"))

