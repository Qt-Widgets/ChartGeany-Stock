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

#include "cgtool.h"

int
help ()
{
  out <<
      "ChartGeany command line tool version " <<
      VERSION_MAJOR << "." << VERSION_MINOR << "." << VERSION_PATCH << ENDL <<
       APPCOPYRIGHT << ENDL << ENDL <<
       APPWEBPAGE << ENDL << ENDL <<
      "This program is free software; you can redistribute it and/or modify" << ENDL <<
      "it under the terms of the GNU General Public License as published by" << ENDL <<
      "the Free Software Foundation; either version 2 of the License, or" << ENDL <<
      "(at your option) any later version." << ENDL <<
      ENDL <<
      "This program is distributed in the hope that it will be useful," << ENDL <<
      "but WITHOUT ANY WARRANTY; without even the implied warranty of" << ENDL <<
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the" << ENDL <<
      "GNU General Public License for more details." << ENDL <<
      ENDL <<
      "You should have received a copy of the GNU General Public License" << ENDL <<
      "along with this program. If not, see http://www.gnu.org/licenses/." << ENDL <<
      ENDL <<

      "syntax: " << TOOLNAME << " mode option [PARAMETER] [DBFILE]" << ENDL << ENDL <<
      "mode: data" << ENDL <<
      "options: " << ENDL <<
      "--export TABLEID             table to psv with .csv extension" << ENDL <<
      "--export-tostdout TABLEID    table to stdout" << ENDL <<
      "--list                       list all data tables" << ENDL << ENDL <<
      "mode: help" << ENDL << ENDL <<
      "mode: info" << ENDL <<
      "--cgscript-toolchain TOOLCHAIN   CGScript toolchain information" << ENDL <<
      "--dbfile                         database file information" << ENDL << ENDL <<
      "mode: module" << ENDL <<
      "options:" << ENDL <<
      "--compile FILE               compile from cgs to cgm" << ENDL <<
      "--delete MODULEID            delete module" << ENDL <<
      "--decompile FILE             decompile cgm to cgs" << ENDL <<
      "--export MODULEID            export cgm" << ENDL <<
      "--import FILE                import cgm" << ENDL <<
      "--list                       list all modules" << ENDL <<
      "--verbose-compile FILE       compile from cgs to cgm" << ENDL;

  return 0;
}
