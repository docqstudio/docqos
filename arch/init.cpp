#include <arch/init.h>
#include <arch/cpu/gdt.h>
#include <arch/cpu/paging.h>
#include <kernel/compiler.h>
#include <kernel/init/bootinfo.h>

namespace arch
{
   using kernel::init::bootinfo_t;
   using kernel::init::bootinfo_memory_map;
   const uint32_t multiboot_magic = 0x36d76289;

      //some multiboot2 structs
   struct multiboot_tag
   {
      uint32_t type;
      uint32_t size;
      enum mtype
      {
         end = 0,
         memory_map = 6,
         framebuffer = 8
      };
   } struct_packed;
   
   struct multiboot_color
   {
      uint8_t red;
      uint8_t green;
      uint8_t blue;
   } struct_packed;

   struct multiboot_framebuffer
   {
      multiboot_tag tag;

      uint64_t address;
      uint32_t pitch;
      uint32_t width;
      uint8_t bpp;
      uint8_t type;
      uint16_t reserved;

      union
      {
         struct 
         {
            uint8_t red_field_position;
            uint8_t red_mask_size;
            uint8_t green_field_position;
            uint8_t green_mask_size;
            uint8_t blue_field_position;
            uint8_t blue_mask_size;
         } struct_packed;
         struct
         {
            uint16_t count;
            multiboot_color palette[0];
         } struct_packed;
      };
   } struct_packed;

   struct multiboot_memory_map_entry
   {
      uint64_t address;
      uint64_t length;
      uint32_t type;
      uint32_t reserved0;

      enum mtype
      {
         normal = 1,
         acpi = 3,
         reserved = 4
      };
   } struct_packed;

   struct multiboot_memory_map
   {
      multiboot_tag tag;
      
      uint32_t count;
      uint32_t version;

      multiboot_memory_map_entry entries[0];
   } struct_packed;

   static int copy_memory_map(
            multiboot_memory_map *map0,bootinfo_t *map1,
            size_t &size)
   {
      map0->count = (map0->tag.size - sizeof(*map0)) / map0->count;
      size_t size0 = sizeof(bootinfo_t) * 2 + 
            map0->count * sizeof(bootinfo_memory_map);
      if(size < size0)
         return -1; //no enough space to save memory map
      size -= size0;

      map1->t = bootinfo_t::memory_map;
      map1->size = size0 - sizeof(bootinfo_t);

      bootinfo_memory_map *map = 
               (bootinfo_memory_map *)map1->data;
      multiboot_memory_map_entry *entry =
               map0->entries;

      for(unsigned int i = 0;i < map0->count;++i,++map,++entry)
      { //copying
         map->start = entry->address;
         map->end = entry->address + entry->length;
         if(entry->type == multiboot_memory_map_entry::normal)
            map->type = bootinfo_memory_map::normal;
         else
            map->type = bootinfo_memory_map::reserved;
      }
      return 0;
   }

   int get_bootinfo(bootinfo_t info[],size_t size,
                  void *stack0)
   {
      struct {uint64_t magic;multiboot_tag *tags;} struct_packed
                      *stack;
      stack = (decltype(stack))stack0; //see also /arch/boot/boot.s
      
      if(stack->magic != multiboot_magic)
         return -1; //check magic number
      multiboot_tag *tags = stack->tags;
      
      while(tags->type != multiboot_tag::end)
      {
         switch(tags->type)
         {
         case multiboot_tag::memory_map:
            if(copy_memory_map(
               (multiboot_memory_map *)tags,info,size))
               goto end; //copy memory map from multiboot tag to boot information
            info = (bootinfo_t *)((uint8_t *)info + info->size);
            break;
         default:
            break;
         }
         tags = 
            (multiboot_tag *)((uint8_t *)tags + ((tags->size + 7) & ~7));
               //next tag
      }
end:
      info->t = bootinfo_t::end; 
      return 0;
   }

   int init(void)
   {
      cpu::init_gdt();
      cpu::init_paging();
      return 0;
   }
}
