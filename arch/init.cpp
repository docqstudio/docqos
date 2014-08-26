#include <arch/init.h>
#include <arch/cpu/gdt.h>

namespace arch
{
   int init(void)
   {
      cpu::init_gdt();
      return 0;
   }
}
