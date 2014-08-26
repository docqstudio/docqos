#pragma once
#include <arch/cpu/type.h>

namespace arch
{
   namespace cpu
   {
      //pgd => pml4e
      //pud => pdpte
      //pmd => pde
      //pte => pte

      const size_t page_offset = 0x8000000000;

      typedef struct {uint64_t val;} pgd_t;
      typedef struct {uint64_t val;} pud_t;
      typedef struct {uint64_t val;} pmd_t;
      typedef struct {uint64_t val;} pte_t;

      enum page_type
      {
         page_present = 0x001,
         page_write   = 0x002,
         page_user    = 0x004,
         page_global  = 0x100
      };

      inline size_t va2pa(void *address)
      { //convert virtual address(in fact liner address) to physical address
         return (size_t)address - page_offset;
      }
      inline void *pa2va(size_t address)
      { //convert physical address to virtual address
         return (void *)(address + page_offset);
      }

      inline void set_pgd(pgd_t *pgd,pud_t *pud,page_type type,size_t index)
      {
         pgd_t entry{va2pa(pud) + type};
         pgd[(index >> 39) & 0x1ff] = entry;
      }
      inline void set_pud(pud_t *pud,pmd_t *pmd,page_type type,size_t index)
      {
         pud_t entry{va2pa(pmd) + type};
         pud[(index >> 30) & 0x1ff] = entry;
      }
      inline void set_pmd(pmd_t *pmd,pte_t *pte,page_type type,size_t index)
      {
         pmd_t entry{va2pa(pte) + type};
         pmd[(index >> 21) & 0x1ff] = entry;
      }
      inline void set_pte(pte_t *pte,size_t addr,page_type type,size_t index)
      {
         pte_t entry{addr + type};
         pte[(index >> 12) & 0x1ff] = entry;
      } //these functions are interface from arch to kernel

      inline void refresh_tlb(void)
      {
         asm volatile
         (
            "movq %%cr3,%%rax\n\t"
            "movq %%rax,%%cr3\n\t"
            :::"%rax","memory"
         );
      }

      inline void load_pgd(pgd_t *pgd)
      {
         asm volatile
         (
            "movq %%rax,%%cr3\n\t"
            ::"a"(va2pa(pgd)):"memory"
         ); //load pages
      }

      void init_paging(void);
   }
}
