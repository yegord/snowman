#pragma once

#if __cpp_constexpr >= 200704
  #define CONSTEXPR constexpr
#else
  #define CONSTEXPR static inline
#endif
