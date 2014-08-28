#pragma once
#include <arch/tool/atomic.h>
#include <kernel/tool/spinlock.h>
#include <kernel/tool/list.h>

namespace kernel
{
   namespace memory
   {
      enum zone_type:uint32_t 
      {
         zone_dma    = 0,
         zone_dma32  = 1,
         zone_normal = 2,
         zone_high   = 3,
         zone_max    = 4
      };

      enum alloc_flags:uint32_t
      {
         alloc_dma         = 0x0000,
         alloc_dma32       = 0x0001,
         alloc_normal      = 0x0002,
         alloc_high        = 0x0003,

         alloc_zone_mask   = 0x000f,

      //the flags below we don't use now
      //just ready for feature
         alloc_atomic      = 0x0000,
         alloc_wait        = 0x0010,
         alloc_io          = 0x0020,
         alloc_fs          = 0x0030,

         alloc_type_mask   = 0x00f0
      };

      enum page_flag:uint32_t
      {
         page_none     = 0x0000,
         page_reserved = 0x0001,
         page_data     = 0x0002,
         page_slab     = 0x0004,
         page_cache    = 0x0008
      };

      const size_t page_size = 4096;
      const size_t page_shift = 12;

      struct __page_private
      {
         kernel::tool::list_t list;
         uint64_t data;
         
         arch::tool::atomic_t<uint32_t> count;
         uint32_t flags;
         
         __page_private(void) : count(0) {}
      };

      class page_t : private __page_private
      {
public:
         page_t(void) = delete;
         ~page_t(void) = delete;

         uint32_t reference(void)
         {return this->count.add_and_get(1);}
         uint32_t dereference(void)
         {return this->count.add_and_get(-1);}
         
         void set_flags(uint32_t flags)
         {this->flags |= flags;}
         void clear_flags(uint32_t flags)
         {this->flags |= ~flags;}

         void *to_address(void);
         size_t to_physical_address(void);
         static page_t *from_address(void *addr);
         static page_t *from_physical_address(size_t addr);

         void free(size_t order = 0);
         static page_t *alloc(
            size_t order = 0,uint32_t flags = alloc_normal | alloc_fs);
      };

      static_assert(sizeof(__page_private) <= 40,"The size of __page_private is too big");
                     //the size of __page_private should not be more than 40
      static_assert(sizeof(page_t) == sizeof(__page_private),
            "The size of page_t and __page_private is different!");

      void init_buddy_system(void);
   }
}
