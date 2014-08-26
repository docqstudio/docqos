#include <arch/cpu/type.h>
#include <arch/cpu/gdt.h>

namespace arch
{
   namespace cpu
   {
      struct normal_descriptor
      {
         uint16_t limit1;
         uint16_t address1;

         uint8_t address2;
         uint8_t type1;

         uint8_t limit2:4;
         uint8_t type2:4;
         uint8_t address3;
      } struct_packed;

      struct system_descriptor
      {
         uint16_t limit1;
         uint16_t address1;

         uint8_t address2;
         uint8_t type1;

         uint8_t limit2:4;
         uint8_t type2:4;
         uint8_t address3;

         uint32_t address4;
         uint32_t reserved;
      } struct_packed;

      struct task_state_segment
      {
         uint32_t reserved1;
         uint64_t rsp0;
         uint64_t rsp1;
         uint64_t rsp2;

         uint64_t reserved2;

         uint64_t ist1;
         uint64_t ist2;
         uint64_t ist3;
         uint64_t ist4;
         uint64_t ist5;
         uint64_t ist6;
         uint64_t ist7;

         uint64_t reserved3;
         uint16_t reserved4;

         uint16_t io_map;
      } struct_packed;

      struct gdtr
      {
         uint16_t size;
         void *address;
      } struct_packed;

      const size_t gdt_size = 11 * sizeof(normal_descriptor);

      enum descriptor_type:uint16_t
      {
         present  = 0x0080,
   
         code     = 0x0018,
         code64   = 0x2000,
   
         data     = 0x0010,
         data_w   = 0x0002,

         tss      = 0x0009,

         dpl0     = 0x0000,
         dpl1     = 0x0020,
         dpl2     = 0x0040,
         dpl3     = 0x0060
      };  

      static uint8_t gdt[gdt_size];
      static gdtr gdtr0;
      
//      static task_switch_segment tss0;

      static void add_normal_descriptor(size_t &index,
                        uint16_t t)
      {
         normal_descriptor *desc = (normal_descriptor *)&gdt[index];
         desc->type1 = t & 0x00ff;
         desc->type2 = (t & 0xf000) >> 12;
         index += sizeof(*desc);
      }

      void init_gdt(void)
      {
         size_t index = 0;
         add_normal_descriptor(index,0x0000);
               //null descriptor
         add_normal_descriptor(index,code | code64 | present | dpl0);
               //kernel code
         add_normal_descriptor(index,code | code64 | present | dpl3);
               //user code
         add_normal_descriptor(index,data | data_w | present | dpl0);
               //kernel data
         add_normal_descriptor(index,data | data_w | present | dpl3);
               //user data

         gdtr0.address = gdt;
         gdtr0.size = index - 1;

         asm volatile
         (
            "lgdt (%%rbx)\n\t" //load gdt
            "movw %%ax,%%ds\n\t"
            "movw %%ax,%%gs\n\t"
            "movw %%ax,%%fs\n\t"
            "movw %%ax,%%es\n\t"
            "movw %%ax,%%ss\n\t" //refresh segment registers
            "movq %%rsp,%%rcx\n\t"
            "movabs $1f,%%rdx\n\t"
            "pushq %%rax\n\t"
            "pushq %%rcx\n\t"
            "pushfq\n\t"
            "pushq %%rdi\n\t"
            "pushq %%rdx\n\t"
            "iretq\n\t"
            "1:\n\t"
            :
            : "a" ((uint64_t) selector_kernel_data),
              "D" ((uint64_t) selector_kernel_code),
              "b" ((size_t) &gdtr0)
            : "%rdx","%rcx"
         );
      }
   }
}
