#pragma once
#include <arch/cpu/type.h>

inline void *operator new(size_t,void *p)
{
   return p;
}

inline void *operator new[](size_t,void *p)
{
   return p;
}
