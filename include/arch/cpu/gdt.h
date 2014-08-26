#pragma once
#include <arch/cpu/type.h>

namespace arch
{
   namespace cpu
   {
      enum selector : uint16_t
      {
         selector_kernel_code    = 0x08,
         selector_user_code      = 0x13,
         selector_kernel_data    = 0x18,
         selector_user_data      = 0x23
      };

      void init_gdt(void);
   }
}
