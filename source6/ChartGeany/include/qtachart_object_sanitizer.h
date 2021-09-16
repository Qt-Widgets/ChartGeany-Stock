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
 
#pragma once

// we use int32_t instead of qint32
#include <cstdint>

#include "defs.h"

// core class for object sanitizer
class ObjectSanitizer
{
public:
  ObjectSanitizer (const void *obj) // constructor
  {
    object = obj;
    reset ();
  }

  ~ObjectSanitizer  (); // destructor

  // increase/decrease Array_t counter;
   inline void cgaInc (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgaac++ : cgaac+=c; }
   inline void cgaDec (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgadc++ : cgadc+=c; }
   inline GNUPURE int32_t cgaDiff (void) const Q_DECL_NOEXCEPT
  { return (cgaac - cgadc); }

  // increase/decrease String_t counter;
   inline void cgsInc (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgsac++ : cgsac+=c; }
   inline void cgsDec (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgsdc++ : cgsdc+=c; }
   inline GNUPURE int32_t cgsDiff (void) const Q_DECL_NOEXCEPT
  { return (cgsac - cgsdc); }

  // increase/decrease ObjectHandler_t counter;
   inline void cgoInc (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgoac++ : cgoac+=c; }
   inline void cgoDec (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgodc++ : cgodc+=c; }
   inline GNUPURE int32_t cgoDiff (void) const Q_DECL_NOEXCEPT
  { return (cgoac - cgodc); }

  // increase/decrease Array_t element counter;
   inline void cgaeInc (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgaeac++ : cgaeac+=c; }
   inline void cgaeDec (int32_t c=0) Q_DECL_NOEXCEPT { c==0 ? cgaedc++ : cgaedc+=c; }
   inline GNUPURE int32_t cgaeDiff (void) const Q_DECL_NOEXCEPT
  { return (cgaeac - cgaedc); }

private:
  const void *object;            // referenced QTACObject
  int32_t cgaac;                 // CGScript Array_t alloc count
  int32_t cgadc;                 // CGScript Array_t disalloc count
  int32_t cgsac;                 // CGScript String_t alloc count
  int32_t cgsdc;                 // CGScript String_r disalloc count
  int32_t cgoac;                 // CGScript ObjectHandler_t alloc count
  int32_t cgodc;                 // CGScript ObjectHandler_t disalloc count
  int32_t cgaeac;                // CGScript Array_t element alloc count
  int32_t cgaedc;                // CGScript Array_t element disalloc count

  // reset the counters
   inline void reset (void) Q_DECL_NOEXCEPT
  { cgaac=cgadc=cgsac=cgsdc=cgoac=cgodc=cgaeac=cgaedc=0; }

  // sumup the counters to the parent's sanitizer
  void sumup (void);
};
