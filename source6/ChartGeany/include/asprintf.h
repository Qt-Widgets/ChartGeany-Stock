/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */
 
#ifndef _ASPRINTF_H
#define _ASPRINTF_H

#if defined (_WIN32) || defined (_WIN64)

#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

extern 
int _vscprintf_so (const char * format, va_list pargs);
    
extern
int vasprintf (char **strp, const char *fmt, va_list ap);

extern 
int asprintf (char *strp[], const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* defined (_WIN32) || defined (_WIN64) */
#endif /* _ASPRINTF_H */
