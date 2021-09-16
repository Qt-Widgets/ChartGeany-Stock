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

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <clocale>
#include <QDesktopServices>
#include <QUrl>
#include <QtGlobal>
#include <QApplication>
#include <QtCore/qmath.h>
#include <QSysInfo>
#include <QTime>
#include <QIcon>
#include <QMessageBox>
#include <QTemporaryFile>
#include <QTextStream>
#include <QPushButton>
#include <QObjectList>
#include <QCursor>

#include "chartapp.h"
#include "common.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include "json.hpp"

using json = nlohmann::json;
using namespace std;
#endif

#ifdef Q_OS_MAC
#include <CoreServices/CoreServices.h>
#endif

#if defined (Q_OS_LINUX) || defined (Q_OS_SOLARIS)
#include <sys/utsname.h>
#endif // Q_OS_LINUX

// normalize table names
void
tablename_normal (QString & tname)
{
  QStringList oldc, substc;

  oldc << CGLiteral (".") << CGLiteral ("=") << CGLiteral ("/")
       << CGLiteral ("!") << CGLiteral ("@") << CGLiteral ("#")
       << CGLiteral ("$") << CGLiteral ("%") << CGLiteral ("^")
       << CGLiteral ("&") << CGLiteral ("*") << CGLiteral ("(")
       << CGLiteral (")") << CGLiteral ("+") << CGLiteral ("-")
       << CGLiteral (",") << CGLiteral (" ") << CGLiteral (":")
       << CGLiteral ("[") << CGLiteral ("]") << CGLiteral ("{")
       << CGLiteral ("}") << CGLiteral ("|") << CGLiteral ("<")
       << CGLiteral (">");

  substc << CGLiteral ("_") << CGLiteral ("a") << CGLiteral ("b")
         << CGLiteral ("c") << CGLiteral ("d") << CGLiteral ("e")
         << CGLiteral ("f") << CGLiteral ("g") << CGLiteral ("h")
         << CGLiteral ("i") << CGLiteral ("j") << CGLiteral ("k")
         << CGLiteral ("l") << CGLiteral ("m") << CGLiteral ("n")
         << CGLiteral ("o") << CGLiteral ("_") << CGLiteral ("q")
         << CGLiteral ("r") << CGLiteral ("s") << CGLiteral ("t")
         << CGLiteral ("u") << CGLiteral ("v") << CGLiteral ("w")
         << CGLiteral ("x") << CGLiteral ("y") << CGLiteral ("z");

  for (qint32 counter = 0, maxcounter = oldc.size ();
       counter < maxcounter; counter ++)
    tname.replace (oldc[counter], substc[counter]);

  if (tname.indexOf(QRegExp("[0-9]"), 0) == 0)
    tname.insert(0, underscore);

  tname = tname.toUpper ();
}

// delay nsecs
void
delay(int secs)
{
  QTime dieTime= QTime::currentTime().addSecs(secs);
  while( QTime::currentTime() < dieTime )
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

// insert or update database
int
updatedb (const QString &SQL, bool trylock)
{
  static int updcounter = 0;
  updcounter += (updcounter == 1000 ? -1000 : 1);

  const QString sql =
    CGLiteral ("BEGIN;") % SQL % CGLiteral ("COMMIT;") %
    (updcounter == 0 ?
     CGLiteral ("PRAGMA shrink_memory;") :
     CGLiteral (""));

  setGlobalError(CG_ERR_OK, __FILE__, __LINE__);
  if (!trylock)
    (qobject_cast <ChartApp *> (qApp))->ioLock ();
  else if ((qobject_cast <ChartApp *> (qApp))->ioTrylock () == false)
    return SQLITE_OK;

  char *errmsg = nullptr;
  int rc = sqlite3_exec(Application_Settings->db, sql.toUtf8(),
                        nullptr, nullptr, &errmsg);

  if (errmsg != nullptr)
    sqlite3_free(errmsg);

  if (rc != SQLITE_OK)
    resetDatabase ();

  (qobject_cast <ChartApp *> (qApp))->ioUnlock ();

  return rc;
}

int
updatedb (const QString &SQL)
{
  return updatedb (SQL, false);
}

// show message box
void
showMessage (QString message)
{
  QMessageBox *msgBox;
  QFont fnt;

  msgBox = new QMessageBox;
  appRestoreOverrideCursor (msgBox);
  fnt = msgBox->font ();
  fnt.setPixelSize (14);
  fnt.setFamily (DEFAULT_FONT_FAMILY);
  fnt.setWeight (QFont::DemiBold);

  msgBox->setWindowTitle (CGLiteral ("Message"));
  msgBox->setWindowIcon (QIcon (QString (":/png/images/icons/PNG/cglogo.png")));
  msgBox->setFont (fnt);
  msgBox->setIcon (QMessageBox::Information);
  msgBox->setText(message % CGLiteral ("           "));
  msgBox->setStandardButtons(QMessageBox::Close);
  msgBox->setDefaultButton(QMessageBox::Close);
  msgBox->setStyleSheet (CGLiteral ("background: transparent; background-color:white;"));
  correctWidgetFonts (msgBox);
  msgBox->exec ();

  delete msgBox;
}

// show download message box
bool
showDownloadMessage ()
{
  QMessageBox *msgBox;
  QPushButton *downloadBtn, *closeBtn;
  QFont fnt;
  bool result = false;

  msgBox = new QMessageBox;
  fnt = msgBox->font ();
  fnt.setPixelSize (14);
  fnt.setFamily (DEFAULT_FONT_FAMILY);
  fnt.setWeight (QFont::DemiBold);

  msgBox->setWindowTitle (CGLiteral ("Download new version"));
  msgBox->setWindowIcon (QIcon (QString (":/png/images/icons/PNG/cglogo.png")));
  msgBox->setFont (fnt);
  msgBox->setIcon (QMessageBox::Warning);
  msgBox->setText(CGLiteral ("There is a new version available for download"));
  downloadBtn = msgBox->addButton ("Download", QMessageBox::AcceptRole);
  closeBtn = msgBox->addButton (CGLiteral ("Close"), QMessageBox::RejectRole);
  msgBox->setStyleSheet (CGLiteral ("background: transparent; background-color:white;"));
  correctWidgetFonts (msgBox);
  msgBox->exec ();
  if (msgBox->clickedButton() == static_cast <QAbstractButton *> (downloadBtn))
    result = true;
  else if (msgBox->clickedButton() ==  static_cast <QAbstractButton *>  (closeBtn))
    result = false;

  delete msgBox;
  return result;
}

// show Ok/Cancel message box
bool
showOkCancel (QString message)
{
  QMessageBox *msgBox;
  QFont fnt;
  bool result = false;

  msgBox = new QMessageBox;
  appRestoreOverrideCursor (msgBox);
  fnt = msgBox->font ();
  fnt.setPixelSize (14);
  fnt.setFamily (DEFAULT_FONT_FAMILY);
  fnt.setWeight (QFont::DemiBold);

  msgBox->setWindowTitle (CGLiteral ("Question"));
  msgBox->setWindowIcon (QIcon (QString (":/png/images/icons/PNG/cglogo.png")));
  msgBox->setFont (fnt);
  msgBox->setIcon (QMessageBox::Question);
  msgBox->setText(message);
  msgBox->setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
  msgBox->setDefaultButton(QMessageBox::Cancel);
  msgBox->setStyleSheet (CGLiteral ("background: transparent; background-color:white;"));
  correctWidgetFonts (msgBox);
  msgBox->exec ();

  if (msgBox->clickedButton() == msgBox->button (QMessageBox::Ok))
    result = true;

  delete msgBox;
  return result;
}

// error messages
QString
errorMessage (CG_ERR_RESULT err)
{
  QStringList ErrorMessage;
  ErrorMessage  <<
                CGLiteral ("No error") <<
                CGLiteral ("Cannot open file") <<
                CGLiteral ("Cannot create temporary table") <<
                CGLiteral ("Cannot create table") <<
                CGLiteral ("Cannot insert data") <<
                CGLiteral ("Cannot delete data") <<
                CGLiteral ("Cannot access database") <<
                CGLiteral ("Invalid reply or network error") <<
                CGLiteral ("Cannot create temporary file") <<
                CGLiteral ("Cannot write to file") <<
                CGLiteral ("Transaction error") <<
                CGLiteral ("Not enough memory") <<
                CGLiteral ("Symbol does not exist") <<
                CGLiteral ("Cannot access data") <<
                CGLiteral ("Network timeout") <<
                CGLiteral ("Invalid data") <<
                CGLiteral ("Request pending") <<
                CGLiteral ("Buffer not found") <<
                CGLiteral ("No quotes for symbol") <<
                CGLiteral ("Operation failed") <<
                CGLiteral ("Compiler not found") <<
                CGLiteral ("Compilation failed") <<
                CGLiteral ("No data") <<
                CGLiteral ("No api key") <<
                CGLiteral ("Invalid object type") <<
                CGLiteral ("Operation aborted");

  GlobalError = CG_ERR_OK;

  return ErrorMessage[err];
}

// full operating system description
extern QString
fullOperatingSystemVersion ()
{
  QString ver = "", desc = "";

#ifdef Q_OS_WIN32
  const QString os = CGLiteral ("Microsoft Windows ");
  ver = CGLiteral (">10 ");
  switch (QSysInfo::WindowsVersion)
  {
  case QSysInfo::WV_NT:
    ver = CGLiteral ("NT ");
    break;
  case QSysInfo::WV_2000:
    ver = CGLiteral ("2000 ");
    break;
  case QSysInfo::WV_XP:
    ver = CGLiteral ("XP ");
    break;
  case QSysInfo::WV_2003:
    ver = CGLiteral ("2003 ");
    break;
  case QSysInfo::WV_VISTA:
    ver = CGLiteral ("Vista ");
    break;
  case QSysInfo::WV_WINDOWS7:
    ver = CGLiteral ("7 ");
    break;
  case QSysInfo::WV_WINDOWS8:
    ver = CGLiteral ("8 ");
    break;
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
  case QSysInfo::WV_WINDOWS8_1:
    ver = CGLiteral ("8.1 ");
    break;
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
  case QSysInfo::WV_WINDOWS10:
    ver = CGLiteral ("10 ");
    break;
#endif
  }
#endif // Q_OS_WIN32

#ifdef Q_OS_MAC
  SInt32 majorVersion,minorVersion,bugFixVersion;

  const QString os = CGLiteral ("Mac OS X ");
  switch (QSysInfo::MacintoshVersion)
  {
  case QSysInfo::MV_CHEETAH:
    ver = CGLiteral ("10.0 Cheetah ");
    break;
  case QSysInfo::MV_PUMA:
    ver = CGLiteral ("10.1 Puma ");
    break;
  case QSysInfo::MV_JAGUAR:
    ver = CGLiteral ("10.2 Jaguar ");
    break;
  case QSysInfo::MV_PANTHER:
    ver = CGLiteral ("10.3 Panther ");
    break;
  case QSysInfo::MV_TIGER:
    ver = CGLiteral ("10.4 Tiger ");
    break;
  case QSysInfo::MV_LEOPARD:
    ver = CGLiteral ("10.5 Leopard ");
    break;
  case QSysInfo::MV_SNOWLEOPARD:
    ver = CGLiteral ("10.6 Snow Leopard ");
    break;
  case QSysInfo::MV_LION:
    ver = CGLiteral ("10.7 Lion ");
    break;
  case QSysInfo::MV_MOUNTAINLION:
    ver = CGLiteral ("10.8 Mountain Lion ");
    break;
  default:
    Gestalt(gestaltSystemVersionMajor, &majorVersion);
    Gestalt(gestaltSystemVersionMinor, &minorVersion);
    Gestalt(gestaltSystemVersionBugFix, &bugFixVersion);
    ver = QString::number ((int) majorVersion) % CGLiteral (".") %
          QString::number ((int) minorVersion) % CGLiteral (" ");
  }
#endif // Q_OS_MAC

#if defined (Q_OS_LINUX) || defined (Q_OS_SOLARIS)
  struct utsname unixname;

  const QString os = (uname (&unixname) >= 0) ? QString (unixname.sysname) %
                     QStringLiteral (" ") :
                     QStringLiteral (" ");

  ver = (uname (&unixname) >= 0) ? QString (unixname.release) %
        QStringLiteral (" ") :
        QStringLiteral (" ");
#endif // Q_OS_LINUX

  const QString full = os + ver + desc;
  return full;
}

// set global error
void
setGlobalError (CG_ERR_RESULT err, const char *_file_, int _line_)
{
#ifndef DEBUG
  Q_UNUSED (_file_)
  Q_UNUSED (_line_)
#endif

  if (GlobalError.fetchAndAddAcquire (0) == CG_ERR_OK)
    GlobalError = err;

#ifdef DEBUG
  if (err != CG_ERR_OK)
    qDebug () << errorMessage (err) << _file_ << _line_;
#endif

  return;
}

// formats' callback
int
sqlcb_formats (void *dummy, int argc, char **argv, char **column)
{
  if (dummy != nullptr)
    return 1;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();
    if (colname == QLatin1String ("FORMAT"))
      ComboItems->formatList << QString (argv[counter]);
  }
  return 0;
}

// timeframes' callback
int
sqlcb_timeframes(void *dummy, int argc, char **argv, char **column)
{
  if (dummy != nullptr)
    return 1;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString::fromUtf8(column[counter]).toUpper ();
    if (colname == QLatin1String ("TIMEFRAME"))
      ComboItems->timeframeList << QString (argv[counter]);
  }
  return 0;
}

// currencies' callback
int
sqlcb_currencies(void *dummy, int argc, char **argv, char **column)
{
  if (dummy != nullptr)
    return 1;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();
    if (colname == QLatin1String ("SYMBOL"))
      ComboItems->currencyList << QString (argv[counter]);
  }
  return 0;
}

// currencies' callback
int
sqlcb_markets(void *dummy, int argc, char **argv, char **column)
{
  if (dummy != nullptr)
    return 1;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();
    if (colname == QLatin1String ("MARKET"))
      ComboItems->marketList << QString (argv[counter]);
  }
  return 0;
}

// transaction types callback
int
sqlcb_transactiontypes (void *dummy, int argc, char **argv, char **column)
{
  if (dummy != nullptr)
    return 1;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();
    if (colname == QLatin1String ("DESCRIPTION"))
      ComboItems->transactiontypeList << QString (argv[counter]);
  }
  return 0;
}

// commission types callback
int
sqlcb_commissiontypes (void *dummy, int argc, char **argv, char **column)
{
  if (dummy != nullptr)
    return 1;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();
    if (colname == QLatin1String ("DESCRIPTION"))
      ComboItems->commissiontypeList << QString (argv[counter]);
  }
  return 0;
}

// symbol's data frames callback
int
sqlcb_dataframes (void *vectorptr, int argc, char **argv, char **column)
{
  Q_UNUSED (argc); Q_UNUSED (column);
  	
  if (vectorptr == nullptr)
    return 1;
  
  QTAChartFrame Frame;
  
  Frame.Open  	 = strtod (argv[0], nullptr);
  Frame.High  	 = strtod (argv[1], nullptr);
  Frame.Low   	 = strtod (argv[2], nullptr);
  Frame.Close 	 = strtod (argv[3], nullptr);
  Frame.AdjClose = strtod (argv[4], nullptr);
  Frame.Volume   = strtod (argv[5], nullptr);
  strcpy (Frame.Date, argv[6]);
  strcpy (Frame.Time, argv[7]);
  if (Frame.Volume == 0.0f)
    Frame.Volume = 1.0f;
  
  FrameVector * const VFrame = static_cast <FrameVector *> (vectorptr);
  *VFrame << Frame;
  return 0;
}

// symbol's fundamentals callback
int
sqlcb_fundamentals (void *data, int argc, char **argv, char **column)
{
  QTAChartData *Data;

  Data = static_cast <QTAChartData*> (data);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toLower ();

    if (colname == QLatin1String ("bv"))
      Data->bv = QString (argv[counter]);
    else if (colname == QLatin1String ("mc"))
      Data->mc = QString (argv[counter]);
    else if (colname == QLatin1String ("ebitda"))
      Data->ebitda = QString (argv[counter]);
    else if (colname == QLatin1String ("pe"))
      Data->pe = QString (argv[counter]);
    else if (colname == QLatin1String ("peg"))
      Data->peg = QString (argv[counter]);
    else if (colname == QLatin1String ("dy"))
      Data->dy = QString (argv[counter]);
    else if (colname == QLatin1String ("epscurrent"))
      Data->epscurrent = QString (argv[counter]);
    else if (colname == QLatin1String ("epsnext"))
      Data->epsnext = QString (argv[counter]);
    else if (colname == QLatin1String ("es"))
      Data->esh = QString (argv[counter]);
    else if (colname == QLatin1String ("ps"))
      Data->ps = QString (argv[counter]);
    else if (colname == QLatin1String ("pbv"))
      Data->pbv = QString (argv[counter]);
  }

  return 0;
}

// sqlite3_exec callback for retrieving application options
int
sqlcb_options (void *classptr, int argc, char **argv, char **column)
{
  AppOptions *options = static_cast <AppOptions *> (classptr);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();

    if (colname == QLatin1String ("PAK"))
      options->pak = QString (argv[counter]);
    else if (colname == QLatin1String ("AVAPIKEY"))
      options->avapikey = QString (argv[counter]);
    else if (colname == QLatin1String ("IEXAPIKEY"))
      options->iexapikey = QString (argv[counter]);
    else if (colname == QLatin1String ("TWELVEDATAAPIKEY"))
      options->tdapikey = QString (argv[counter]);
    else if (colname == QLatin1String ("SHOWSPLASHSCREEN"))
    {
      if (atoi (argv[counter]) == 1)
        options->showsplashscreen = true;
      else
        options->showsplashscreen = false;
    }
    else if (colname == QLatin1String ("AUTOUPDATE"))
    {
      if (atoi (argv[counter]) == 1)
        options->autoupdate = true;
      else
        options->autoupdate = false;
    }
    else if (colname == QLatin1String ("DEVMODE"))
    {
      if (atoi (argv[counter]) == 1)
        options->devmode = true;
      else
        options->devmode = false;
    }
    else if (colname == QLatin1String ("CHECKNEWVERSION"))
    {
      if (atoi (argv[counter]))
        options->checknewversion = true;
      else
        options->checknewversion = false;
    }
    else if (colname == QLatin1String ("LONGBP"))
    {
      if (atoi (argv[counter]))
        options->longbp = true;
      else
        options->longbp = false;
    }
    else if (colname == QLatin1String ("ENABLEPROXY"))
    {
      if (atoi (argv[counter]))
        options->enableproxy = true;
      else
        options->enableproxy = false;
    }
    else if (colname == QLatin1String ("PROXYHOST"))
      options->proxyhost = QString (argv[counter]);
    else if (colname == QLatin1String ("PROXYUSER"))
      options->proxyuser = QString (argv[counter]);
    else if (colname == QLatin1String ("PROXYPASS"))
      options->proxypass = QString (argv[counter]);
    else if (colname == QLatin1String ("PROXYPORT"))
      options->proxyport = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("NETTIMEOUT"))
      options->nettimeout = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("SCROLLSPEED"))
      options->scrollspeed = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("VOLUME"))
      options->showvolume = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("LINEAR"))
      options->linear = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("GRID"))
      options->showgrid = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("CHARTSTYLE"))
      options->chartstyle = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("ONLINEPRICE"))
      options->showonlineprice = QString (argv[counter]).toShort ();
    else if (colname == QLatin1String ("LINECOLOR"))
      options->linecolor =
        QColor ((QRgb) QString (argv[counter]).toULongLong ());
    else if (colname == QLatin1String ("BARCOLOR"))
      options->barcolor =
        QColor ((QRgb) QString (argv[counter]).toULongLong ());
    else if (colname == QLatin1String ("FORECOLOR"))
      options->forecolor =
        QColor ((QRgb) QString (argv[counter]).toULongLong ());
    else if (colname == QLatin1String ("BACKCOLOR"))
      options->backcolor =
        QColor ((QRgb) QString (argv[counter]).toULongLong ());
  }
  return 0;
}

// nsymbols callback
int
sqlcb_nsymbols(void *nsymptr, int argc, char **argv, char **column)
{
  Q_UNUSED (argc)
  Q_UNUSED (column)

  int nsymbols;

  nsymbols = QString (argv[0]).toInt ();
  *(int *) nsymptr = nsymbols;

  return 0;
}

// ticker symbols callback
int
sqlcb_tickersymbols (void *data, int argc, char **argv, char **column)
{
  QStringList *symbols, s;
  
  symbols = static_cast <QStringList *> (data);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    const QString colname = QString (column[counter]).toLower ();

    if (colname == QLatin1String ("symbol"))
      s.append (QString (argv[counter]));
  }

  *symbols += s;
  return 0;
}

// ticker feed call back
int
sqlcb_tickerfeed (void *data, int argc, char **argv, char **column)
{
  QStringList *feed;

  feed = static_cast <QStringList *> (data);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    const QString colname = QString (column[counter]).toLower ();

    if (colname == QLatin1String ("feed") || colname == QLatin1String ("datafeed"))
      (*feed).append (QString (argv[counter]));
  }

  return 0;
}

// load ticker symbols
CG_ERR_RESULT
loadTickerSymbols (QStringList & symbol, QStringList & feed)
{
  QString query = CGLiteral ("SELECT SYMBOL FROM TICKER_SYMBOLS ORDER BY SYMBOL;");
  int rc = selectfromdb(query.toUtf8(), sqlcb_tickersymbols,
                    static_cast <void *> (&symbol));
  if (rc == SQLITE_OK)
  {
    query = CGLiteral ("SELECT FEED FROM TICKER_SYMBOLS ORDER BY SYMBOL;");
    rc = selectfromdb(query.toUtf8(), sqlcb_tickerfeed,
                      static_cast <void *> (&feed));
    if (rc != SQLITE_OK)
    {
      symbol.clear ();
      return CG_ERR_DBACCESS;
    }
  }
  else
    return CG_ERR_DBACCESS;

  return CG_ERR_OK;
}

// load portfolio symbols
CG_ERR_RESULT
loadPortfolioSymbols (QStringList &symbol, QStringList & feed, int pfid)
{
  QStringList wsymbol, wfeed;

  const QString table = CGLiteral ("pftrans_") % QString::number (pfid) % CGLiteral ("summary");
  QString query = CGLiteral ("SELECT SYMBOL FROM ") % table % CGLiteral (" WHERE QUANTITY <> 0 ORDER BY SYMBOL;");
  int rc = selectfromdb(query.toUtf8(), sqlcb_tickersymbols,
                    static_cast <void *> (&wsymbol));
  if (rc == SQLITE_OK)
  {
    query += CGLiteral ("SELECT DATAFEED FROM ") % table % CGLiteral (" WHERE QUANTITY <> 0 ORDER BY SYMBOL;");
    rc = selectfromdb(query.toUtf8(), sqlcb_tickerfeed,
                      static_cast <void *> (&wfeed));
    if (rc != SQLITE_OK)
    {
      symbol.clear ();
      return CG_ERR_DBACCESS;
    }
  }
  else
    return CG_ERR_DBACCESS;

  symbol = wsymbol;
  feed = wfeed;

  return CG_ERR_OK;
}

// load ticker symbols
CG_ERR_RESULT
saveTickerSymbols (QStringList & symbol, QStringList & feed)
{
  QString query;
  int rc;

  query = CGLiteral ("DELETE FROM TICKER_SYMBOLS;");
  for (qint32 counter = 0; counter < symbol.size (); counter ++)
  {
    query += CGLiteral ("INSERT INTO TICKER_SYMBOLS (SYMBOL, FEED) VALUES ('") %
             symbol[counter] % CGLiteral ("','") % feed[counter] % CGLiteral ("');");
  }
  query.append ('\n');

  rc = updatedb (query);
  if (rc != SQLITE_OK)
    return CG_ERR_DBACCESS;

  return CG_ERR_OK;
}

// load application options
CG_ERR_RESULT
loadAppOptions (AppOptions *options)
{
  QString query, platform = platformString ();
  int rc;

  if (options->platform.size () > 0)
    platform = options->platform;

  query = CGLiteral ("SELECT * FROM toolchains WHERE platform = '") %
          platform % CGLiteral ("';");
  rc = selectfromdb(query.toUtf8(), sqlcb_toolchain, options);
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    return CG_ERR_DBACCESS;
  }

  query = CGLiteral ("SELECT * FROM options WHERE recid = 1;");
  rc = selectfromdb(query.toUtf8(), sqlcb_options, options);
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    return CG_ERR_DBACCESS;
  }

  return CG_ERR_OK;
}

// save application options
CG_ERR_RESULT
saveAppOptions (AppOptions *options)
{
  QString query;
  int rc;

  query = CGLiteral ("UPDATE options SET ");
  query.append ('\n');
  query += CGLiteral ("pak = '") % options->pak + CGLiteral ("'");
  query += CGLiteral (", avapikey = '") % options->avapikey + CGLiteral ("'");
  query += CGLiteral (", iexapikey = '") % options->iexapikey + CGLiteral ("'");
  query += CGLiteral (", twelvedataapikey = '") % options->tdapikey + CGLiteral ("'");

  if (options->showsplashscreen)
    query += CGLiteral (", showsplashscreen = 1 ");
  else
    query += CGLiteral (", showsplashscreen = 0 ");

  if (options->autoupdate)
    query += CGLiteral (", autoupdate = 1 ");
  else
    query += CGLiteral (", autoupdate = 0 ");

  if (options->devmode)
    query += CGLiteral (", devmode = 1 ");
  else
    query += CGLiteral (", devmode = 0 ");

  if (options->checknewversion)
    query += CGLiteral (", checknewversion = 1 ");
  else
    query += CGLiteral (", checknewversion = 0 ");

  if (options->enableproxy)
    query += CGLiteral (", enableproxy = 1 ");
  else
    query += CGLiteral (", enableproxy = 0 ");

  if (options->longbp)
    query += CGLiteral (", longbp = 1 ");
  else
    query += CGLiteral (", longbp = 0 ");

  query += CGLiteral (", proxyhost = '") % options->proxyhost % CGLiteral ("'") %
           CGLiteral (", proxyuser = '") % options->proxyuser % CGLiteral ("'") %
           CGLiteral (", proxypass = '") % options->proxypass % CGLiteral ("'");
  query.append ('\n');
  query += CGLiteral (", proxyport = ") % QString::number (options->proxyport) %
           CGLiteral (", nettimeout = ") % QString::number (options->nettimeout) %
           CGLiteral (", scrollspeed = ") % QString::number (options->scrollspeed) %
           CGLiteral (", linecolor = ") % QString::number ((qreal) options->linecolor.rgb (), 'f', 0) %
           CGLiteral (", barcolor = ") % QString::number ((qreal) options->barcolor.rgb (), 'f', 0) %
           CGLiteral (", backcolor = ") % QString::number ((qreal) options->backcolor.rgb (), 'f', 0) %
           CGLiteral (", forecolor = ") % QString::number ((qreal) options->forecolor.rgb (), 'f', 0);

  if (options->showvolume)
    query += CGLiteral (", volume = 1");
  else
    query += CGLiteral (", volume = 0");

  if (options->linear)
    query += CGLiteral (", linear = 1");
  else
    query += CGLiteral (", linear = 0");

  query += CGLiteral (", chartstyle = ") % QString::number (options->chartstyle, 'f', 0);

  if (options->showonlineprice)
    query += CGLiteral (", onlineprice = 1");
  else
    query += CGLiteral (", onlineprice = 0");

  if (options->showgrid)
    query += CGLiteral (", grid = 1");
  else
    query += CGLiteral (", grid = 0");

  query += CGLiteral (" WHERE recid = 1;");

  query.append ('\n');
  query += CGLiteral ("UPDATE toolchains SET ") %
           CGLiteral ("compiler = '") % options->compiler % CGLiteral ("', ") %
           CGLiteral ("compilerdbg = '") % options->compilerdbg % CGLiteral ("', ") %
           CGLiteral ("compilerrel = '") % options->compilerrel % CGLiteral ("', ") %
           CGLiteral ("linker = '") % options->linker % CGLiteral ("', ") %
           CGLiteral ("linkerdbg = '") % options->linkerdbg % CGLiteral ("', ") %
           CGLiteral ("linkerrel = '") % options->linkerrel % CGLiteral ("' ") %
           CGLiteral ("WHERE platform = '") % options->platform % CGLiteral ("';");

  rc = updatedb (query);
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    return CG_ERR_DBACCESS;
  }

  return CG_ERR_OK;
}

// correct the fonts of a button in a button box
void
correctButtonBoxFonts (QDialogButtonBox *box,
                       QDialogButtonBox::StandardButton button)
{
  QFont font;
  QPushButton *btn;

  btn = box->button (button);
  btn->setFocusPolicy (Qt::NoFocus);
  // correctFontSize (btn);
  font = btn->font ();
  font.setWeight (QFont::DemiBold);
  btn->setFont (font);

  return;
}

// corect title bar of QDialog
void
correctTitleBar (QDialog *dialog)
{
  Qt::WindowFlags flags = dialog->windowFlags();
  Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & (~helpFlag);
  dialog->setWindowFlags(flags);
  dialog->setWindowFlags (((dialog->windowFlags() | Qt::CustomizeWindowHint)
                           & ~Qt::WindowCloseButtonHint));
}

void
correctTitleBar (QMainWindow *window)
{
  Qt::WindowFlags flags = window->windowFlags();
  Qt::WindowFlags helpFlag = Qt::WindowContextHelpButtonHint;
  flags = flags & (~helpFlag);
  window->setWindowFlags(flags);
  window->setWindowFlags (((window->windowFlags() | Qt::CustomizeWindowHint)
                           & ~Qt::WindowCloseButtonHint));
}

// native http header
QString
nativeHttpHeader ()
{
  QString header;

  header.reserve (4096);
  header = QString (APPNAME) % CGLiteral ("/") %
           QString::number (VERSION_MAJOR) % CGLiteral (".") %
           QString::number (VERSION_MINOR) % CGLiteral (".") %
           QString::number (VERSION_PATCH) % CGLiteral (" ") %
           fullOperatingSystemVersion () % CGLiteral (" ") %
           QString::number (QT_POINTER_SIZE*8) % CGLiteral (" ") %
           RunCounter % CGLiteral (" ") % UID % CGLiteral (" ") %
           Application_Settings->options.pak % CGLiteral (" ");

  return header;
}

// random http header
QString
httpHeader ()
{
  QTime time = QTime::currentTime();

  qsrand((uint)time.msec());
  return UserAgents[qrand() % UserAgents.size ()];
}

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

static bool
json_parse_qt5 (QString jsonstr, QStringList *node, QStringList *value, void *n1)
{
  QJsonObject *n = nullptr;
  bool result = true, allocated = false;

  if (n1 == nullptr)
  {
    QJsonDocument doc;
    QJsonParseError err;

    node->clear ();
    value->clear ();

    jsonstr = jsonstr.trimmed ();
    doc = QJsonDocument::fromJson(jsonstr.toUtf8(), &err);
    if (err.error !=  QJsonParseError::NoError)
    {
      result = false;
      goto json_parse_qt5_end;
    }

    n = new QJsonObject;
    allocated = true;
    *n = doc.object ();
  }
  else
    n = static_cast <QJsonObject *> (n1);

  for (qint32 counter = 0; counter < (*n).keys ().size (); counter ++)
  {
    if ((*n).value ((*n).keys ().at (counter)).type () == QJsonValue::Array)
    {
      QJsonArray arr = (*n).value ((*n).keys ().at (counter)).toArray ();
      foreach (const QJsonValue it, arr)
      {
        QJsonObject n2 = it.toObject ();
        json_parse_qt5 (jsonstr, node, value, static_cast <void *> (&n2));
      }
    }

    if ((*n).value ((*n).keys ().at (counter)).type () == QJsonValue::Object)
    {
      QJsonObject n2 = (*n).value ((*n).keys ().at (counter)).toObject ();

      if (!json_parse_qt5 (jsonstr, node, value, static_cast <void *> (&n2)))
      {
        result = false;
        goto json_parse_qt5_end;
      }
    }

    node->append ((*n).keys ().at (counter));

    QString nodeval ("");
    if ((*n).value ((*n).keys ().at (counter)).type () == QJsonValue::Double)
    {
      double d = (*n).value ((*n).keys ().at (counter)).toDouble ();
      nodeval = QString("%1").arg(d, 0, 'f', -1).simplified ();
    }
    else
      nodeval = (*n).value ((*n).keys ().at (counter)).toString ();

    value->append (nodeval);
  }

json_parse_qt5_end:
  if (allocated) delete n;
  return result;
}
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)

bool
json_parse (QString jsonstr, QStringList *node, QStringList *value, void *n1)
{
  Q_UNUSED (n1)

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
  return json_parse_qt5 (jsonstr, node, value, nullptr);
#else // QT_VERSION < QT_VERSION_CHECK(5, 0, 0)

  json j;
  try
  {
    j = json::parse(jsonstr.toUtf8());
  }
  catch(json::parse_error)
  {
    return false;
  }

  string cpp_string = j.dump(0);
  QString qjs = QString::fromStdString (cpp_string);
  qjs.replace ("{", "");
  qjs.replace ("}", "");
  qjs.replace ("[", "");
  qjs.replace ("]", "");
  qjs.replace (QChar ('"'), "");
  qjs.replace (",", " ");
  QStringList Nodes = qjs.split(QRegExp("\\n"));
  foreach (QString Node, Nodes)
  {
    Node = Node.trimmed ();
    if (Node.contains (":"))
    {
      QStringList nvpair = Node.split (":");
      node->append (nvpair[0].trimmed ());
      value->append (nvpair[1].trimmed ());
    }
  }

  return true;
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
}

// object's family tree of descendants
QObjectList
familyTree (QObject *obj)
{
  QObjectList list;

  foreach (QObject *object, obj->children ())
  {
    list += object;
    if (object->children ().size () > 0)
      list += familyTree (object);
  }

  return list;
}

// reset the database
void
resetDatabase ()
{
  int rc;

  // close
  sqlite3_close (Application_Settings->db);

  // open sqlite db
  rc = sqlite3_open_v2 (Application_Settings->sqlitefile.toUtf8 (),
                        &Application_Settings->db,
                        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE|
                        SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_FULLMUTEX, nullptr);
  if (rc != SQLITE_OK) // if open failed, quit application
  {
    showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
    sqlite3_close (Application_Settings->db);
    qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
    exit (1);
#else
    std::quick_exit (1);
#endif
  }
  sqlite3_extended_result_codes(Application_Settings->db, 1);

  // execute pragma
  rc = sqlite3_exec(Application_Settings->db, Application_Settings->pragma.toUtf8(), nullptr, nullptr, nullptr);
  if (rc != SQLITE_OK) // if open failed, quit application
  {
    showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
    sqlite3_close (Application_Settings->db);
    qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
    exit (1);
#else
    quick_exit (1);
#endif
  }
}

// update price table
static QMutex updatePriceMutex;
static QElapsedTimer updatePriceTimer;

void
updatePrice (RTPrice &rtprice)
{
  updatePriceMutex.tryLock (100);

  if (!updatePriceTimer.isValid ())
    updatePriceTimer.start ();

  if (rtprice.price.toDouble () <= 0)
    rtprice.price = CGLiteral ("0");

  static QString SQL = CGLiteral ("");
  SQL +=
    CGLiteral ("DELETE FROM prices WHERE symbol = '") % rtprice.symbol %
    CGLiteral ("' AND feed = '") % rtprice.feed % CGLiteral ("'; ") %
    CGLiteral ("INSERT INTO prices (symbol, feed, date, price, volume, time, change, \
          prcchange, timestamp) VALUES ('") %
    rtprice.symbol % CGLiteral ("','") %
    rtprice.feed % CGLiteral ("','") %
    rtprice.date % CGLiteral ("',") %
    rtprice.price % CGLiteral (",'") %
    rtprice.volume % CGLiteral ("','") %
    rtprice.time % CGLiteral ("','") %
    rtprice.change % CGLiteral ("','") %
    rtprice.prcchange % CGLiteral ("', (datetime('now','localtime')));");

  if (updatePriceTimer.hasExpired (9000))
  {
    int rc = updatedb (SQL, true);
    if (rc != CG_ERR_OK)
      setGlobalError (CG_ERR_TRANSACTION, __FILE__, __LINE__);
#ifdef DEBUG
    qDebug () << SQL;
#endif
    SQL = CGLiteral ("");
    updatePriceTimer.restart ();
  }

  updatePriceMutex.unlock ();
}

// override and restore cursor
void
appSetOverrideCursor (const QWidget *wid, const QCursor & cursor)
{
  // ((QWidget *) wid)->setCursor (cursor);
  Q_UNUSED (wid)
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QApplication::setOverrideCursor (cursor);
#else
  QGuiApplication::setOverrideCursor (cursor);
#endif
}

// available geometry
#include <QDesktopWidget>

const QRect
availableGeometry (int screen)
{
#if QT_VERSION < QT_VERSION_CHECK (5, 0, 0)
  return qApp->desktop()->availableGeometry(screen);
#else
  const QList <QScreen *> screens =QGuiApplication::screens ();
  if (screen == -1)
    return screens[0]->availableGeometry();
  else
    return screens[screen]->availableGeometry();
#endif
}

// restore application's cursor
void
appRestoreOverrideCursor (const QWidget *wid)
{
  // ((QWidget *) wid)->setCursor (QCursor(Qt::ArrowCursor));
  Q_UNUSED (wid)
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  QApplication::restoreOverrideCursor ();
#else
  QGuiApplication::restoreOverrideCursor ();
#endif
}

#ifdef Q_CC_MSVC
#include <windows.h>
#include <assert.h>

typedef BOOL (WINAPI *SetProcessDPIAwarePtr)(VOID);

INT APIENTRY DllMain(HMODULE hDLL, DWORD reason, LPVOID reserved)
{
  Q_UNUSED (reserved);
  Q_UNUSED (hDLL);
  if (reason == DLL_PROCESS_ATTACH )
  {
    // Make sure we're not already DPI aware
    assert( !IsProcessDPIAware() );

    // First get the DPIAware function pointer
    SetProcessDPIAwarePtr lpDPIAwarePointer = (SetProcessDPIAwarePtr)
        GetProcAddress(GetModuleHandle((LPCWSTR) "user32.dll"),
                       "SetProcessDPIAware");

    // Next make the page writeable so that we can change the function assembley
    DWORD oldProtect;
    VirtualProtect((LPVOID)lpDPIAwarePointer, 1, PAGE_EXECUTE_READWRITE, &oldProtect);

    // write "ret" as first assembly instruction to avoid actually setting HighDPI
    BYTE newAssembly[] = {0xC3};
    memcpy(lpDPIAwarePointer, newAssembly, sizeof(newAssembly));

    // change protection back to previous setting.
    VirtualProtect((LPVOID)lpDPIAwarePointer, 1, oldProtect, nullptr);
  }
  return TRUE;
}
#endif // Q_CC_MSVC

#include "create_portfolio_views.cpp"
