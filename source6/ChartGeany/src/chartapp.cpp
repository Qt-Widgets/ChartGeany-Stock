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

#include "chartapp.h"

ChartApp::ChartApp (int & argc, char **argv): QApplication(argc, argv)
{
  modmutex = new QMutex;
  iomutex  = new QMutex;
}

ChartApp::~ChartApp ()
{
  delete modmutex;
  delete iomutex;
}

void
ChartApp::moduleLock (QObject *obj)
{
  modmutex->lock ();
  lockholder = obj;
}

void
ChartApp::moduleUnlock (QObject *obj)
{
  Q_UNUSED (obj)

  modmutex->unlock ();

  lockholder = nullptr;
}

void
ChartApp::ioLock ()
{
  iomutex->lock ();
}

void
ChartApp::ioUnlock ()
{
  iomutex->unlock ();
}

bool
ChartApp::ioTrylock ()
{
  return iomutex->tryLock (20);
}
