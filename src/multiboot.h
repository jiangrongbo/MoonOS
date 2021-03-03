// Declares the multiboot info structure

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

//
//https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format
//

#include "common.h"

#define MULTIBOOT_FLAG_MEM      0x001
#define MULTIBOOT_FLAG_DEVICE   0x002
#define MULTIBOOT_FLAG_CMDLINE  0x004
#define MULTIBOOT_FLAG_MODS     0x008
#define MULTIBOOT_FLAG_AOUT     0x010
#define MULTIBOOT_FLAG_ELF      0x020
#define MULTIBOOT_FLAG_MMAP     0x040
#define MULTIBOOT_FLAG_CONFIG   0x080
#define MULTIBOOT_FLAG_LOADER   0x100
#define MULTIBOOT_FLAG_APM      0x200
#define MULTIBOOT_FLAG_VBE      0x400

typedef unsigned char           multiboot_uint8_t;
typedef unsigned short          multiboot_uint16_t;
typedef unsigned int            multiboot_uint32_t;
typedef unsigned long long      multiboot_uint64_t;

struct multiboot
{
    u32int flags;
    u32int mem_lower;
    u32int mem_upper;
    u32int boot_device;
    u32int mods_count;
    void   *mods_addr;
    u32int num;
    u32int size;
    u32int addr;
    u32int shndx;
    u32int mmap_length;
    void   *mmap_addr;
    u32int drives_length;
    u32int drives_addr;
    u32int config_table;
    u32int boot_loader_name;
    u32int apm_table;
    u32int vbe_control_info;
    u32int vbe_mode_info;
    u32int vbe_mode;
    u32int vbe_interface_seg;
    u32int vbe_interface_off;
    u32int vbe_interface_len;
}__attribute__((section(".multiboot")));

typedef struct multiboot_header multiboot_header_t;

struct multiboot_mmap {
    u32int              entry_size;
    unsigned long long  base;
    unsigned long long  length;
    u32int              type;
}__attribute__((section(".multiboot")));

struct multiboot_mods
{
    u32int    mod_start;
    u32int    mod_end;
    char      *cmdline;
    u32int    reserved;
}__attribute__((section(".multiboot")));

#endif  // MULTIBOOT_H
