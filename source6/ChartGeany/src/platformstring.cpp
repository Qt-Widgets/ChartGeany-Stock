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

#ifdef CGTOOL
#include "cgtool.h"
#else
#include "common.h"
#endif

#include <QString>
#include "cgliteral.h"

// returns the platform string (eg linux64-gcc)
QString
platformString (void)
{
  QString platform = CGLiteral ("unknown"), compiler = CGLiteral ("unknown");

#ifdef Q_OS_LINUX
  platform = CGLiteral ("linux");
#endif

#ifdef Q_OS_WIN32
  platform = CGLiteral ("win");
#endif

#ifdef Q_OS_MAC
  platform = CGLiteral ("darwin");
#endif

#ifdef Q_OS_SOLARIS
  platform = CGLiteral ("sunos");
#endif

  if (QT_POINTER_SIZE == 4)
    platform += CGLiteral ("32");
  else
    platform += CGLiteral ("64");

#ifdef Q_OS_MAC
  compiler = CGLiteral ("-cc");
#endif

#ifdef Q_OS_WIN32
  compiler = CGLiteral ("-cgs");
#endif

#ifdef Q_OS_LINUX
  compiler = CGLiteral ("-cgs");
#endif

#ifdef Q_OS_SOLARIS
  compiler = CGLiteral ("-gcc");
#endif

  platform = platform + compiler;

  return platform;
}
