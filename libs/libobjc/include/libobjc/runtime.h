#ifndef _LIBOBJC_RUNTIME_H
#define _LIBOBJC_RUNTIME_H

#include <libobjc/isa.h>
#include <libobjc/selector.h>
#include <stdint.h>

struct objc_class;
struct objc_object;
struct objc_selector;
struct category_t;

typedef struct objc_class* Class;
typedef struct objc_object* id;
typedef void (*IMP)(void /* id, SEL, ... */);
#define nil_method (IMP)0

union isa_t {
    isa_t() = default;
    isa_t(uintptr_t value)
        : m_bits(value)
    {
    }

    inline void set_class(Class cls, objc_object* obj) { m_cls = cls; }
    inline Class get_class() { return Class(m_bits & ISA_MASK); }

private:
    // Since all mallocs in the OS are 4 bytes aligned
    // the last bits we can reuse
    uintptr_t m_bits;
    Class m_cls;
};

struct objc_object {

    inline void init_isa(Class cls) { m_isa.set_class(cls, this); }
    Class isa() { return m_isa.get_class(); }

private:
    isa_t m_isa;
};

struct class_data_bits_t {
    friend objc_class;

private:
    uintptr_t bits;
    bool bit(uintptr_t bit) const { return bits & bit; }
};

struct objc_class : objc_object {
    objc_class(const objc_class&) = delete;
    objc_class(objc_class&&) = delete;
    void operator=(const objc_class&) = delete;
    void operator=(objc_class&&) = delete;

    inline void set_superclass(Class superclass) { m_superclass = superclass; }
    inline Class superclass() const { return m_superclass; }

private:
    Class m_superclass;
    class_data_bits_t m_bits;
};

#endif // _LIBOBJC_RUNTIME_H