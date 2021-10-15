/**
 * @file bootloader_stivale2.c
 * @author Tim (V01D)
 * @brief Hanldes information from the bootloader
 * @version 0.1
 * @date 2021-04-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "bootloader_stivale2.h"
#include <drivers/io/serial.h>
#include <amd64/moon.h>
#include <stivale2.h>
#include <int/idt.h>
#include <int/gdt.h>
#include <kernel.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/memdefs.h>
#include <panic.h>
#include <stdint.h>
#include <printk.h>
#include <libgraphics/double-buffering.h>
#include <libgraphics/draw.h>
#include <libgraphics/bootsplash.h>
#include <drivers/keyboard/keyboard.h>

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id);

static __section_align uint8_t stack[345859];

struct stivale2_struct_tag_rsdp rsdp_tag = {
    .tag = {
        .identifier = STIVALE2_STRUCT_TAG_RSDP_ID,
        .next = 0}};

struct stivale2_tag level5_paging_tag = {
    .identifier = STIVALE2_HEADER_TAG_5LV_PAGING_ID,
    .next = (uintptr_t)&rsdp_tag};

struct stivale2_header_tag_smp smp_hdr_tag = {
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_SMP_ID,
        .next = (uintptr_t)&level5_paging_tag}};

struct stivale2_header_tag_framebuffer framebuffer_hdr_tag = {
    // All tags need to begin with an identifier and a pointer to the next tag.
    .tag = {
        .identifier = STIVALE2_HEADER_TAG_FRAMEBUFFER_ID,
        .next = (uintptr_t)&smp_hdr_tag},
    .framebuffer_width = 0,
    .framebuffer_height = 0,
    .framebuffer_bpp = 0};


__SECTION(".stivale2hdr")
struct stivale2_header stivale_hdr = {
    .entry_point = 0,
    .stack = (uintptr_t)stack + sizeof(stack),
    .flags = 0,
    .tags = (uintptr_t)&framebuffer_hdr_tag
};

void *stivale2_get_tag(struct stivale2_struct *stivale2_struct, uint64_t id)
{
    struct stivale2_tag *current_tag = (void *)stivale2_struct->tags;
    for (;;)
    {

        if (current_tag == NULL)
        {
            return NULL;
        }

        if (current_tag->identifier == id)
        {
            return current_tag;
        }

        // Get a pointer to the next tag in the linked list and repeat.
        current_tag = (void *)current_tag->next;
    }
}

const char serial_message[] = {
    "███╗   ███╗ ██████╗  ██████╗ ███╗   ██╗ ██████╗ ███████╗ \n"
    "████╗ ████║██╔═══██╗██╔═══██╗████╗  ██║██╔═══██╗██╔════╝ \n"
    "██╔████╔██║██║   ██║██║   ██║██╔██╗ ██║██║   ██║███████╗ \n"
    "██║╚██╔╝██║██║   ██║██║   ██║██║╚██╗██║██║   ██║╚════██║ \n"
    "██║ ╚═╝ ██║╚██████╔╝╚██████╔╝██║ ╚████║╚██████╔╝███████║ \n"
    "╚═╝     ╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝ ╚══════╝ \n"};

const char fb_message[] = {
    "\n"
    " _____             _____ _____  \n"
    "|     |___ ___ ___|     |   __| \n"
    "| | | | . | . |   |  |  |__   | \n"
    "|_|_|_|___|___|_|_|_____|_____| \n"
    "\n"};

void banner(bool serial_only)
{
    if (serial_only)
    {
        debug(false, "%s", serial_message);
        return;
    }

    printk("main", "Welcome to MoonOS\n");
    printk("banner", "%s", fb_message);
    delay(200);
}

/**
 * @brief Kernel entry point
 * 
 * @param[in] bootloader_info Various information from the limine bootloader
 */
void kinit(struct stivale2_struct *bootloader_info)
{
    boot_info_t bootvars; //Hardware information from the bootloader
    serial_set_color(BASH_WHITE);
    banner(true); /* Write banner to serial device */

    struct stivale2_struct_tag_memmap *mmap = stivale2_get_tag(bootloader_info, STIVALE2_STRUCT_TAG_MEMMAP_ID);
    struct stivale2_struct_tag_framebuffer *fb = stivale2_get_tag(bootloader_info, STIVALE2_STRUCT_TAG_FRAMEBUFFER_ID);
    struct stivale2_struct_tag_smp *smp = stivale2_get_tag(bootloader_info, STIVALE2_STRUCT_TAG_SMP_ID);
    struct stivale2_struct_tag_rsdp *rsdp = stivale2_get_tag(bootloader_info, STIVALE2_STRUCT_TAG_RSDP_ID);

    init_gdt();
    init_idt();

    if (fb != NULL)
    {
        bootvars.vesa.fb_addr = fb->framebuffer_addr;
        bootvars.vesa.fb_width = fb->framebuffer_width;
        bootvars.vesa.fb_height = fb->framebuffer_height;
        bootvars.vesa.fb_bpp = fb->framebuffer_bpp;
        bootvars.vesa.fb_pitch = fb->framebuffer_pitch;
    }

    if (mmap != NULL)
    {
        pmm_init(mmap->memmap, mmap->entries);
        vmm_init(check_la57());
        create_safe_panic_area();

        double_buffering_init(&bootvars);
        printk_init();
        generic_keyboard_init(CHARSET_EN_US);

        bootsplash();

        banner(false);
        printk("pmm", "Initialized pmm\n");

        /* vmm */
        if (check_la57())
        {
            debug(true, "Using 5 level paging\n");
            printk("vmm", "pml5 resides at 0x%llx\n", cr_read(CR3));
        }
        else
        {
            debug(true, "Using 4 level paging\n");
            printk("vmm", "pml4 resides at 0x%llx\n", cr_read(CR3));
        }
        printk("vmm", "Initialized vmm\n");
        printk("bootloader-stivale2", "Initialized double buffering\n");
    }
    else
    {
        debug(false, "\n!Did not get a memory map from the bootloader!\n");
        for (;;)
            ;
    }

    if (smp != NULL)
    {
        bootvars.cpu.processor_count = smp->cpu_count;
        bootvars.cpu.bootstrap_processor_lapic_id = smp->bsp_lapic_id;
        bootvars.cpu.acpi_processor_uid = smp->smp_info->processor_id;
        bootvars.cpu.lapic_id = smp->smp_info->lapic_id;
        bootvars.cpu.smp_info = smp->smp_info;
    }

    if (rsdp != NULL)
    {
        bootvars.rsdp.rsdp_address = rsdp->rsdp;
    }

    log_cpuid_results();

    kmain(&bootvars);
}