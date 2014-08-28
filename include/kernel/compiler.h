#pragma once

#define likely(x) __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x),0)

#define offset_of(type,member) \
   ((size_t)&((type *)nullptr)->member)

#define struct_packed __attribute__ ((packed))
