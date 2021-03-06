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

#include <QtGlobal>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QTime>
#include <QDir>
#include <QFontDatabase>
#include <QFileInfo>
#include <QFileDialog>
#include <QTextStream>
#include <QTemporaryFile>
#include <QProgressBar>
#include <QDateTime>
#include <QColor>
#include <QPixmap>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QLayout>
#include <QMainWindow>

#include "netservice.h"
#include "sqlite3.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "splashdialog.h"
#include "qtachart.h"
#include "portfolio.h"
#include "editorwidget.h"
#include "debugdialog.h"
#include "unix_signals.h"
#include "cgscript.h"

const char DEFAULT_FONT_FAMILY[] = "Tahoma";
#ifdef Q_OS_MAC
const int  FONT_POINTSIZE_PAD = 3;
const int  FONT_PIXELSIZE_PAD = 3;
#else
const int  FONT_POINTSIZE_PAD = 1;
const int  FONT_PIXELSIZE_PAD = 1;
#endif
const int  CHART_FONT_SIZE_PAD = 3;

AppSettings *Application_Settings;
SQLists *ComboItems;
LoadCSVDialog *loadcsvdialog;
DownloadDataDialog *downloaddatadialog;
TemplateManagerDialog *templatemanager;
ProgressDialog *progressdialog;
DebugDialog *debugdialog;
NetService *MasterNetService;
YahooFeed *Yahoo;
IEXFeed *IEX;
AlphaVantageFeed *AlphaVantage;
TwelveDataFeed *TwelveData;
QProgressBar *GlobalProgressBar;
static SplashDialog *splash = nullptr;
QString Year, Month, Day, UID, RunCounter;
QStringList UserAgents;
QMutex *ResourceMutex = nullptr;
size_t CGScriptFunctionRegistrySize;

int NCORES;
bool FULL = false;
bool showlicense;

QAtomicInt GlobalError;

// load application fonts
void
MainWindow::loadFonts ()
{
  QFile file;
  QByteArray barray;

  file.setFileName (CGLiteral (":/fonts/fonts/tahoma.ttf"));
  if (file.open(QIODevice::ReadOnly))
  {
    barray.clear ();
    barray = file.readAll ();
    QFontDatabase::addApplicationFontFromData (barray);
    file.close ();
  }

  barray.clear ();
}

// check if there is new version available
void
MainWindow::checkNewVersion ()
{
  QTemporaryFile tempFile;      // temporary file
  QTextStream in;
  QString line, urlstr;
  CG_ERR_RESULT ioresult = CG_ERR_OK;

  newversion = false;

  if (WinStore)
    return;

  urlstr =
#ifdef Q_OS_LINUX
    CGLiteral ("https://chart-geany.sourceforge.io/version/linux/current.txt");
#else
#ifdef Q_OS_WIN
    CGLiteral ("https://chart-geany.sourceforge.io/version/windows/current.txt");
#else
#ifdef Q_OS_MAC
    CGLiteral ("https://chart-geany.sourceforge.io/version/mac/current.txt");
#endif
#ifdef Q_OS_SOLARIS
  CGLiteral ("https://chart-geany.sourceforge.io/version/sunos/current.txt");
#endif
#endif
#endif

  // open temporary file
  if (!tempFile.open ())
    return;

  tempFile.resize (0);

  ioresult = MasterNetService->httpGET (urlstr, tempFile, nullptr);
  if (ioresult != CG_ERR_OK)
    return;

  in.setDevice (&tempFile);
  in.seek (0);
  line = in.readAll ();

  if (line.size () < 1)
    return;

  QStringList versiondigits = line.split (CGLiteral ("."), QString::SkipEmptyParts);
  if (versiondigits.length () == 3)
  {
    if (versiondigits[0].toInt () > VERSION_MAJOR)
    {
      newversion = true;
      return;
    }

    if (versiondigits[0].toInt () < VERSION_MAJOR)
      return;

    if (versiondigits[1].toInt () > VERSION_MINOR)
    {
      newversion = true;
      return;
    }

    if (versiondigits[1].toInt () < VERSION_MINOR)
      return;

    if (versiondigits[2].toInt () > VERSION_PATCH)
    {
      newversion = true;
      return;
    }
  }
}

extern int
sqlcb_dbversion (void *versionptr, int argc, char **argv, char **column);

static int
sqlcb_dbdata (void *dummy, int argc, char **argv, char **column)
{
  QString colname;

  if (dummy != nullptr)
    return 1;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    colname = QString::fromUtf8(column[counter]);
    colname = colname.toUpper ();
    if (colname == QLatin1String ("UID"))
      UID = QString::fromUtf8 (argv[counter]);
    if (colname == QLatin1String ("RUNCOUNTER"))
      RunCounter = QString::fromUtf8 (argv[counter]);
  }

  return 0;
}

// constructor
MainWindow::MainWindow (QWidget * parent):
  QMainWindow (parent), ui (new Ui::MainWindow)
{
  Q_UNUSED (QTACastFromConstVoid)

  const QString stylesheet =
    CGLiteral ("background: transparent; background-color: white; color:black");
  QDateTime datetime;
  QFileInfo dbfile;
  QFile initcopy;
  QString SQLCommand = CGLiteral ("");
  int rc, dbversion;

  sqlitebuff = nullptr;
  showlicense = false;
  GlobalError = CG_ERR_OK;
  ResourceMutex = new QMutex (QMutex::NonRecursive);

  ui->setupUi (this);
  correctTitleBar (this);

  UserAgents
      << CGLiteral ("Mozilla/5.0 (Windows NT 6.2; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/29.0.1547.2 Safari/537.36")
      << CGLiteral ("Mozilla/5.0 (iPad; CPU OS 6_0 like Mac OS X) AppleWebKit/536.26 (KHTML, like Gecko) Version/6.0 Mobile/10A5355d Safari/8536.25")
      << CGLiteral ("Mozilla/5.0 (Windows NT 6.2) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/28.0.1467.0 Safari/537.36")
      << CGLiteral ("Mozilla/5.0 (Windows NT 6.1; WOW64; rv:40.0) Gecko/20100101 Firefox/40.1")
      << CGLiteral ("Mozilla/5.0 (X11; Linux x86_64; rv:17.0) Gecko/20121202 Firefox/17.0 Iceweasel/17.0.1")
      << CGLiteral ("Mozilla/5.0 (Windows NT 6.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/41.0.2228.0 Safari/537.36")
      << CGLiteral ("Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.0) Opera 12.14")
      << CGLiteral ("Mozilla/5.0 (compatible; MSIE 9.0; Windows Phone OS 7.5; Trident/5.0; IEMobile/9.0)")
      << CGLiteral ("Opera/12.0 (Windows NT 5.2;U;en)Presto/22.9.168 Version/12.00")
      << CGLiteral ("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/42.0.2311.135 Safari/537.36 Edge/12.246")
      << CGLiteral ("Mozilla/4.0 (compatible; MSIE 10.0; Windows NT 6.1; Trident/5.0)")
      << CGLiteral ("Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; AS; rv:11.0) like Gecko");

// set the sqlite db path
  appsettings.sqlitefile = QDir::homePath () % QDir::separator()  % CGLiteral (".config") % QDir::separator()  % APPDIR % QDir::separator()  % DBNAME;

// set the sqlite db status
  QFileInfo encstatus;
  QString encstatusname = QDir::homePath () % "/" % ".config" % "/" % APPDIR % "/" % ENCSTATUS;
  encstatus.setFile (encstatusname);

// check db existence
  dbfile.setFile (appsettings.sqlitefile);

  if (dbfile.exists () == true)
  {
    // check if db is encrypted and decrypt it
    if (encstatus.exists () == false)
    {
      WaitDialog *decryptdlg = new WaitDialog;
      decryptdlg->setMessage (QString::fromUtf8 ("Decrypting database. Please wait..."));
      decryptdlg->show ();
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

      SQLCommand  =
        CGLiteral ("PRAGMA key = ") % DBKEY + CGLiteral (";") %
        CGLiteral ("PRAGMA locking_mode = EXCLUSIVE;BEGIN EXCLUSIVE;COMMIT;") %
        CGLiteral ("ATTACH DATABASE '") %
        appsettings.sqlitefile % CGLiteral ("2' AS plaintext KEY '';") %
        CGLiteral ("SELECT sqlcipher_export('plaintext');  DETACH DATABASE plaintext;");

      // open sqlite db
      rc = sqlite3_open_v2 (Application_Settings->sqlitefile.toUtf8 (),
                            &Application_Settings->db,
                            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE|
                            SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_FULLMUTEX, NULL);
      if (rc != SQLITE_OK) // if open failed, quit application
      {
        delete decryptdlg;
        showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
        sqlite3_close (appsettings.db);
        qApp->exit (1);
#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
        exit (1);
#else
        quick_exit (1);
#endif
      }

      // apply decrypt pragmas
      rc = sqlite3_exec(appsettings.db, SQLCommand.toUtf8(), nullptr, this, nullptr);
      if (rc != SQLITE_OK) // if open failed, quit application
      {
        delete decryptdlg;
        showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
        sqlite3_close (appsettings.db);
        qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
        exit (1);
#else
        quick_exit (1);
#endif
      }
      sqlite3_close (appsettings.db);

      // delete old and rename new file
      QFile::remove (appsettings.sqlitefile);
      QFile::rename(appsettings.sqlitefile + "2", appsettings.sqlitefile);

      // create status file
      QFile statfile (encstatusname);
      statfile.open(QIODevice::WriteOnly);
      statfile.close ();

      decryptdlg->hide ();
      showMessage (QString::fromUtf8 ("Decryption completed. Now restart the application."));
      qApp->exit (0);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
      exit (1);
#else
      quick_exit (1);
#endif
    }
  }

  SQLCommand = ' ';
  SQLCommand += CGLiteral (
    "BEGIN EXCLUSIVE;COMMIT;\
     PRAGMA auto_vacuum=2; \
     PRAGMA journal_mode=TRUNCATE; \
     PRAGMA wal_checkpoint(TRUNCATE);");

  appsettings.pragma = SQLCommand;

  ticker = nullptr;
  expandedChartFlag = false;
  tickerVisible = false;
  newversion = false;
  setWindowTitle (QApplication::applicationName ());
  this->setStatusBar ( nullptr );
  this->setStyleSheet ("background-color:white;");
  tabWidget = ui->tabWidget;
  ui->tabWidget->clear ();
  ui->tabWidget->setUsesScrollButtons (true);
  ui->tabWidget->setStyleSheet ("QTabBar::tab { height: 25px;}");
  ui->tabWidget->setDocumentMode (false);

  ui->developButton->setVisible (false);
  ui->debugButton->setVisible (false);
  // ui->modulesButton->setVisible (false);

  ui->managerButton->setStyleSheet (stylesheet);
  ui->screenshotButton->setStyleSheet (stylesheet);
  ui->infoButton->setStyleSheet (stylesheet);
  ui->homeButton->setStyleSheet (stylesheet);
  ui->optionsButton->setStyleSheet (stylesheet);
  ui->tickerButton->setStyleSheet (stylesheet);
  ui->portfolioButton->setStyleSheet (stylesheet);
  ui->exitButton->setStyleSheet (stylesheet);
  ui->developButton->setStyleSheet (stylesheet);
  ui->debugButton->setStyleSheet (stylesheet);
  ui->modulesButton->setStyleSheet (stylesheet);

  // current year, month - 1, day
  datetime = QDateTime::currentDateTimeUtc ();
  Year = QString::number(datetime.date ().year ());
  Month = QString::number(datetime.date ().month () - 1);
  Day = QString::number(datetime.date ().day ());

  // connect to signals
  connect (ui->tabWidget, SIGNAL(tabCloseRequested(int)), this,
           SLOT(closeTab_clicked (int)));
  connect (ui->tabWidget, SIGNAL(currentChanged(int)), this,
           SLOT(currentTab_changed (int)));
  connect (ui->managerButton, SIGNAL (clicked ()), this,
           SLOT (managerButton_clicked ()));
  connect (ui->portfolioButton, SIGNAL (clicked ()), this,
           SLOT (portfolioButton_clicked ()));
  connect (ui->modulesButton, SIGNAL (clicked ()), this,
           SLOT (modulesButton_clicked ()));
  connect (ui->developButton, SIGNAL (clicked ()), this,
           SLOT (developButton_clicked ()));
  connect (ui->screenshotButton, SIGNAL (clicked ()), this,
           SLOT (screenshotButton_clicked ()));
  connect (ui->optionsButton, SIGNAL (clicked ()), this,
           SLOT (optionsButton_clicked ()));
  connect (ui->tickerButton, SIGNAL (clicked ()), this,
           SLOT (tickerButton_clicked ()));
  connect (ui->debugButton, SIGNAL (clicked ()), this,
           SLOT (debugButton_clicked ()));
  connect (ui->homeButton, SIGNAL (clicked ()), this,
           SLOT (homeButton_clicked ()));
  connect (ui->infoButton, SIGNAL (clicked ()), this,
           SLOT (infoButton_clicked ()));
  connect (ui->exitButton, SIGNAL (clicked ()), this,
           SLOT (exitButton_clicked ()));

  // export application settings
  Application_Settings = &appsettings;

  // set static heap size for sqlite: 48M
  sqlitebuff = malloc (1024*1024*48);
  if (sqlitebuff != nullptr)
    sqlite3_config (SQLITE_CONFIG_HEAP, sqlitebuff, 1024*1024*48, 64);

  // enable serialization
  sqlite3_config (SQLITE_CONFIG_SERIALIZED);

  // check db existence and create it if needed
  dbfile.setFile (appsettings.sqlitefile);
  if (dbfile.exists () == false)
  {
    if (!WinStore) showlicense = true;
    if (!QDir (QDir::homePath () % QDir::separator()  % CGLiteral (".config") % QDir::separator()  % APPDIR).exists ())
      QDir ().mkpath (QDir::homePath () % QDir::separator()  % CGLiteral (".config") % QDir::separator()  % APPDIR);

    // open sqlite db
    rc = sqlite3_open_v2 (Application_Settings->sqlitefile.toUtf8 (),
                          &Application_Settings->db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE|
                          SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) // if open failed, quit application
    {
      showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
      sqlite3_close (appsettings.db);
      qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
      exit (1);
#else
      quick_exit (1);
#endif
    }
    sqlite3_extended_result_codes(appsettings.db, 1);

    // apply pragmas
    rc = sqlite3_exec(Application_Settings->db, Application_Settings->pragma.toUtf8(), nullptr, this, nullptr);
    if (rc != SQLITE_OK) // if open failed, quit application
    {
      showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
      sqlite3_close (appsettings.db);
      qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
      exit (1);
#else
      quick_exit (1);
#endif
    }

    if (dbman (1, appsettings) != CG_ERR_OK)
    {
      showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
      qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
      exit (1);
#else
      quick_exit (1);
#endif
    }

    // create status file
    QFile statfile (encstatusname);
    statfile.open(QIODevice::WriteOnly);
    statfile.close ();

    /*  Keep this here for development and debug. DO NOT DELETE
        initcopy.setFileName ("geanymasterbase.dat");
        initcopy.copy (appsettings.sqlitefile);
    */
  }
  else
  {
    // open sqlite db
    rc = sqlite3_open_v2 (Application_Settings->sqlitefile.toUtf8 (),
                          &Application_Settings->db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE|
                          SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_FULLMUTEX, NULL);
    if (rc != SQLITE_OK) // if open failed, quit application
    {
      showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
      sqlite3_close (appsettings.db);
      qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
      exit (1);
#else
      quick_exit (1);
#endif
    }
    sqlite3_extended_result_codes(appsettings.db, 1);
  }

  // export classes and variables
  ComboItems = &comboitems;

  // apply pragmas
  rc = sqlite3_exec(Application_Settings->db,
                    Application_Settings->pragma.toUtf8(),
                    nullptr, this, nullptr);
  if (rc != SQLITE_OK) // if open failed, quit application
  {
    showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
    sqlite3_close (appsettings.db);
    qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
    exit (1);
#else
    quick_exit (1);
#endif
  }

  // check version
  SQLCommand = CGLiteral ("SELECT * FROM VERSION;");
  rc = sqlite3_exec(Application_Settings->db, SQLCommand.toUtf8(),
                    sqlcb_dbversion, (void *) &dbversion, nullptr);
  if (rc != SQLITE_OK) // if open failed, quit application
  {
    showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
    sqlite3_close (appsettings.db);
    qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
    exit (1);
#else
    quick_exit (1);
#endif
  }

  if (dbversion == -1) // invalid dbversion
  {
    showMessage (QString::fromUtf8 ("Invalid data file. Application quits."));
    sqlite3_close (appsettings.db);
    qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
    exit (1);
#else
    quick_exit (1);
#endif
  }

  dbversion ++; // upgrade db file
  if (dbman (dbversion, appsettings) != CG_ERR_OK)
  {
    showMessage (QString::fromUtf8 ("Cannot create or open database. Application quits."));
    sqlite3_close (appsettings.db);
    qApp->exit (1);

#if defined (Q_OS_WIN) || defined (Q_OS_MAC) || defined (Q_OS_SOLARIS)
    exit (1);
#else
    quick_exit (1);
#endif
  }

  // sqlite3 limit options for linux
#ifdef Q_OS_LINUX
  // maximum columns
  sqlite3_limit(appsettings.db, SQLITE_LIMIT_COLUMN, 256);

  // maximum sql length
  sqlite3_limit(appsettings.db, SQLITE_LIMIT_SQL_LENGTH, 20971520);

  // maximum compound select
  sqlite3_limit(appsettings.db, SQLITE_LIMIT_COMPOUND_SELECT, 128);

  // maximum expression depth
  sqlite3_limit(appsettings.db, SQLITE_LIMIT_EXPR_DEPTH, 256);
#endif

  // load application's options
  loadAppOptions (&Application_Settings->options);

  // export master net service
  MasterNetService =
    new (std::nothrow) NetService (Application_Settings->options.nettimeout,
                                   nativeHttpHeader ().toLatin1 (), this);
  if (MasterNetService == nullptr)
  {
    showMessage (QString::fromUtf8 ("Cannot initialize network services. Application quits."));
    sqlite3_close (appsettings.db);
    qApp->exit (1);
  }

  Yahoo = new (std::nothrow) YahooFeed (MasterNetService);
  IEX = new (std::nothrow) IEXFeed (MasterNetService);
  AlphaVantage = new (std::nothrow) AlphaVantageFeed (MasterNetService);
  TwelveData = new (std::nothrow) TwelveDataFeed (MasterNetService);


  // show developer mode buttons
  if (Application_Settings->options.devmode == true)
  {
    ui->developButton->setVisible (true);
    ui->debugButton->setVisible (true);
    // ui->modulesButton->setVisible (true);
  }

  // show splash
  if (Application_Settings->options.showsplashscreen == true)
  {
    splash = new SplashDialog (this);
    correctWidgetFonts (splash);
    splash->setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, splash->size(), availableGeometry (-1)));
    if (!WinStore)
    {
      splash->show ();
      QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
      if (!showlicense) delay (3);
    }
  }

  // increase run counter
  SQLCommand = CGLiteral ("UPDATE VERSION SET RUNCOUNTER = RUNCOUNTER + 1;");
  rc = sqlite3_exec(Application_Settings->db, SQLCommand.toUtf8(), nullptr, this, nullptr);

  // vacuum
  SQLCommand = CGLiteral ("REINDEX; PRAGMA incremental_vacuum (256);");
  rc = sqlite3_exec(Application_Settings->db, SQLCommand.toUtf8(), sqlcb_dbdata, nullptr, nullptr);

  // load run counter and UID
  SQLCommand = CGLiteral ("SELECT * FROM VERSION;");
  rc = sqlite3_exec(Application_Settings->db, SQLCommand.toUtf8(), sqlcb_dbdata, nullptr, nullptr);

  // initialize SQL statements
  strcpy (comboitems.formats_query, "select FORMAT from FORMATS");
  strcpy (comboitems.timeframes_query, "select TIMEFRAME from TIMEFRAMES_ORDERED");
  strcpy (comboitems.currencies_query, "select SYMBOL from CURRENCIES");
  strcpy (comboitems.markets_query, "select MARKET from MARKETS");
  strcpy (comboitems.datafeeds_query, "select * from DATAFEEDS order by FEEDNAME");
  strcpy (comboitems.transactiontypes_query, "select * from TRANSACTIONTYPES");
  strcpy (comboitems.commissiontypes_query, "select * from COMMISSIONTYPES");

  // create widgets
  loadcsvdlg = new LoadCSVDialog (this);
  downloaddatadlg = new DownloadDataDialog (this);
  datamanagerdlg = new DataManagerDialog (this);
  portfoliomanagerdlg = new PortfolioManagerDialog (this);
  debugdlg = new DebugDialog (this);
  modulemanagerdlg = new ModuleManagerDialog (this);
  progressdlg = new ProgressDialog (this);
  waitdlg = new WaitDialog;
  templatemanagerdlg = new TemplateManagerDialog (this);
  optionsdlg = new OptionsDialog (this);
  infodlg = new InfoDialog (this);

  // load application fonts
  loadFonts ();

  // check for new vesrion
  checkNewVersion ();

  // export classes and variables
  loadcsvdialog = loadcsvdlg;
  downloaddatadialog = downloaddatadlg;
  progressdialog = progressdlg;
  templatemanager = templatemanagerdlg;
  debugdialog = debugdlg;

  // check if some initialization failed
  if (GlobalError.fetchAndAddAcquire (0)!= CG_ERR_OK)
  {
    ui->managerButton->setEnabled (false);
    ui->screenshotButton->setEnabled (false);
    ui->infoButton->setEnabled (false);
    ui->optionsButton->setEnabled (false);
    ui->tabWidget->setEnabled (false);
  }

  // initially remove all tabs
  while (ui->tabWidget->count () > 1)
    ui->tabWidget->removeTab (0);

  ui->tabWidget->resize (width () - 2, height () - 60);

  if (newversion)
    if (Application_Settings->options.checknewversion == true)
      if (showDownloadMessage ())
        QDesktopServices::openUrl (QUrl (APPWEBPAGE));

  waitdlg->setMessage (QString::fromUtf8 ("Exiting. Please wait..."));

#ifdef Q_OS_MAC
  NCORES = 1;
#else
  NCORES = QThread::idealThreadCount ();
  if (NCORES == -1)
    NCORES = 1;
#endif // Q_OS_MAC

  correctWidgetFonts (this);
  if (Application_Settings->options.showsplashscreen == true)
    splash->hide ();

#ifdef DEBUG
  // disable all modules
  SQLCommand = CGLiteral ("UPDATE modules SET STATUS ='DISABLED';");
  rc = sqlite3_exec(Application_Settings->db, SQLCommand.toUtf8(), sqlcb_dbdata, nullptr, nullptr);
#endif

  // disable all modules when move from a platform to another
  SQLCommand = CGLiteral ("UPDATE modules SET STATUS = 'DISABLED' WHERE PLATFORM <> '") %
               platformString () % CGLiteral ("';");
  rc = sqlite3_exec(Application_Settings->db, SQLCommand.toUtf8(), sqlcb_dbdata, nullptr, nullptr);

  // initialize cgscript
  CGScriptFunctionRegistrySize = cgscript_init ();

  // connect to unix signals
#ifndef Q_OS_WIN
  UnixSignals *usignals = new UnixSignals (this);
  connect (usignals, SIGNAL(sigTerm ()), this, SLOT (closing ()));
  connect (usignals, SIGNAL(sigHup ()), this, SLOT (closing ()));
  connect (usignals, SIGNAL(sigAbrt ()), this, SLOT (closing ()));
#endif

  if (!Application_Settings->options.showsplashscreen || WinStore)
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), availableGeometry (-1)));
  else
    showMaximized();
}

// destructor
MainWindow::~MainWindow ()
{
  MasterNetService->setAcceptRequestStatus (false);
  	
  if (ticker != nullptr)
  {
    delete ticker;
    ticker = nullptr;
  }

  if (ui->tabWidget != nullptr)
  {
    delete ui->tabWidget;
    ui->tabWidget = nullptr;
  }

  QList<QWidget *> allWidgets = this->findChildren<QWidget *> ();

  foreach (QWidget *wid, allWidgets)
  {
    if (wid->objectName () == QLatin1String ("Chart") ||
        wid->objectName () == QLatin1String ("Editor"))
      delete wid;
  }

  // close database
  if (Application_Settings->db != nullptr)
  {
    sqlite3_close (Application_Settings->db);
    Application_Settings->db = nullptr;
  }

  QString sqlitebackupfile =
    QDir::homePath () % QDir::separator()  % CGLiteral (".config") % QDir::separator()  % APPDIR % QDir::separator()  % DBNAMEBACKUP;

  QFile::remove(sqlitebackupfile);
  QFile::copy(appsettings.sqlitefile, sqlitebackupfile);

  if (waitdlg != nullptr)
  {
    delete waitdlg;
    waitdlg = nullptr;
  }

  if (sqlitebuff != nullptr)
  {
    sqlite3_config (SQLITE_CONFIG_HEAP, nullptr, 0, 64);
    free (sqlitebuff);
  }

  delete ui;
  delete MasterNetService;
}

// shutdown application
void
MainWindow::closing (void)
{
  static bool closed = false;

  if (closed == false)
    closed = true;
  else
    return;

  waitdlg->show ();
  delay (2);

  qApp->exit (0);
}

// enable ticker button
void
MainWindow::enableTickerButton ()
{
  ui->tickerButton->setEnabled (true);
};

// disable ticker button
void
MainWindow::disableTickerButton ()
{
  ui->tickerButton->setEnabled (false);
};

// add a new chart
CG_ERR_RESULT
MainWindow::addChart (TableDataVector & datavector)
{
  QTAChart *tachart;
  QTAChartData Data;
  QString SQLCmd, title, subtitle;
  int rc;
  CG_ERR_RESULT result = CG_ERR_OK;

  tachart = new (std::nothrow) QTAChart (ui->tabWidget);
  if (!tachart)
  {
    result = CG_ERR_NOMEM;
    setGlobalError(result, __FILE__, __LINE__);
    showMessage (errorMessage (result));
    return result;
  }

  if (tachart->getClassError () != CG_ERR_OK)
  {
    result = tachart->getClassError ();
    setGlobalError(result, __FILE__, __LINE__);
    showMessage (errorMessage (result));
    if (tachart != nullptr)
      delete tachart;
    return result;
  }

  foreach (const TableDataClass tdc, datavector)
  {
    ResourceMutex->lock ();
    tachart->loadFrames (tdc.tablename);
    ResourceMutex->unlock ();
    if (tachart->getClassError () != CG_ERR_OK)
    {
      result = tachart->getClassError ();
      setGlobalError(result, __FILE__, __LINE__);
      showMessage (errorMessage (result));
      delete tachart;
      return result;
    }
  }

  tachart->setSymbolKey (datavector[0].tablename);
  tachart->setObjectName ("Chart");

  // load data
  SQLCmd = CGLiteral ("select * from basedata where base = '") %
           datavector[0].base % CGLiteral ("'");
  rc = sqlite3_exec(Application_Settings->db, SQLCmd.toUtf8(),
                    sqlcb_fundamentals, static_cast <void *> (&Data), nullptr);
  if (rc != SQLITE_OK)
  {
    delete tachart;
    result = CG_ERR_ACCESS_DATA;
    setGlobalError(result, __FILE__, __LINE__);
    showMessage (CGLiteral ("Symbol ") % datavector[0].symbol % 
                 CGLiteral (": ") % errorMessage (result));
    return result;
  }

  tachart->loadData (Data);
  tachart->setSymbol (datavector[0].symbol);
  tachart->setFeed (datavector[0].source);
  title = datavector[0].symbol;
  subtitle = datavector[0].name;

  tachart->setAlwaysRedraw (true);
  tachart->setTitle (title, subtitle);

  ui->tabWidget->addTab (tachart, datavector[0].symbol % CGLiteral (" ") % (datavector[0].adjusted == CGLiteral ("NO")?CGLiteral ("RAW"):CGLiteral ("ADJ")));
  ui->tabWidget->setCurrentIndex (ui->tabWidget->count () - 1);
  tachart->setTabText (datavector[0].symbol % CGLiteral (" ") % (datavector[0].adjusted == CGLiteral ("NO")?CGLiteral ("RAW"):CGLiteral ("ADJ")));

  // remove tooltip from close tab buttons
  for (auto button : ui->tabWidget->findChildren<QAbstractButton*> ())
    if (button->inherits("CloseButton"))
      button->setToolTip(CGLiteral (""));
  
  return result;
}

// add a new portfolio
CG_ERR_RESULT
MainWindow::addPortfolio (int pf_id, QString title, QString currency, QString feed)
{
  CG_ERR_RESULT result = CG_ERR_OK;
  Portfolio *portfolio;
  QStringList tabkeys;

  tabkeys = getTabKeys ("Portfolio");
  if (tabkeys.size () > 0)
  {
    for (qint32 counter = 0; counter < tabkeys.size (); counter ++)
    {
      if (tabkeys[counter].toInt () == pf_id)
      {
        ui->tabWidget->setCurrentIndex (counter);
        return result;
      }
    }
  }

  portfolio = new (std::nothrow) Portfolio (pf_id, ui->tabWidget);
  if (!portfolio)
  {
    result = CG_ERR_NOMEM;
    setGlobalError(result, __FILE__, __LINE__);
    showMessage (errorMessage (result));
    return result;
  }

  portfolio->setObjectName ("Portfolio");
  portfolio->setTitle (title);
  portfolio->setFeed (feed);
  portfolio->setCurrency (currency);
  ui->tabWidget->addTab (portfolio, title);
  ui->tabWidget->setCurrentIndex (ui->tabWidget->count () - 1);

  return result;
}

// expanded chart
bool
MainWindow::expandedChart () const Q_DECL_NOEXCEPT
{
  return expandedChartFlag;
}

// expand/shrink chart
void
MainWindow::setExpandChart (bool expandflag)
{
  int pad;

  if (expandflag == expandedChartFlag)
    return;

  expandedChartFlag = expandflag;

  if (!expandedChartFlag)
    pad = 60;
  else
    pad = 0;

  if (tickerVisible)
    pad += TICKER_HEIGHT;

  int max = ui->tabWidget->count ();
  if (!expandflag)
  {
    if (max == 0)
      return;
    ui->managerButton->show ();
    ui->screenshotButton->show ();
    ui->optionsButton->show ();
    ui->tickerButton->show ();
    ui->portfolioButton->show ();
    ui->homeButton->show ();
    ui->infoButton->show ();
    ui->exitButton->show ();
    ui->modulesButton->show ();
    if (Application_Settings->options.devmode == true)
    {
      ui->debugButton->show ();
      ui->developButton->show ();
    }
    ui->tabWidget->move (0, 55);
    ui->tabWidget->resize (width () - 2, height () - pad);
  }
  else
  {
    ui->managerButton->hide ();
    ui->screenshotButton->hide ();
    ui->optionsButton->hide ();
    ui->tickerButton->hide ();
    ui->portfolioButton->hide ();
    ui->developButton->hide ();
    ui->homeButton->hide ();
    ui->infoButton->hide ();
    ui->exitButton->hide ();
    ui->debugButton->hide ();
    ui->modulesButton->hide ();
    ui->tabWidget->move (0, 0);
    ui->tabWidget->resize (width () - 2, height () - (pad + 5));

    if (max == 0)
      return;
  }

  for (qint32 counter = 0; counter < max; counter ++)
  {
    if (ui->tabWidget->widget(counter)->objectName () == QLatin1String ("Chart"))
      ui->tabWidget->widget(counter)->resize (ui->tabWidget->width () - 2,
                                              ui->tabWidget->height () - 20);
  }
}

// set developer's mode
void
MainWindow::setDevMode (bool devmodeflag)
{
  if (devmodeflag)
  {
    ui->developButton->setVisible (true);
    ui->debugButton->setVisible (true);
    return;
  }

  ui->developButton->setVisible (false);
  ui->debugButton->setVisible (false);
  return;
}

// get the database keys of all open charts
QStringList
MainWindow::getTabKeys (QString type)
{
  static QStringList keys;
  int max = ui->tabWidget->count ();

  keys.clear ();

  if (max == 0)
    return keys;

  for (qint32 counter = 0; counter < max; counter ++)
  {
    QWidget *wid;

    wid = ui->tabWidget->widget(counter);
    if (wid->objectName () == type && type == QLatin1String ("Chart"))
    {
      QTAChart *chart;
      chart = qobject_cast <QTAChart *> (wid);
      keys += chart->getSymbolKey ();
    }
    else if (wid->objectName () == type && type == QLatin1String ("Portfolio"))
    {
      Portfolio *portfolio;
      portfolio = qobject_cast <Portfolio *> (wid);
      keys += QString::number (portfolio->id ());
    }
    else
      keys += CGLiteral ("");
  }

  return keys;
}

/// Signals
///

// managerButton_clicked ()
void
MainWindow::managerButton_clicked ()
{
  datamanagerdlg->show ();
}

// portfolioButton_clicked ()
void
MainWindow::portfolioButton_clicked ()
{
  portfoliomanagerdlg->show ();
}

// modulesButton_clicked ()
void
MainWindow::modulesButton_clicked ()
{
  modulemanagerdlg->show ();
}

// developButton_clicked ()
void
MainWindow::developButton_clicked ()
{
  EditorWidget *editor;
  editor = new (std::nothrow) EditorWidget (ui->tabWidget);
  editor->setObjectName ("Editor");
  ui->tabWidget->addTab (editor, "New Module.cgs");
  ui->tabWidget->setCurrentIndex (ui->tabWidget->count () - 1);
}

// debugButton_clicked ()
void
MainWindow::debugButton_clicked ()
{
  if (debugdlg == nullptr)
    return;
  debugdlg->show ();
  debugdlg->activateWindow ();
  debugdlg->raise ();
}

// managerButton_clicked ()
void
MainWindow::screenshotButton_clicked ()
{
  QFileDialog *fileDialog;
  QTAChart *chart;
  QString fileName = "";
  QPixmap screenshot;

  if (ui->tabWidget->count () == 0)
  {
    showMessage ("Open a chart first please.");
    return;
  }

  if (ui->tabWidget->widget(ui->tabWidget->currentIndex ())->objectName () != QLatin1String ("Chart"))
  {
    showMessage ("Screenshots available only for charts.");
    return;
  }

  fileDialog = new QFileDialog;
  correctTitleBar (fileDialog);
  correctWidgetFonts (fileDialog);
  chart = qobject_cast <QTAChart *> (ui->tabWidget->widget(ui->tabWidget->currentIndex ()));

  chart->setCustomBottomText (APPWEBPAGE);
#if QT_VERSION < QT_VERSION_CHECK (5, 0, 0)
  screenshot = QPixmap::grabWidget (chart);
#else
  screenshot = this->grab (QRect (tabWidget->mapToParent(QPoint(0,0)), tabWidget->size ()));
#endif

  fileName = fileDialog->getSaveFileName(this, "Save chart", "", "Image (*.png)");

  if (fileName.isEmpty ())
    goto screenshotButton_clicked_end;

  if (fileName.mid (fileName.size () - 4).toLower () != QLatin1String (".png"))
    fileName += CGLiteral (".png");

  screenshot.save(fileName, "PNG");
  showMessage ("Screenshot saved.");

screenshotButton_clicked_end:
  chart->restoreBottomText ();
  delete fileDialog;

}

// exit_clicked ()
void
MainWindow::exitButton_clicked ()
{
  bool answer;

  answer = showOkCancel (CGLiteral ("Quit ") % QApplication::applicationName () % CGLiteral ("?"));
  if (!answer)
    return;

  closing ();
}

// options_clicked ()
void
MainWindow::optionsButton_clicked ()
{
  if (optionsdlg == nullptr)
    return;

  /* this is a weird trick in order to keep os x running */
  optionsdlg->exec ();
  delete optionsdlg;
  optionsdlg = new (std::nothrow) OptionsDialog (this);

  if (optionsdlg == nullptr)
    return;

  correctWidgetFonts (optionsdlg);
}

// home_clicked ()
void
MainWindow::homeButton_clicked ()
{
  QDesktopServices::openUrl(QUrl(APPWEBPAGE));
}

// info_clicked ()
void
MainWindow::infoButton_clicked ()
{
  if (infodlg == nullptr)
    return;

  infodlg->show ();
  infodlg->activateWindow ();
  infodlg->raise ();
}

// ticker_clicked ()
void
MainWindow::tickerButton_clicked ()
{
  QStringList lsymbol, lfeed;
  int pad = 0, max;

  if (tickerVisible == false)
  {
    CG_ERR_RESULT result;

    result = loadTickerSymbols (lsymbol, lfeed);
    if (result == CG_ERR_OK)
    {
      if (lsymbol.size () == 0)
      {
        showMessage ("No symbols found in ticker");
        return;
      }
    }
    else
    {
      showMessage (errorMessage (result));
      return;
    }
  }

  ui->tickerButton->setEnabled (false);

  if (tickerVisible)
  {
    tickerVisible = false;
    ticker->hide ();
    delete ticker;
    ticker = nullptr;
    ui->tickerButton->setEnabled (true);
  }
  else
  {
    tickerVisible = true;
    ticker = new StockTicker (this);

    //if (GlobalError.fetchAndAddAcquire (0)== CG_ERR_OK)
    //{
    ticker->setVisible (false);
    ticker->update ();
    this->layout ()->addWidget (ticker);
    /*
    }
    else
    {
      tickerVisible = false;
      ticker->setVisible (false);
      showMessage (errorMessage (GlobalError.fetchAndAddAcquire (0)));
      delete ticker;
      ticker = nullptr;
      ui->tickerButton->setEnabled (true);
    }
    */
  }

  if (!expandedChartFlag)
    pad = 60;
  else
    pad = 0;

  if (tickerVisible)
    pad += TICKER_HEIGHT;

  if (!expandedChartFlag)
  {
    ui->managerButton->show ();
    ui->screenshotButton->show ();
    ui->optionsButton->show ();
    ui->homeButton->show ();
    ui->infoButton->show ();
    ui->exitButton->show ();
    ui->tabWidget->move (0, 55);
    ui->tabWidget->resize (width () - 2, height () - pad);
  }
  else
  {
    ui->managerButton->hide ();
    ui->screenshotButton->hide ();
    ui->optionsButton->hide ();
    ui->portfolioButton->hide ();
    ui->developButton->hide ();
    ui->homeButton->hide ();
    ui->infoButton->hide ();
    ui->exitButton->hide ();
    ui->tabWidget->move (0, 0);
    ui->tabWidget->resize (width () - 2, height () - (pad + 5));
  }

  if (tickerVisible)
  {
    ticker->move (0, height () - TICKER_HEIGHT);
    ticker->resize (width (), TICKER_HEIGHT);
    ticker->show ();
  }

  max = ui->tabWidget->count ();
  if (max > 0)
    for (qint32 counter = 0; counter < max; counter ++)
      ui->tabWidget->widget(counter)->resize (ui->tabWidget->width () - 2,
                                              ui->tabWidget->height () - 20);

  if (tickerVisible)
  {
    ticker->setGeometry (0, height () - (TICKER_HEIGHT + 2),
                         width (), TICKER_HEIGHT);
    ticker->show ();
  }
}

// closeTab_clicked ()
void
MainWindow::closeTab_clicked (int index)
{
  MasterNetService->setAcceptRequestStatus (false);
  	
  QWidget *wid;

  if (ui->tabWidget->count () == 1)
    setExpandChart (false);

  ui->tabWidget->widget (index)->hide ();
  wid = ui->tabWidget->widget (index);
  ui->tabWidget->removeTab (index);
  delete wid;
  
  MasterNetService->setAcceptRequestStatus (true);
}

// currentTab_changed ()
void
MainWindow::currentTab_changed (int index)
{
  int max;

  max = ui->tabWidget->count ();
  if (max < 1)
    return;

  for (qint32 counter = 0; counter < max; counter ++)
    ui->tabWidget->findChild<QTabBar *> (QLatin1String("qt_tabwidget_tabbar"))->
    setTabTextColor (counter, QColor(Qt::black));

  ui->tabWidget->findChild<QTabBar *> (QLatin1String("qt_tabwidget_tabbar"))->
  setTabTextColor (index, QColor(Qt::blue));

  ui->tabWidget->widget (index)->setFocus (Qt::OtherFocusReason);
}

/// Events
///
// show
void
MainWindow::showEvent (QShowEvent * event)
{
  Q_UNUSED (event)
}

// resize
void
MainWindow::resizeEvent (QResizeEvent * event)
{
  QSize newsize;
  int max, pad;

  if (event->oldSize () == event->size ())
    return;

  newsize = event->size ();
  ui->homeButton->move (newsize.width () - 140, ui->infoButton->y ());
  ui->infoButton->move (newsize.width () - 95, ui->infoButton->y ());
  ui->exitButton->move (newsize.width () - 50, ui->exitButton->y ());

  if (!expandedChartFlag)
    pad = 60;
  else
    pad = 0;

  if (tickerVisible)
    pad += TICKER_HEIGHT;

  ui->tabWidget->resize (newsize.width () - 2, newsize.height () - pad);

  max = ui->tabWidget->count ();
  if (max > 0)
    for (qint32 counter = 0; counter < max; counter ++)
      ui->tabWidget->widget(counter)->resize (ui->tabWidget->width () - 2,
                                              ui->tabWidget->height () - 20);

  if (tickerVisible)
    ticker->setGeometry (0, height () - (TICKER_HEIGHT + 2),
                         width (), TICKER_HEIGHT);

#ifndef Q_OS_MAC
  if (showlicense)
  {
    if (windowState () == Qt::WindowMaximized && infodlg != nullptr)
    {
      infodlg->licensedlg->setGeometry(QStyle::alignedRect(Qt::LeftToRight,
                                       Qt::AlignCenter,
                                       infodlg->licensedlg->size(),
                                       availableGeometry (-1)));
      infodlg->licensedlg->show ();
      infodlg->licensedlg->raise ();
      infodlg->licensedlg->activateWindow ();
    }
  }
#endif // Q_OS_MAC
}

// focus
void
MainWindow::focusInEvent (QFocusEvent * event)
{
  ui->tabWidget->setFocus (event->reason ());
}
