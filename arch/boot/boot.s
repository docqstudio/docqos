.section ".boot","ax"
.code32
.global _start32
.extern __multiboot_load_end
.extern __multiboot_all_end 
      #defined in kernel.ldscript

.set PAGE_OFFSET              ,0x8000000000
.set BOOT_PAGE_PML4E_ADDRESS  ,0x60000
.set BOOT_PAGE_PDPTE_ADDRESS  ,0x62000
.set BOOT_PAGE_PDE_ADDRESS    ,0x64000

9:
.long 0xe85250d6                 #multiboot2 magic number
.long 0                          #arch:i386
.long (8f - 9b)                  #multiboot header length
.long -(0xe85250d6 +0 + 8f - 9b) #check sum

.word 2,0                  #address tag
.long 24                   #tag size
.long 9b                   #header address
.long 8f                   #load address
.long __multiboot_load_end #load end address
.long __multiboot_all_end  #bss end address

.word 3,0         #entry address tag
.long 12          #tag size
.long _start32    #entry address
.long 0           #align

.word 5,0   #framebuffer tag
.long 20    #tag size
.long 1024  #width
.long 768   #height
.long 32    #depth
.long 0     #align

.word 0,0   #end tag
.long 8     #tag size

8:

boot_gdt32:
.quad 0x0000000000000000 #null
.quad 0x00cf9a000000ffff #code32
.quad 0x00cf92000000ffff #data32
boot_gdt64:
.quad 0x0000000000000000 #null
.quad 0x00a09a0000000000 #code64
.quad 0x00a0920000000000 #data64

boot_gdtr32:.word 23
            .long boot_gdt32
boot_gdtr64:.word 23
            .quad boot_gdt64
boot_gdtr64_paging:.word 23
                   .quad (boot_gdt64 + PAGE_OFFSET)

_start32:
   pushl $0
   pushl %eax        #pushq %rax
   pushl $0
   pushl %ebx        #pushq %rbx

   pushl $0x40000
   popfl
   pushfl
   orl $0x40000,(%esp)
   jz 3f             #check if we can use cpuid
   movl $0,(%esp)
   popfl

   movl $0x80000000,%eax
   cpuid
   cmpl $0x80000004,%eax
   jb 3f

   movl $0x80000001,%eax
   cpuid             #check if it supports x86_64
   btl $29,%edx
   jnc 3f

   lgdt (boot_gdtr32)
   ljmp $0x8,$1f

1:
   movw $0x10,%ax
   movw %ax,%es
   movw %ax,%gs
   movw %ax,%fs
   movw %ax,%ds
   movw %ax,%ss         #refresh segment registers

   movl $BOOT_PAGE_PDE_ADDRESS,%eax
   movl $(1024 * 1024 * 0 + 0x83),  (%eax)
   movl $(1024 * 1024 * 2 + 0x83), 8(%eax)
   movl $(1024 * 1024 * 4 + 0x83),16(%eax)
   movl $(1024 * 1024 * 6 + 0x83),24(%eax)
   movl $(1024 * 1024 * 8 + 0x83),32(%eax)

   movl $0, 4(%eax)
   movl $0,12(%eax)
   movl $0,20(%eax)
   movl $0,28(%eax)
   movl $0,36(%eax)

   orl $3,%eax
   movl $BOOT_PAGE_PDPTE_ADDRESS,%ebx
   movl %eax,(%ebx)
   movl $0,4(%ebx)

   orl $3,%ebx
   movl $BOOT_PAGE_PML4E_ADDRESS,%eax
   movl %ebx,(%eax)
   movl %ebx,8(%eax)
   movl $0,4(%eax)
   movl $0,12(%eax)
                  #init page tables

   movl %eax,%cr3

   movl %cr4,%eax
   orl $(1 << 5),%eax
   movl %eax,%cr4

   movl $0xc0000080,%ecx
   rdmsr
   orl $(1 << 8),%eax
   wrmsr
  
   movl %cr0,%eax
   orl $(1 << 31),%eax
   movl %eax,%cr0

   lgdt (boot_gdtr64)
   ljmp $0x8,$2f #go into long mode
2: .code64

   xorq %rcx,%rcx
   not %ecx
   andq %rcx,%rsp

   lgdt (boot_gdtr64_paging)

   popq %rbx
   popq %rax
   movabs $stack_top,%rsp #kernel stack

   movabs $(PAGE_OFFSET + 8),%rcx
   addq %rcx,%rbx
   movabs $_start64,%rcx
   jmp *%rcx

3:
   cli
   hlt
   jmp 3b

.section ".text","ax"
.code64
.extern kmain

_start64:
   pushq %rbx
   pushq %rax
   movq %rsp,%rdi

   callq kmain
0:
   hlt
   jmp 0b

.section ".bss","aw"
kernel_stack:
.fill 0x1000,0x1,0x0
stack_top:
