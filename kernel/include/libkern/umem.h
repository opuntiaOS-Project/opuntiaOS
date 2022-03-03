#ifndef _KERNEL_LIBKERN_UMEM_H
#define _KERNEL_LIBKERN_UMEM_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

#define USER_STR_MAXLEN (128)
#define USER_PTRARRAY_MAXLEN (128)
#define USER_STRARRAY_MAXLEN (128)

bool umem_validate_str(const char __user* us, size_t maxlen);
bool umem_validate_ptrarray(const void __user** uptrarr, size_t maxlen);
static ALWAYS_INLINE bool umem_validate_strarray(const char __user** s, size_t maxlen) { return umem_validate_ptrarray((const void __user**)s, maxlen); }
void* umem_bring_to_kernel(const void __user* data, size_t size);
char* umem_bring_to_kernel_str(const char __user* data, size_t maxlen);
char* umem_bring_to_kernel_str_with_len(const char __user* data, size_t size);
char** umem_bring_to_kernel_strarray(const char __user** udata, size_t maxlen);
void umem_copy_to_user(void __user* dest, const void* src, size_t length);
void umem_copy_from_user(void* dest, const void __user* src, size_t length);

static ALWAYS_INLINE void umem_get_user_1(uint8_t* ptr, uint8_t* uptr) { *ptr = *uptr; }
static ALWAYS_INLINE void umem_get_user_2(uint16_t* ptr, uint16_t* uptr) { *ptr = *uptr; }
static ALWAYS_INLINE void umem_get_user_4(uint32_t* ptr, uint32_t* uptr) { *ptr = *uptr; }
static ALWAYS_INLINE void umem_get_user_8(uint64_t* ptr, uint64_t* uptr) { *ptr = *uptr; }

static ALWAYS_INLINE void umem_put_user_1(uint8_t* val, uint8_t* uptr) { *uptr = *val; }
static ALWAYS_INLINE void umem_put_user_2(uint16_t* val, uint16_t* uptr) { *uptr = *val; }
static ALWAYS_INLINE void umem_put_user_4(uint32_t* val, uint32_t* uptr) { *uptr = *val; }
static ALWAYS_INLINE void umem_put_user_8(uint64_t* val, uint64_t* uptr) { *uptr = *val; }

#define __umem_put_user_impl_size(size, x, ptr) umem_put_user_##size(x, ptr)
#define umem_put_user(x, ptr)                                                     \
    ({                                                                            \
        __typeof__(*(ptr)) __user_val;                                            \
        __user_val = x;                                                           \
        switch (sizeof(*(ptr))) {                                                 \
        case 1:                                                                   \
            __umem_put_user_impl_size(1, (uint8_t*)&__user_val, (uint8_t*)ptr);   \
            break;                                                                \
        case 2:                                                                   \
            __umem_put_user_impl_size(2, (uint16_t*)&__user_val, (uint16_t*)ptr); \
            break;                                                                \
        case 4:                                                                   \
            __umem_put_user_impl_size(4, (uint32_t*)&__user_val, (uint32_t*)ptr); \
            break;                                                                \
        case 8:                                                                   \
            __umem_put_user_impl_size(8, (uint64_t*)&__user_val, (uint64_t*)ptr); \
            break;                                                                \
        default:                                                                  \
            umem_copy_to_user(ptr, &__user_val, sizeof(*(ptr)));                  \
            break;                                                                \
        }                                                                         \
    })

#define __umem_get_user_impl_size(size, x, ptr) umem_get_user_##size(x, ptr)
#define umem_get_user(ptr, uptr)                                           \
    ({                                                                     \
        switch (sizeof(*(ptr))) {                                          \
        case 1:                                                            \
            __umem_get_user_impl_size(1, (uint8_t*)ptr, (uint8_t*)uptr);   \
            break;                                                         \
        case 2:                                                            \
            __umem_get_user_impl_size(2, (uint16_t*)ptr, (uint16_t*)uptr); \
            break;                                                         \
        case 4:                                                            \
            __umem_get_user_impl_size(4, (uint32_t*)ptr, (uint32_t*)uptr); \
            break;                                                         \
        case 8:                                                            \
            __umem_get_user_impl_size(8, (uint64_t*)ptr, (uint64_t*)uptr); \
            break;                                                         \
        default:                                                           \
            umem_copy_from_user(ptr, uptr, sizeof(*(ptr)));                \
            break;                                                         \
        }                                                                  \
    })

#endif // _KERNEL_LIBKERN_UMEM_H
