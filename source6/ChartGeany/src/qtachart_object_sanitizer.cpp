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
 
// CGScript sanitizer 

#include "qtachart_object_sanitizer.h"
#include "qtachart_object.h"
#include "debugdialog.h"

extern DebugDialog *debugdialog;

ObjectSanitizer::~ObjectSanitizer (void)
{
  sumup ();
}

// sumup the counters to the parent's sanitizer
void
ObjectSanitizer::sumup (void)
{
  QTACObject *obj =
    static_cast <QTACObject *> (const_cast <void *> (object));

  if (CGSCRIPT_SANITIZER && obj->cgscriptdebug)
  {
    const QString type = obj->modinit==nullptr ? "Object: " : "Module: ";
    const QString dbg =
      CGLiteral ("\n") % type % obj->objectName () %
      CGLiteral (" sanitizer report:") % CGLiteral ("\n") %
      CGLiteral ("Array_t: alloc ") % QString::number (cgaac) %
      CGLiteral (" delloc ") % QString::number (cgadc) %
      CGLiteral (" leak ") % QString::number (cgaDiff ()) % CGLiteral ("\n") %

      CGLiteral ("String_t: alloc ") % QString::number (cgsac) %
      CGLiteral (" dealloc ") % QString::number (cgsdc) %
      CGLiteral (" leak ") % QString::number (cgsDiff ()) % CGLiteral ("\n") %

      CGLiteral ("ObjectHandler_t: alloc ") % QString::number (cgoac) %
      CGLiteral (" dealloc ") % QString::number (cgodc) %
      CGLiteral (" leak ") % QString::number (cgoDiff ()) % CGLiteral ("\n") %

      CGLiteral ("Array_t elements: alloc ") % QString::number (cgaeac) %
      CGLiteral (" dealloc ") % QString::number (cgaedc) %
      CGLiteral (" leak ") % QString::number (cgaeDiff ());

    debugdialog->appendText (dbg);
  }

  if (obj->modinit != nullptr)
    return;

  if  (obj->parentObject == nullptr)
    return;

  ObjectSanitizer *san = obj->parentObject->sanitizer;

  // sumup Array_t
  if (cgaac != 0) san->cgaInc (cgaac);
  if (cgadc != 0) san->cgaDec (cgadc);

  // sumup String_t
  if (cgsac != 0) san->cgsInc (cgsac);
  if (cgsdc != 0) san->cgsDec (cgsdc);

  // sumup ObjectHandler_t
  if (cgoac != 0) san->cgoInc (cgoac);
  if (cgodc != 0) san->cgoDec (cgodc);

  // sumup Array_t elements
  if (cgaeac != 0) san->cgaeInc (cgaeac);
  if (cgaedc != 0) san->cgaeDec (cgaedc);

  reset ();
}
