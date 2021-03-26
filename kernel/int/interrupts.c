#include "interrupts.h"
#include "idt.h"
#include <stddef.h>
#include <stdint.h>

isr_t isr_handler_array[256];

//TODO: Update exception messages to comply with the AMD manual
static const char* exception_messages[31] = {
   "Type: (#DE) Division Exception",
   "Type: (#DB) Debug Exception",
   "Type: (NONE) NMI interrupt (Non Maskable Interrupt)",
   "Type: (#BP) Breakpoint Exception",
   "Type: (#OF) Into detected Overflow Exception",
   "Type: (#BR) BOUND Range Exceeded",
   "Type: (#UD) Invalid Opcode (Undefined Opcode)",
   "Type: (#NM) Device Not Available (No Math Coprocessor)",
   "Type: (#DF) Double Fault",
   "Type: (NONE) Coprocessor Segment Overrun (reserved)",
   "Type: (#TS) Invalid TSS",
   "Type: (#NP) Segment Not Present",
   "Type: (#SS) Stack-Segment Fault",
   "Type: (#GP) General Protection Fault",
   "Type: (#PF) Page Fault",
   "Type: (NONE) Unknown interrupt",
   "Type: (#CF) Coprocessor fault",
   "Type: (#AC) Alignment check",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved",
   "Type: (NONE) Reserved"
};

void irq_handler(regs_t regs) {
    if (regs.isr_number >= 40 /*irq8+*/)
    {
        //master and slave eoi
        outb(0xA0, 0x20);
        outb(0x20, 0x20);
    }
    else {
        //master eoi
        outb(0x20, 0x20);
    }
    if (isr_handler_array[regs.isr_number != 0]) {
        isr_t handler = isr_handler_array[regs.isr_number];
        handler(regs);
    }
}

void isr_handler(regs_t regs)
{
    /* CPU exceptions */
    if (regs.isr_number < 32)
    {
        set_color(VGA_BLACK, VGA_LIGHT_RED);
        serial_set_color(BASH_RED);
        kprintf("[INTR] %s (err_code %d)\n", exception_messages[regs.isr_number], regs.error_code);
        debug("INT#%d - %s (err_code %d)\n", regs.isr_number, exception_messages[regs.isr_number], regs.error_code);
        serial_set_color(BASH_WHITE);
        debug("Register dump:\n"                      \
                "rax 0x%x, rbx 0x%x, rcx 0x%x, rdx 0x%x\n"    \
                "rbp 0x%x, rsp 0x%x, rdi 0x%x, rsi 0x%x\n"    \
                "rip 0x%x, cs  0x%x, ss  0x%x, rflags 0x%x\n" \
                "r8  0x%x, r9  0x%x, r10 0x%x, r11  0x%x\n"   \
                "r12 0x%x, r13 0x%x, r14 0x%x, r15  0x%x\n",
                                                                regs.rax,
                                                                regs.rbx,
                                                                regs.rcx,
                                                                regs.rdx,
                                                                regs.rbp,
                                                                regs.rsp,
                                                                regs.rdi,
                                                                regs.rsi,
                                                                regs.rip,
                                                                regs.cs,
                                                                regs.ss,
                                                                regs.rflags,
                                                                regs.r8,
                                                                regs.r9,
                                                                regs.r10,
                                                                regs.r11,
                                                                regs.r12,
                                                                regs.r13,
                                                                regs.r14,
                                                                regs.r15
                                                                );
        
        set_color(VGA_BLACK, VGA_WHITE);
        
        asm("hlt");
    }
    
    if (isr_handler_array[regs.isr_number] != 0)
    {
        isr_t handler = isr_handler_array[regs.isr_number];
        handler(regs);
    }

    else {
        kprintf("Unhandled interrupt: 0x%x (%d)\n", regs.isr_number);
    }
    
    /* Todo: Handle isr's 48-255 ie. user defined isr's (note: isr's 32-47 are marked as IRQ's and handled separately */
}

void install_isr(uint8_t base, isr_t handler)
{
    if (isr_handler_array[base] == 0)
        isr_handler_array[base] = handler;
    else
        kprintf("The interrupt ( %d ) has already been registered!\n", base);
}

void uninstall_isr(uint8_t base)
{
    isr_handler_array[base] = 0;
}