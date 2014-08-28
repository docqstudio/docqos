#include <arch/init.h>
#include <kernel/init/bootinfo.h>
#include <kernel/memory/buddy.h>

namespace kernel
{
   namespace init
   {
      extern "C" void *__kernel_end;
      extern "C" bootinfo_receiver __bootinfo_start;
      extern "C" bootinfo_receiver __bootinfo_end;

      static void parse_bootinfo(bootinfo_t *info)
      {
         while(info->t != bootinfo_t::end)
         {
            bootinfo_receiver *receiver = &__bootinfo_start;
            while(receiver < &__bootinfo_end)
            { 
               //foreach receivers to find the receiver
               // that wants receive this boot information
               if(receiver->type == info->t)
                  (*receiver->receiver)(info);
               ++receiver;
            }

            info = (bootinfo_t *)((uint8_t *)info + info->size);
         }
      }

      extern "C" int kmain(void *bootloader)
      {
         __kernel_end = &__kernel_end + sizeof(__kernel_end);
              //set kernel end address

         {
            uint8_t bootinfo[200];
            arch::get_bootinfo(
               (bootinfo_t *)bootinfo,
               sizeof(bootinfo),bootloader);
            parse_bootinfo((bootinfo_t *)bootinfo);
         }   //get and parse bootinfo
         arch::init();

         kernel::memory::init_buddy_system();

         return 0;
      }
   }
}
