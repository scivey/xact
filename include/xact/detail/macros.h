#pragma once

#ifdef __builtin_expect
  #define XACT_LIKELY(x) __builtin_expect(x, 1)
  #define XACT_UNLIKELY(x) __builtin_expect(x, 0)
#else
  #define XACT_LIKELY(x) x 
  #define XACT_UNLIKELY(x) x
#endif

#define XACT_CHECK(expr) do { \
    auto _xact_result__ = (expr); \
    if (XACT_UNLIKELY(!_xact_result__)) { \
      throw std::runtime_error("check failed: '" #expr "'!"); \
    } \
  } while (0)



#ifndef NDEBUG
  #define XACT_DCHECK(expr) XACT_CHECK(expr)
#else
  #define XACT_DCHECK(expr) ((void) (expr))
#endif


#define XACT_RW_BARRIER() asm volatile("" ::: "memory")
#define XACT_MFENCE_BARRIER() asm volatile("mfence" ::: "memory")
