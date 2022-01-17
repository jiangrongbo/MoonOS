#include "task.h"
#include <printk.h>

// Temporary code for testing
static task_t tasks[10];
static int counter = 0;

void add_task(task_t task)
{
    printk("task", "Task: %s | Pagemap: 0x%lX | Virtual entrypoint: 0x%lx\n", task.descriptor,  task.pagemap, task.entrypoint);
    tasks[counter++] = task;
}

task_t new_task(const char *descriptor, Elf64_Addr entrypoint)
{
    return (task_t){.descriptor = descriptor, .pagemap = vmm_create_new_pagemap(), .entrypoint = entrypoint};
}