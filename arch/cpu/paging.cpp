#include <arch/cpu/paging.h>
#include <kernel/init/bootinfo.h>

namespace arch
{
   namespace cpu
   {
      using kernel::init::bootinfo;
      using kernel::init::bootinfo_memory_map;

      extern "C" void *__kernel_end;
      static size_t memory_size = 4ul * 1024 * 1024 * 1024;
      
      static void receive_memory_map(bootinfo *info)
      { //FIXME:in fact,this function should NOT write here
        //it should write into /kernel/memory/memory.cpp
         bootinfo_memory_map *map = (decltype(map))info->data;
         size_t count = (info->size - sizeof(*info)) / sizeof(*map);
         
         for(unsigned int i = 0;i < count;++i)
            if(map[i].type == bootinfo_memory_map::normal)
               memory_size = map[i].end; //get memory size from memory map
      }

      void init_paging(void)
      {
         //size_t mapping_size = kernel::memory::get_memory_size();
         size_t mapping_size = memory_size;
         //calculate the number of pages
         size_t pmd_count = mapping_size / (1024 * 1024 * 2);
         if(mapping_size % (1024 * 1024 * 2) != 0)
            ++pmd_count;

         size_t pud_count = pmd_count / 512;
         if(pmd_count % 512 != 0)
            ++pud_count;

         size_t pgd_count = pud_count / 512;
         if(pud_count % 512 != 0)
            ++pgd_count;

         ++pgd_count;

         size_t real_pmd = (pmd_count + 511) & ~511;
         size_t real_pud = (pud_count + 511) & ~511;
         size_t real_pgd = (pgd_count + 511) & ~511;
                     //the real number of pages

         //init pmd
         uint64_t *kernel_pmd = 
            (uint64_t *)((((size_t)__kernel_end) + 4095) & ~4095);
         for(size_t i = 0;i < pmd_count;++i)
            kernel_pmd[i] = 2ul * 1024 * 1024 * i + 0x183;
         for(size_t i = pmd_count;i < real_pmd;++i)
            kernel_pmd[i] = 0;

         //init pud
         uint64_t *kernel_pud = kernel_pmd + real_pmd;
         for(size_t i = 0;i < pud_count;++i)
            kernel_pud[i] = va2pa(&kernel_pmd[i * 512]) + 0x003;
         for(size_t i = pud_count;i < real_pud;++i)
            kernel_pud[i] = 0;

         //init pgd
         uint64_t *kernel_pgd = kernel_pud + real_pud;
         for(size_t i = 0;i < 1;++i)
            kernel_pgd[i] = 0;
         for(size_t i = 1;i < pgd_count;++i)
            kernel_pgd[i] = va2pa(&kernel_pud[(i - 1) * 512]) + 0x003;
         for(size_t i = pgd_count;i < real_pgd;++i)
            kernel_pgd[i] = 0;

         __kernel_end = &kernel_pgd[real_pgd];
         //update __kernel_end

         load_pgd((pgd_t *)kernel_pgd); //load pages
      }

      register_bootinfo_receiver(bootinfo::memory_map,receive_memory_map);
   }
}
