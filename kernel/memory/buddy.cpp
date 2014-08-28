#include <arch/cpu/paging.h>
#include <arch/memory/memory.h>
#include <kernel/compiler.h>
#include <kernel/init/bootinfo.h>
#include <kernel/memory/memory.h>
#include <kernel/memory/buddy.h>
#include <kernel/memory/new.h>

namespace kernel
{
   namespace memory
   {
      using kernel::init::bootinfo_t;
      using kernel::init::bootinfo_memory_map;
      
      const unsigned int max_order = 11;
      struct zone_t
      { //the struct of memory zone
         size_t page_count = 0; //total page count of this zone
         size_t free_page_count = 0; //free page count of this zone

         kernel::tool::spinlock_t lock; //spinlock of this zone
         kernel::tool::list_t free_list[max_order];
                           //free list of this zone
      };


      extern "C" void *__kernel_end;

      static bootinfo_memory_map memory_map[5];
      static size_t memory_map_count = 0;
      static __page_private *page_map;
      static size_t page_count;

      static zone_t memory_zone[zone_max];

      static void receive_memory_map(bootinfo_t *bootinfo)
      { //get the boot information
         bootinfo_memory_map *map = (decltype(map))bootinfo->data;
         size_t count = (bootinfo->size - sizeof(*bootinfo)) / sizeof(map);
         
         for(size_t i = 0;i < count;++i)
         {
            if(map[i].type == bootinfo_memory_map::normal)
            {
               memory_map[memory_map_count++] = map[i];
               if(memory_map_count >= 
                     sizeof(memory_map)/sizeof(memory_map[0]))
                  break;
            } //saves normal memory area to memory_map
         }
      }

      static bool page_is_buddy(__page_private *page,size_t order)
      {
         if(page->count.get() == 0 && //nobody references the page
            (page->flags & page_data) &&
            !(page->flags & page_reserved) &&
            page->data == order)
            return true;
         return false;
      }

      static __page_private *alloc_pages(
                     size_t order = 0,
                     uint32_t flags = alloc_normal | alloc_fs)
      {
         size_t type = flags & alloc_zone_mask;
         zone_t *zone = &memory_zone[type];
         size_t size,current_order;
         __page_private *page,*buddy;

         zone->lock.lock();
         while(!zone->free_page_count && type)
         {
            zone->lock.unlock();
            zone = &memory_zone[--type];
            zone->lock.lock();
         }
         if(unlikely(!zone->free_page_count))
         {
            zone->lock.unlock();
            return 0;
         }

         for(current_order = order;
               current_order < max_order;++current_order)
         {
            if(zone->free_list[current_order].empty())
               continue;
            page = zone->free_list[current_order].value<
                  __page_private *,offset_of(__page_private,list)>();
            page->list.remove(); //get the page

            zone->free_page_count -= 1 << current_order;
            zone->lock.unlock();

            page->count.add(1);
            page->flags &= ~page_data;

            size = 1ul << current_order;
            while(current_order > order)
            {
               --current_order;
               size >>= 1;
               buddy = page + size;
               buddy->flags |= page_data;
               buddy->data = current_order;
               
               zone->lock.lock();
               zone->free_page_count += 1 << current_order;
               buddy->list.insert_before(&zone->free_list[current_order]);
                  //insert this buddy to the free list
               zone->lock.unlock();
            }
            return page;
         }
         zone->lock.unlock();

         //it should never arrive here...
         //panic();
         return 0;
      }

      static void free_pages(__page_private *page,size_t order = 0)
      {
         size_t index = page - page_map;
         size_t buddy_index;         
         __page_private *buddy;

         if(index >= page_count)
            return;
         if(page->count.add_and_get(-1) != 0)
            return; //someone is still using this page....
         zone_t &zone = memory_zone[
               arch::memory::get_address_zone(index << page_shift)];
               //get the memory zone for this page
         zone.lock.lock();
         zone.free_page_count += 1 << order;

         page->flags = page_none;
         page->data = 0;
         while(order < max_order - 1)
         {
            //looking for buddies
            buddy_index = index ^ (1 << order);
            buddy = page_map + buddy_index;
            if(!page_is_buddy(buddy,order))
               break;
            //found one
            buddy->flags = page_none;
            buddy->data = 0;
            buddy->list.remove();
            index &= buddy_index;
            ++order;
         }
         page = page_map + index;
         page->data = order;
         page->flags = page_data;
         page->count.set(0);
         
         page->list.insert_before(&zone.free_list[order]);
         zone.lock.unlock();
      }
      
      void init_buddy_system(void)
      {
         page_count = get_memory_size() >> page_shift;

         page_map = (__page_private *)__kernel_end;
         __kernel_end = (uint8_t *)__kernel_end + 
                        page_count * sizeof(*page_map);

         for(size_t i = 0;i < page_count;++i)
         {
            __page_private &pg = page_map[i];
            pg.flags = page_none;
            pg.data = 0;
            new (&pg.count) decltype(pg.count)(1);
         } //init the global values

         new (memory_zone) decltype(memory_zone)();

         size_t kernel_end = arch::cpu::va2pa(__kernel_end);
         for(size_t i = 0;i < memory_map_count;++i)
         {
            size_t start = memory_map[i].start;
            size_t end = memory_map[i].end;
            if(end <= kernel_end)
               continue;
            if(start < kernel_end)
               start = kernel_end;
            size_t start_index = start >> page_shift;
            if(start & (page_size - 1))
               ++start_index;
            size_t end_index = end >> page_shift;
            for(size_t j = start_index;j < end_index;++j)
               free_pages(page_map + j,0);
         } //free all pages we can use
         for(size_t i = 0;i < zone_max;++i)
            memory_zone[i].page_count = memory_zone[i].free_page_count;

         //just do a test
         page_t *pg = page_t::alloc();
         pg->free();
      }

      void *page_t::to_address(void)
      {
         __page_private *pg = this;
         size_t addr = pg - page_map;
         return arch::cpu::pa2va(addr << page_shift);
      }

      size_t page_t::to_physical_address(void)
      {
         __page_private *pg = this;
         return (pg - page_map) << page_shift;
      }

      page_t *page_t::from_address(void *addr)
      {
         size_t address = arch::cpu::va2pa(addr);
         return (page_t *)(page_map + (address >> page_shift));
      }

      page_t *page_t::from_physical_address(size_t addr)
      {
         return (page_t *)(page_map + (addr >> page_shift));
      }

      page_t *page_t::alloc(size_t order,uint32_t flags)
      {
         return (page_t *)alloc_pages(order,flags);
      } //alloc pages

      void page_t::free(size_t order)
      {
         return free_pages((__page_private *)this,order);
      } //free pages


      register_bootinfo_receiver(bootinfo_t::memory_map,
            receive_memory_map);
   }
}
