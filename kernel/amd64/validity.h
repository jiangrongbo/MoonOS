#ifndef VALIDITY_DEFS_H
#define VALIDITY_DEFS_H

//Some #define's were taken from https://github.com/GENADEV/GENADEV_OS/blob/main/src/kernel/arm-v-8/genadev.h
#define __export extern 
#define __no_optimize volatile
#define __no_return __attribute__((noreturn)) void
#define __section_align  __attribute__((aligned (16)))
#define __section_align8 __attribute__((aligned (8)))
#define __pack_bytes __attribute__((packed))
#define __SECTION(_section) __attribute__((section(_section)))
#define __DEPRECATION_INBOUND __attribute__((warning("This functionality is marked for deprecation. Please update your code to meet the latest requirements ASAP.")))
#define __DEPRECATION_OVERDUE __attribute__((error ("This functionality is marked deprecated. Please update your code to meet the latest requirements to ensure a successful compilation of the Validity kernel.")))

#endif // VALIDITY_DEFS_H