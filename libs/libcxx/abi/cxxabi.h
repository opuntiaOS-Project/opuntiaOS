#ifndef _CXXABI_H
#define _CXXABI_H

#define VIS_HIDDEN __attribute__((visibility("hidden")))
#define VIS_INTERNAL __attribute__((visibility("internal")))

extern "C" {
int __cxa_atexit(void (*)(void*), void*, void*);
void __cxa_finalize(void*);
}

#endif /* _CXXABI_H */
