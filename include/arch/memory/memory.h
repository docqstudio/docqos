#pragma once
#include <kernel/memory/buddy.h>

namespace arch
{
   namespace memory
   {
      inline kernel::memory::zone_type get_address_zone(
                        size_t address)
      {
         using kernel::memory::zone_type; 
         if(address < 16ul * 1024 * 1024)
            return zone_type::zone_dma;
         else if(address < 4ul * 1024 * 1024 * 1024)
            return zone_type::zone_dma32;
         else
            return zone_type::zone_normal;
      }
   }
}
