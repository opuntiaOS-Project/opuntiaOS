#ifndef __oneOS__LibCXX__TYPES_H
#define __oneOS__LibCXX__TYPES_H

typedef unsigned long  uint64_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef long           int64_t;
typedef int            int32_t;
typedef short          int16_t;
typedef char           int8_t;

#ifdef __i386__
typedef unsigned long size_t;
#elif __arm__
typedef unsigned int size_t;
#endif

#ifdef __cplusplus
typedef decltype(nullptr) nullptr_t;
#endif

#define true  1
#define false 0
#define NULL ((void*)0)

#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)
#define major(dev) ((unsigned int)((dev) >> MINORBITS))
#define minor(dev) ((unsigned int)((dev)&MINORMASK))
#define makedev(ma, mi) (((ma) << MINORBITS) | (mi))

#endif /* __oneOS__LibC__TYPES_H */