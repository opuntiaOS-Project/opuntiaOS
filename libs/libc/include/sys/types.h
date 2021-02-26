#ifndef _LIBC_SYS_TYPES_H
#define _LIBC_SYS_TYPES_H

typedef unsigned long  uint64_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef long           int64_t;
typedef int            int32_t;
typedef short          int16_t;
typedef char           int8_t;

#define true  1
#define false 0

#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)
#define major(dev) ((unsigned int)((dev) >> MINORBITS))
#define minor(dev) ((unsigned int)((dev)&MINORMASK))
#define makedev(ma, mi) (((ma) << MINORBITS) | (mi))

#endif /* _LIBC_SYS_TYPES_H */