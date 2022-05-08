/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <mem/kmalloc.h>
#include <tasking/cpu.h>

#define DEBUG_UMEM_ACCESSES

/**
 * @brief Checks if the given string is valid and does not exceed the maximum length.
 *        Should be called before calling strlen on User C String.
 *
 * @param us The pointer to C String (char*).
 * @param maxlen The maximum length of the string.
 */
bool umem_validate_str(const char __user* us, size_t maxlen)
{
    char* s = (char*)us;
    if (!s) {
        return false;
    }

    for (int i = 0; i < maxlen + 1; i++) {
        if (s[i] == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Checks if the given array is valid and does not exceed the maximum length.
 *        Should be called before calling ptrarray_len on User Array.
 *
 * @param us The pointer to C array of pointers (char*).
 * @param maxlen The maximum length of the string.
 */
bool umem_validate_ptrarray(const void __user** s, size_t maxlen)
{
    if (!s) {
        return false;
    }

    for (int i = 0; i < maxlen + 1; i++) {
        if (s[i] == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Copies user's C String (char*) to kernel space.
 *
 * @param data Pointer to data in userspace.
 * @param size
 * @return char*
 */
void* umem_bring_to_kernel(const void __user* data, size_t size)
{
    void* kdata = kmalloc(size);
    if (!kdata) {
        return NULL;
    }
    umem_copy_from_user(kdata, data, size);
    return kdata;
}

char* umem_bring_to_kernel_str_with_len(const char __user* data, size_t size)
{
    char* kdata = (char*)kmalloc(size + 1);
    if (!kdata) {
        return NULL;
    }
    umem_copy_from_user(kdata, data, size);
    kdata[size] = '\0';
    return kdata;
}

/**
 * @brief Validates user's CString/char* and brings it to the kernel space.
 *
 * @param data Pointer to the string.
 * @return Pointer to the newly allocated string in kernel space.
 */
char* umem_bring_to_kernel_str(const char __user* data, size_t maxlen)
{
    if (!umem_validate_str(data, maxlen)) {
        return NULL;
    }
    return umem_bring_to_kernel_str_with_len(data, strlen((char*)data));
}

/**
 * @brief Validates user's Array of CString/char** and brings it to the kernel space.
 *
 * @param data Pointer to the array of strings.
 * @return Pointer to the newly allocated array in kernel space.
 */
char** umem_bring_to_kernel_strarray(const char __user** data, size_t maxlen)
{
    if (!umem_validate_strarray(data, maxlen)) {
        return NULL;
    }

    size_t arrlen = ptrarray_len((const void**)data);
    char** res = kmalloc(arrlen * sizeof(char*));

    for (int i = 0; i < arrlen; i++) {
        res[i] = umem_bring_to_kernel_str(data[i], maxlen);
    }

    return res;
}

#define access_ok(src, len) (THIS_CPU->data_access_type == DATA_ACCESS_KERNEL || (IS_USER_VADDR(src) && IS_USER_VADDR(src + len - 1)))

/**
 * @brief Copies data from the kernel buffer to the active address space.
 *
 * @param dest The data destination.
 * @param src The data source.
 * @param length The length of data to be copied.
 */
void umem_copy_to_user(void __user* dest, const void* src, size_t length)
{
    if (!access_ok((uintptr_t)dest, length)) {
#ifdef DEBUG_UMEM_ACCESSES
        // TODO: Remove kpanic when kernel is checked.
        kpanic("Access is not allowed. Were we in kernel?");
#endif // DEBUG_UMEM_ACCESSES
        return;
    }
    vmm_ensure_writing_to_active_address_space((uintptr_t)dest, length);
    memcpy((void*)dest, src, length);
}

/**
 * @brief Copies data from the user buffer to the kernel space.
 *
 * @param dest The data destination. Must be kernel address.
 * @param src The data source.
 * @param length The length of data to be copied.
 */
void umem_copy_from_user(void* dest, const void __user* src, size_t length)
{
    if (!access_ok((uintptr_t)src, length)) {
#ifdef DEBUG_UMEM_ACCESSES
        // TODO: Remove kpanic when kernel is checked.
        kpanic("Access is not allowed. Were we in kernel?");
#endif // DEBUG_UMEM_ACCESSES
        return;
    }
    memcpy(dest, (void*)src, length);
}
