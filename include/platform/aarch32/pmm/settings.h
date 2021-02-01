#ifndef __oneOS__AARCH32__PMM__SETTINGS_H
#define __oneOS__AARCH32__PMM__SETTINGS_H

/* TODO: By default settings for Cortex-A15 are here */
#include <platform/aarch32/target/cortex-a15/memmap.h>

#define MEMORY_MAP_REGION (arm_memmap)
#define PMM_BLOCK_SIZE (1024)
#define PMM_BLOCK_SIZE_KB (1)
#define PMM_BLOCKS_PER_BYTE (8)

#endif /* __oneOS__AARCH32__PMM__SETTINGS_H */