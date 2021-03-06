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

#include <QProcess>

#include "defs.h"

class Compile : public QObject
{
  Q_OBJECT

public:
  explicit Compile (void);
  ~Compile (void);

  // compile CGScript programs
  CG_ERR_RESULT
    CGSCript (QString programsource, // the source code
              QString &programname,   // program's name
              QString &messages,      // compiler/linker output messages
              bool mode);             // true for debug
private:
  QProcess *process;            // compile and link process
  QString compilestr;           // command line for compiling
  QString linkstr;              // command line for linking
  QString compiletmpfname;      // tempory file name for compiling
  QString linktmpfname;         // tempory file name for linking
  QString output;               // compiler messages
  bool debug;                   // true when debuging
  bool compilestatus;           // status of compilation

private slots:
  void onStdoutAvailable(void);
  void onFinished(int,QProcess::ExitStatus);

};
