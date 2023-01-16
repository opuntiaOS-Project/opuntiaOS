#ifndef _CXXABI_H
#define _CXXABI_H

#include <stdint.h>
#include <typeinfo>

#define VIS_HIDDEN __attribute__((visibility("hidden")))
#define VIS_INTERNAL __attribute__((visibility("internal")))

extern "C" {
int __cxa_atexit(void (*)(void*), void*, void*);
void __cxa_finalize(void*);
}

namespace __cxxabiv1 {

// Defenitions from https://itanium-cxx-abi.github.io/cxx-abi/abi.html
class __fundamental_type_info : public std::type_info {
public:
    ~__fundamental_type_info();
};
class __array_type_info : public std::type_info {
public:
    ~__array_type_info();
};
class __function_type_info : public std::type_info {
public:
    ~__function_type_info();
};
class __enum_type_info : public std::type_info {
public:
    ~__enum_type_info();
};
class __class_type_info : public std::type_info {
public:
    ~__class_type_info();

    enum __class_type_info_codes {
        CLASS_TYPE_INFO_CODE,
        SI_CLASS_TYPE_INFO_CODE,
        VMI_CLASS_TYPE_INFO_CODE,
    };

    virtual __class_type_info_codes class_type_info_code() const { return CLASS_TYPE_INFO_CODE; }
};
class __si_class_type_info : public __class_type_info {
public:
    ~__si_class_type_info();
    const __class_type_info* __base_type;

    virtual __class_type_info_codes class_type_info_code() const { return SI_CLASS_TYPE_INFO_CODE; }
};

struct __base_class_type_info {
public:
    const __class_type_info* __base_type;
    long __offset_flags;

    enum __offset_flags_masks {
        __virtual_mask = 0x1,
        __public_mask = 0x2,
        __offset_shift = 8
    };

    inline bool is_virtual() const { return (__offset_flags & __virtual_mask) != 0; }
    inline bool is_public() const { return (__offset_flags & __public_mask) != 0; }
    inline long offset() const { return __offset_flags >> __offset_shift; }
    inline long flags() const { return __offset_flags & ((1L << __offset_shift) - 1); }
};

class __vmi_class_type_info : public __class_type_info {
public:
    ~__vmi_class_type_info();

    uint32_t m_flags;
    uint32_t m_base_count;
    __base_class_type_info m_base_info[1];

    enum __flags_masks {
        __non_diamond_repeat_mask = 0x1,
        __diamond_shaped_mask = 0x2
    };

    virtual __class_type_info_codes class_type_info_code() const { return SI_CLASS_TYPE_INFO_CODE; }
};

class __pbase_type_info : public std::type_info {
public:
    ~__pbase_type_info();

    unsigned int __flags;
    const std::type_info* __pointee;

    enum __masks {
        __const_mask = 0x1,
        __volatile_mask = 0x2,
        __restrict_mask = 0x4,
        __incomplete_mask = 0x8,
        __incomplete_class_mask = 0x10,
        __transaction_safe_mask = 0x20,
        __noexcept_mask = 0x40
    };
};

class __pointer_type_info : public __pbase_type_info {
public:
    ~__pointer_type_info();
};

class __pointer_to_member_type_info : public __pbase_type_info {
public:
    ~__pointer_to_member_type_info();
};

}

namespace abi = __cxxabiv1;

#endif /* _CXXABI_H */
