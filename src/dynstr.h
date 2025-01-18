/**
 * @file dynstr.h
 *
 * @brief Dynamic string
 *
 * This is a simple implementation of a dynamic string, i.e. a string that is kept in
 * a buffer that is automatically reallocated if the space is over after a cat operation.
 *
 * SPDX-FileCopyrightText: 2008-2025 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

struct dynstr;

extern void dynstr_strcat(struct dynstr *str, const char *src, size_t len);
extern void dynstr_strcpy(struct dynstr **dst, struct dynstr *src);
extern void dynstr_printcat(struct dynstr *str, const char *fmt, ...);
extern void dynstr_init(struct dynstr **str, size_t len);
extern void dynstr_fini(struct dynstr **str);
extern char *dynstr_getbuff(struct dynstr *str);
