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

// Qt, third party and system headers
#include <math.h>
#include <limits>
#include <QtGlobal>
#include <QMutex>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QMainWindow>

#include "defs.h"
#include "cgliteral.h"
#include "debug.h"

#define underscore CGLiteral ("_")
#define comma      CGLiteral (",")
#define dash       CGLiteral ("-")
#define dot        CGLiteral (".")

// extern classes
#include "loadcsvdialog.h"
#include "datamanagerdialog.h"
#include "modulemanagerdialog.h"
#include "debugdialog.h"
#include "templatemanagerdialog.h"
#include "downloaddatadialog.h"
#include "progressdialog.h"
#include "waitdialog.h"
#include "qtachart.h"
#include "netservice.h"
#include "feedyahoo.h"
#include "feediex.h"
#include "feedav.h"
#include "feedtd.h"

// extern variables
extern QProgressBar *GlobalProgressBar;         // progress bar proxy
extern QTAChart *chartwidget;                   // widget of the chart
extern LoadCSVDialog *loadcsvdialog;            // dialog to load CSV file
extern DownloadDataDialog *downloaddatadialog;  // dialog to download data
extern ProgressDialog *progressdialog;          // dialog to show progress
extern DebugDialog *debugdialog;                // debug console
extern TemplateManagerDialog *templatemanager;  // template manager dialog
extern NetService *MasterNetService;			// master NetService
extern YahooFeed *Yahoo;						// Yahoo feed
extern IEXFeed *IEX;							// IEX feed
extern AlphaVantageFeed *AlphaVantage;			// AlphaVantage feed
extern TwelveDataFeed *TwelveData;				// TwelveData feed
extern QMutex *ResourceMutex;                   // mutex to protect shared resources
extern SQLists *ComboItems;             // QStringLists used as combo box items and more
extern QString installationPath;        // the path ChartGeany binary is installed
extern QStringList UserAgents;          // Http user agents
extern int NCORES;                      // number of active cores
extern const char DEFAULT_FONT_FAMILY[];
extern const int  FONT_POINTSIZE_PAD;
extern const int  FONT_PIXELSIZE_PAD;
extern const int  CHART_FONT_SIZE_PAD;
extern bool showlicense;
extern bool WinStore;
extern "C" void * CGScriptFunctionRegistry_ptr ();
extern size_t CGScriptFunctionRegistrySize;

// extern functions

// normalize table names
void
tablename_normal (QString & tname);

// database manager
CG_ERR_RESULT
dbman (int dbversion, AppSettings appsettings);

// show a message box
extern void
showMessage (QString message);

// show an Ok/Cancel question box
extern bool
showOkCancel (QString message);

// show download message box
extern bool
showDownloadMessage (void);

// delay nsecs
extern void
delay(int secs);

// insert or update rows in database tables
extern int
updatedb (const QString &SQL);

extern int
updatedb (const QString &SQL, bool trylock);

// select from database
extern int
selectfromdb (const char *sql, int (*callback)(void*,int,char**,char**), void *arg1);

// select count (*) query. returns the counter or -1 on error
extern int
selectcount (const QString &SQL);

// returns description of an error code
extern QString
errorMessage (CG_ERR_RESULT err);

// returns full operating system description
extern QString
fullOperatingSystemVersion (void);

// sqlite3_exec callback for retrieving supported data formats
extern int
sqlcb_formats(void *dummy, int argc, char **argv, char **column);

// sqlite3_exec callback for retrieving supported timeframes
extern int
sqlcb_timeframes(void *dummy, int argc, char **argv, char **column);

// sqlite3_exec callback for retrieving supported currencies
extern int
sqlcb_currencies(void *dummy, int argc, char **argv, char **column);

// sqlite3_exec callback for retrieving markets
extern int
sqlcb_markets(void *dummy, int argc, char **argv, char **column);

// sqlite3_exec callback for retrieving datafeeds
extern int
sqlcb_datafeeds(void *dummy, int argc, char **argv, char **column);

// sqlite3_exec callback for retrieving symbol's data frames
extern int
sqlcb_dataframes (void *vectorptr, int argc, char **argv, char **column);

// sqlite3_exec callback for retrieving symbol's fundamental data
extern int
sqlcb_fundamentals (void *data, int argc, char **argv, char **column);

// sqlite3_exec callback for retrieving nsymbols
extern int
sqlcb_nsymbols(void *nsymptr, int argc, char **argv, char **column);

// sqlite3_exec callback for ticker symbols
extern int
sqlcb_tickersymbols (void *data, int argc, char **argv, char **column);

// sqlite3_exec callback for ticker feed
extern int
sqlcb_tickerfeed (void *data, int argc, char **argv, char **column);

// sqlite3_exec callback for transaction types
extern int
sqlcb_transactiontypes (void *data, int argc, char **argv, char **column);

// sqlite3_exec callback for commission types
extern int
sqlcb_commissiontypes (void *data, int argc, char **argv, char **column);

// correct font size for widget and children
static inline void
correctFontSize (QWidget *widget)
{
  QFont fnt;

  fnt = widget->font ();
  fnt.setFamily (DEFAULT_FONT_FAMILY);
  if (fnt.pointSize () != -1)
    fnt.setPointSize (fnt.pointSize () + FONT_POINTSIZE_PAD);
  else
    fnt.setPixelSize (fnt.pixelSize () + FONT_PIXELSIZE_PAD);

  widget->setFont (fnt);
}

template <typename T> 
void
correctWidgetFonts (T *widget)
{
  QList<QWidget *> allWidgets = widget->template findChildren<QWidget *> ();

  foreach (QWidget *wid, allWidgets)
    correctFontSize (wid);

  correctFontSize (widget);	
};

// correct the fonts of a button in a button box
extern void
correctButtonBoxFonts (QDialogButtonBox *box,
                       QDialogButtonBox::StandardButton button);

// corect title bar of QDialog
extern void
correctTitleBar (QDialog *dialog);

extern void
correctTitleBar (QMainWindow *window);

// native http header
extern QString
nativeHttpHeader (void);

// random http header
extern QString
httpHeader (void);

// json parse
extern bool
json_parse (QString jsonstr, QStringList *node, QStringList *value, void *n1);

// object's family tree of descendants
extern QObjectList
familyTree (QObject *obj);

// reset database
extern void
resetDatabase ();

// update price table
extern void
updatePrice (RTPrice &rtprice);

// roundf(3) for Windows
#ifdef Q_OS_WIN32
inline float
roundf(float x)
{
  return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}
#endif // Q_OS_WIN32

// returns the number of decimal digits
inline GNUPURE int
fracdig (const qreal r) Q_DECL_NOEXCEPT 
{
  return (r >= 1.0) ? 2 : 4;
}

// convert null terminated string to upper case
static Q_DECL_CONSTEXPR char *
cg_chartoupper (char *c)
{
  return (*c >= 'a' && *c <= 'z' ) ? &(c[0]=(*c - 32)) : c;
}

static Q_DECL_CONSTEXPR char *
cg_strtoupper (char *str, std::size_t l=0)
{
  return (*str != 0) ? 
         cg_strtoupper (&(str[0] = *cg_chartoupper(&str[0])) + 1, 
                                                  ((l==0) ? 0 : l) + 1) : 
         str - ((l==0) ? 0 : l);
}

// convert null terminated string to lower case
static Q_DECL_CONSTEXPR char *
cg_chartolower (char *c)
{
  return (*c >= 'A' && *c <= 'Z' ) ? &(c[0]=(*c + 32)) : c;
}

static Q_DECL_CONSTEXPR char *
cg_strtolower (char *str, std::size_t l=0)
{
  return (*str != 0) ? 
         cg_strtolower (&(str[0] = *cg_chartolower (&str[0])) + 1, 
                                                    ((l==0) ? 0 : l) + 1) : 
         str - ((l==0) ? 0 : l);
}		

// override cursor
extern void
appSetOverrideCursor (const QWidget *widget, const QCursor & cursor);

// restore overrided cursor
extern void
appRestoreOverrideCursor (const QWidget *widget);

// available geometry
extern const QRect
availableGeometry (int screen);

// create portfolio views
extern QString
createportfolioviews (QString viewname);

// platform string (eg linux64-gcc)
extern QString
platformString (void);

#if defined (Q_OS_LINUX) || defined (Q_OS_MAC)
// install signal handlers
extern void
install_signal_handlers ();

// get last signal
extern int
get_last_signal ();
#endif // Q_OS_LINUX || Q_OS_MAC

