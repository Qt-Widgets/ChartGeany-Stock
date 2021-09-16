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

#include <QString>
#include <QVector>
#include "defs.h"
#include "cgscript.h"
#include "cgliteral.h"

// Error registry record
typedef struct
{
  ErrCode_t err;        // error code
  QString msg;          // error message
} ErrorRegistryRecord;
Q_DECLARE_TYPEINFO (ErrorRegistryRecord, Q_MOVABLE_TYPE);

// Error registry
static QVector <ErrorRegistryRecord> ErrorRegistry;

// Register an error
static void
RegisterError (const ErrCode_t code, const QString message)
{
  ErrorRegistryRecord rec;
  rec.err = code;
  rec.msg = message;
  ErrorRegistry.append (rec);
}

// Fill the error registry
void
fill_error_registry ()
{
  Q_UNUSED (QTACastFromConstVoid)

  RegisterError (CGSERR_OK, CGLiteral (" No error "));
  RegisterError (CGSERR_INITIALIZATION_FAILED, CGLiteral (" Initialization failed "));
  RegisterError (CGSERR_RUNTIME_ERROR, CGLiteral (" Run-time error "));
  RegisterError (CGSERR_UNINITIALIZED, CGLiteral (" Uninitialized module "));
  RegisterError (CGSERR_ALREADY_INITIALIZED, CGLiteral (" Module is initialized "));
  RegisterError (CGSERR_NO_PARENT_CHART, CGLiteral (" No parent chart"));
  RegisterError (CGSERR_NO_MODULE, CGLiteral (" No module "));
  RegisterError (CGSERR_NO_ARRAY, CGLiteral (" No array "));
  RegisterError (CGSERR_ARRAY_CREATION_AFTER_INIT, CGLiteral (" Array creation outside Init() "));
  RegisterError (CGSERR_ARRAY_ELEM_BIG, CGLiteral (" Array element too big "));
  RegisterError (CGSERR_ARRAY_ELEM_ZERO, CGLiteral (" Array element size is zero "));
  RegisterError (CGSERR_ARRAY_CREATION_FAILED, CGLiteral (" Array creation failed "));
  RegisterError (CGSERR_ARRAY_SUBSCRIPT, CGLiteral (" Array subscript out of range "));
  RegisterError (CGSERR_MEMORY, CGLiteral (" Out of memory "));
  RegisterError (CGSERR_TIMEFRAME, CGLiteral (" Invalid time frame "));
  RegisterError (CGSERR_INVALID_ARGUMENT, CGLiteral (" Invalid argument "));
  RegisterError (CGSERR_NEGATIVE_SHIFT, CGLiteral (" Negative shift "));
  RegisterError (CGSERR_INVALID_MODTYPE, CGLiteral (" Invalid module type "));
  RegisterError (CGSERR_INTERNAL_100, CGLiteral (" Internal error 100 ")); // function not found in registry
  RegisterError (CGSERR_INTERNAL_101, CGLiteral (" Internal error 101 ")); // input variable creation failed
  RegisterError (CGSERR_FPE, CGLiteral (" Floating point exception "));
  RegisterError (CGSERR_SEGV, CGLiteral (" Segmentation fault "));
  RegisterError (CGSERR_ILL, CGLiteral (" Illegal instruction "));
  RegisterError (CGSERR_SIGNAL, CGLiteral (" Signal not handled "));
}

// StrError2: Return string describing error number
extern "C" Q_DECL_EXPORT const char *
StrError2_imp (const ObjectHandler_t objptr, ErrCode_t code)
{
  const String_t Serrormsg =
    StrInit2_imp (objptr, "ErrorMessage", " Unknown error ");

  const int maxrec = ErrorRegistry.size ();
  int counter = 0;

  while (counter < maxrec)
  {
    if (ErrorRegistry.at (counter).err == code)
    {
      Cstr2Str_imp (objptr, Serrormsg, ErrorRegistry.at (counter).msg.toStdString().c_str());
      counter = maxrec;
    }
    else
      counter ++;
  }

  return Str2Cstr_imp (Serrormsg);
}
