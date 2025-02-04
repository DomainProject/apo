/* SPDX-FileCopyrightText: 2023-2024 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdlib.h>

#define EXTLD(NAME) extern const char NAME[]; \
                    extern const size_t NAME_len;
#define LDVAR(NAME) NAME
#define LDLEN(NAME) NAME_len

// Define resources variables
EXTLD(ddm_v6_asp)

// Use this code to access the resources
//  size_t length = LDLEN(ddm_v6_asp);
//  uint8_t *data = LDVAR(ddm_v6_asp);
