/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Modified by: bellrise
 */
#ifndef _LIBC_SYS_CDEFS_H
#define _LIBC_SYS_CDEFS_H

#if defined(__cplusplus)
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

/* Define __use_instead macro for some functions so the user can be warned
   about better/faster functions. */
#ifndef __use_instead
#ifdef __clang__
#define __use_instead(F) __attribute__((diagnose_if(1, "use " F " instead", \
    "warning")))
#elif defined(__GNUC__)
#define __use_instead(F) __attribute__((warning("use " F " instead")))
#endif
#endif

#endif // _LIBC_SYS_CDEFS_H
