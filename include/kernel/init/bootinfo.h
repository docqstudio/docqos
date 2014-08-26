#pragma once
#include <arch/cpu/type.h>

namespace kernel
{
   namespace init
   {
      struct bootinfo
      {
         enum type:uint32_t
         {
            memory_map,
            memory_size,
            framebuffer,
            end
         };
         type t; //type of this information
         size_t size; //size of this information
         uint8_t data[0]; //saves data for this informaton
      };
      struct bootinfo_receiver
      {
         bootinfo::type type;
         void (*receiver)(bootinfo *);
      };

      struct bootinfo_memory_map
      {
         size_t start;
         size_t end;
         enum :uint64_t {normal,reserved} type;
      };

      //in future,there will be struct bootinfo_framebuffer etc

#define register_bootinfo_receiver(type,receiver) \
   __attribute__((section(".bootinfo"))) \
   static volatile kernel::init::bootinfo_receiver \
               __bir_##receiver{type,receiver};
         //saves the receiver to the section ".bootinfo"
   }
}
