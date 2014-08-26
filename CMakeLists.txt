cmake_minimum_required(VERSION 3.0)

enable_language(ASM-ATT)
set(CMAKE_CXX_FLAGS "-nostdlib -fexceptions \
     -ffreestanding -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
     -mno-sse3 -mno-3dnow -fno-strict-aliasing -mcmodel=large \
     -nostdlib -m64 -fno-builtin -O3 -Wall -Werror -std=c++14 \
     -fno-rtti -Wextra")

include_directories("include")

set(SRC_LIST   arch/boot/boot.s
               arch/init.cpp
               arch/cpu/gdt.cpp
               arch/cpu/paging.cpp
               kernel/init/main.cpp)
set(CDROM_FILE os.iso)
set(CDROM_DIR "cdrom")
set(BINARY_KERNEL "cdrom/bkernel")

add_executable(kernel ${SRC_LIST})
set_target_properties(kernel PROPERTIES LINK_FLAGS "-nostdlib -Tkernel.ldscript -s")

add_custom_command(OUTPUT "cdrom/efi.img"
   COMMAND cp /usr/lib/grub/x86_64-efi/kernel.img efi.img
   COMMAND echo 1 > ../updated #tell iso generater we updated..
   WORKING_DIRECTORY ${CDROM_DIR}
   COMMENT "Copying efi.img..."
) #FIXME: it seems I shouldn't do this..
  #but if I don't,grub-mkrescue doesn't work....
add_custom_target(copy_efi ALL DEPENDS "cdrom/efi.img")

add_custom_command(OUTPUT ${BINARY_KERNEL}
   COMMAND objcopy -O binary ../kernel bkernel
   COMMAND echo 1 > ../updated #tell iso generater we updated..
   WORKING_DIRECTORY ${CDROM_DIR}
   COMMENT "Generating binary kernel..."
   DEPENDS kernel
)
add_custom_target(binary_kernel ALL DEPENDS kernel ${BINARY_KERNEL})

add_custom_command(OUTPUT ${CDROM_FILE}
   COMMAND grub-mkrescue -o ${CDROM_FILE} ${CDROM_DIR}
   COMMENT "Generating ${CDROM_FILE}..."
   DEPENDS "compile/updated"
)
add_custom_target(cdrom ALL DEPENDS binary_kernel copy_efi ${CDROM_FILE})

add_custom_target(run DEPENDS cdrom
   COMMAND qemu-system-x86_64 -cdrom ${CDROM_FILE}
   COMMENT "Running docq os by qemu..."
) #make run
