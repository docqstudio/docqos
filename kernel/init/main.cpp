#include <arch/init.h>

namespace kernel
{
   namespace init
   {
      extern "C" int kmain(void)
      {
         arch::init();
         return 0;
      }
   }
}
