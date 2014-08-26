#include <arch/init.h>
#include <kernel/init/bootinfo.h>

namespace kernel
{
   namespace init
   {
      extern "C" void *__kernel_end;
      extern "C" bootinfo_receiver __bootinfo_start;
      extern "C" bootinfo_receiver __bootinfo_end;

      static void parse_bootinfo(
         kernel::init::bootinfo *info)
      {
         while(info->t != kernel::init::bootinfo::end)
         {
            bootinfo_receiver *receiver = &__bootinfo_start;
            while(receiver < &__bootinfo_end)
            { 
               //foreach receivers to find the receiver
               // that wants receive this boot information
               if(receiver->type == info->t)
               {
                  (*receiver->receiver)(info);
                  break;
               }
               ++receiver;
            }

            info = (bootinfo *)((uint8_t *)info + info->size);
         }
      }

      extern "C" int kmain(void *bootloader)
      {
         __kernel_end = &__kernel_end + sizeof(__kernel_end);
              //set kernel end address
         {
            uint8_t bootinfo[200];
            arch::get_bootinfo(
               (kernel::init::bootinfo *)bootinfo,
               sizeof(bootinfo),bootloader);
            parse_bootinfo((kernel::init::bootinfo *)bootinfo);
         }   //get and parse bootinfo
         arch::init();
         return 0;
      }
   }
}
