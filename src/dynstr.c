/**
 * @file dynstr.c
 *
 * @brief Dynamic string
 *
 * This is a simple implementation of a dynamic string, i.e. a string that is kept in
 * a buffer that is automatically reallocated if the space is over after a cat operation.
 *
 * SPDX-FileCopyrightText: 2008-2025 Alessandro Pellegrini <a.pellegrini@ing.uniroma2.it>
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "dynstr.h"


/**
 * @brief A simple dynamic string data structure to assemble command line arguments
 */
struct dynstr {
	char *buff; ///< The current string
	size_t size; ///< The total number of available characters
	size_t used; ///< The current number of used characters
};


/**
 * @brief A version of strcat() working on struct dynstr
 * @param str The dynamic string where to perform the cat
 * @param src The string to cat
 * @param len The length of the string to cat
 */
void dynstr_strcat(struct dynstr *str, const unsigned char *src, size_t len)
{
	while(str->used + len >= str->size) {
		str->size *= 2;
		str->buff = realloc(str->buff, str->size);
		if(!str->buff) {
			perror("dynstr_strcat: could not allocate memory for new_buffer");
			exit(EXIT_FAILURE);
		}
	}

	strncat(str->buff, src, len);
	str->used += len;
}

/**
 * @brief A version of strcpy() working on struct dynstr
 * @param dst The dynamic string to store the copy. It must not be alrady initalized.
 * @param src The string to copy.
 */
void dynstr_strcpy(struct dynstr **dst, struct dynstr *src)
{
	dynstr_init(dst, src->used);
	dynstr_strcat(*dst, src->buff, src->used);
}

/**
 * @brief Function to assemble a string to append to a dynamic string
 *
 * @param str The dynamic string to append the generated string to
 * @param fmt The format string (printf-like)
 * @param ... A variable number of arguments
 */
void dynstr_printcat(struct dynstr *str, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	size_t len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	char buffer[len + 1];

	va_start(args, fmt);
	vsprintf(buffer, fmt, args);
	va_end(args);

	dynstr_strcat(str, buffer, len);
}

/**
 * @brief Initialize a dynamic string
 * @param str The dynamic string to initialize
 * @param len The initial size of the buffer
 */
void dynstr_init(struct dynstr **str, size_t len)
{
	assert(len > 0);
	size_t pow2len = (size_t)1 << ((sizeof(len) * 8) - __builtin_clzl(len - 1));
	*str = malloc(sizeof(**str));
	(*str)->buff = malloc(pow2len);
	(*str)->buff[0] = '\0';
	(*str)->size = pow2len;
	(*str)->used = 1;
}

/**
 * @brief Free a dynamic string
 *
 * This function frees all the memory internally used by a dynamic string.
 *
 * @param str the dynamic string to release
 */
void dynstr_fini(struct dynstr **str)
{
	assert(str);
	assert(*str);
	free((*str)->buff);
	free(*str);
	*str = NULL;
}

/**
 * @brief Access the current string kept by the dynamic string
 *
 * @param str The dynamic string to be accessed
 *
 * @return a pointer to a C string
 */
char *dynstr_getbuff(struct dynstr *str)
{
	return str->buff;
}
