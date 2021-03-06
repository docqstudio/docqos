#pragma once
#include <arch/cpu/type.h>

namespace kernel
{
   namespace init
   {
      struct bootinfo_t;
   }
}

namespace arch
{
   int get_bootinfo(kernel::init::bootinfo_t info[]
                  ,size_t size,void *stack0);
   int init(void);
}
