/* SPDX-FileCopyrightText: 2023-2024 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdlib.h>

#ifdef __APPLE__
#include <mach-o/getsect.h>

#define EXTLD(NAME) extern const unsigned char _section$__DATA__##NAME[];
#define LDVAR(NAME) _section$__DATA__##NAME
#define LDLEN(NAME) (getsectbyname("__DATA", "__" #NAME)->size)

#elif (defined __WIN32__) /* mingw */

#define EXTLD(NAME)                                                                                                    \
	extern const unsigned char binary_##NAME##_start[];                                                            \
	extern const unsigned char binary_##NAME##_end[];
#define LDVAR(NAME) binary_##NAME##_start
#define LDLEN(NAME) ((binary_##NAME##_end) - (binary_##NAME##_start))

#else /* gnu/linux ld */

#define EXTLD(NAME)                                                                                                    \
	extern const unsigned char _binary_##NAME##_start[];                                                           \
	extern const unsigned char _binary_##NAME##_end[];
#define LDVAR(NAME) (const char *)_binary_##NAME##_start
#define LDLEN(NAME) ((_binary_##NAME##_end) - (_binary_##NAME##_start))
#endif

// Define resources variables
EXTLD(ddm_v5_asp)

// Use this code to access the resources
//  size_t length = LDLEN(ddm_v5_asp);
//  uint8_t *data = LDVAR(ddm_v5_asp);
