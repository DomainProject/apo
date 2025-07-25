/* SPDX-FileCopyrightText: 2023-2024 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#pragma once

#include <stdlib.h>

#define EXTLD(NAME) extern const unsigned char (NAME)[]; \
                    extern const size_t NAME ##_len;
#define LDVAR(NAME) NAME
#define LDLEN(NAME) NAME ##_len

// Define resources variables
EXTLD(ddm_asp)

// Use this code to access the resources
//  size_t length = LDLEN(ddm_asp);
//  uint8_t *data = LDVAR(ddm_asp);
