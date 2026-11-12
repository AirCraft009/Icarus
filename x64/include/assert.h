//
// Created by cocon on 01.11.2026.
//

#ifndef ICARUS_ASSERT_H
#define ICARUS_ASSERT_H

__attribute__((noreturn))
void kernel_assert_failed(const char *expression, const char *file, int line);

#ifdef NDEBUG
#define assert(condition) ((void)0)
#else
#define assert(condition) \
((condition) ? (void)0 : kernel_assert_failed(#condition, __FILE__, __LINE__))
#endif

/* In C23 static_assert is a keyword, so only define it for older standards. */
#if !defined(__cplusplus) && (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L)
#define static_assert _Static_assert
#endif

#endif //ICARUS_ASSERT_H
