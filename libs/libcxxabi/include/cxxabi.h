#ifndef _CXXABI_H
#define _CXXABI_H

extern "C" {
int __cxa_atexit(void (*)(void*), void*, void*);
void __cxa_finalize(void*);
}

#endif /* _CXXABI_H */
