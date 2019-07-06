#include "../port/port.h"

#define MASTER_PIC_CMD  0x0020
#define MASTER_PIC_DATA 0x0021
#define SLAVE_PIC_CMD   0x00A0
#define SLAVE_PIC_DATA  0x00A1
#define ICW4_8086	    0x01

void pic_remap(unsigned int offset1, unsigned int offset2);