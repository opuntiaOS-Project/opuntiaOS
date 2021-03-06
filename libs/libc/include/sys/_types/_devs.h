#ifndef _LIBC_SYS__TYPES__DEVS_H
#define _LIBC_SYS__TYPES__DEVS_H

#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)
#define major(dev) ((unsigned int)((dev) >> MINORBITS))
#define minor(dev) ((unsigned int)((dev)&MINORMASK))
#define makedev(ma, mi) (((ma) << MINORBITS) | (mi))

#endif // _LIBC_SYS__TYPES__DEVS_H