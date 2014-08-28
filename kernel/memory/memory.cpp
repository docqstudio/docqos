#include <kernel/init/bootinfo.h>

namespace kernel
{
   namespace memory
   {
      using ::kernel::init::bootinfo_t;
      using ::kernel::init::bootinfo_memory_map;

      static size_t memory_size = 0;

      static void calculate_memory_size(bootinfo_t *bootinfo)
      {
         size_t count = (bootinfo->size - sizeof(*bootinfo)) / 
                  sizeof(bootinfo_memory_map);
         bootinfo_memory_map *map = (decltype(map)) bootinfo->data;
         
         for(size_t i = 0;i < count;++i)
            if(map[i].type == bootinfo_memory_map::normal)
               memory_size = map[i].end;
      }

      size_t get_memory_size(void)
      {
         return memory_size;
      }

      register_bootinfo_receiver(bootinfo_t::memory_map,
               calculate_memory_size);
   }
}
