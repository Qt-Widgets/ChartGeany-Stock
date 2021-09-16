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

#include "asprintf.h"
#include "common.h"

#ifndef FORMAT
#ifdef 	Q_CC_GNU
#define FORMAT		__attribute__ ((format (printf, 1, 0)))
#else
#ifdef	Q_CC_MSVC
#define FORMAT		__attribute__ ((format (ms_printf, 1, 0)))
#else
#define FORMAT
#endif
#endif
#endif

/// stdio.h
// printf_imp: printf (3) hook implementation
extern "C" Q_DECL_EXPORT int
printf_imp (const char *str, va_list ap)
{
  QString output = "";
  char *ptr;

  if (vasprintf (&ptr, str, ap) != -1)
  {
    output = QString (ptr);
    free (ptr);
  }
  
  debugdialog->appendText (QString (output));
  return static_cast <int> (strlen (output.toUtf8 ()));
}
