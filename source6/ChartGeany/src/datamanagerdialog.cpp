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

#include <memory>

#include <QScopedPointer>
#include <QDialogButtonBox>
#include <QScrollBar>
#include <QResizeEvent>
#include <QDateTime>
#include "ui_datamanagerdialog.h"
#include "mainwindow.h"
#include "common.h"
#include "feedyahoo.h"
#include "feedav.h"
#include "feediex.h"
#include "feedtd.h"

using std::unique_ptr;

static Q_DECL_CONSTEXPR int NCOLUMNS = 14;

extern int
sqlcb_symbol_table(void *classptr, int argc, char **argv, char **column);

// constructor
DataManagerDialog::DataManagerDialog (QWidget * parent):
  QDialog (parent), ui (new Ui::DataManagerDialog)
{
  const QString
  stylesheet = CGLiteral ("background: transparent; background-color: white;"),
  buttonstylesheet = CGLiteral ("background: transparent; background-color: white; color:black;"),
  stylesheet3 = CGLiteral ("selection-background-color: blue;");
  QStringList cheadersList, // list of columns' headers
              filter;

  int colwidth = 0;

  cheadersList << CGLiteral ("  Symbol  ")
               << CGLiteral ("  Name  ")
               << CGLiteral ("  Market  ")
               << CGLiteral ("  Feed  ")
               << CGLiteral ("  T.F.  ")
               << CGLiteral ("  Starts  ")
               << CGLiteral ("  Ends  ")
               << CGLiteral ("  Curr.  ")
               << CGLiteral ("  Key  ")
               << CGLiteral ("  Adj.  ")
               << CGLiteral ("  Base  ")
               << CGLiteral ("  Path  ")
               << CGLiteral ("  Format ")
               << CGLiteral ("  Last Update ");

  symFilter = CGLiteral ("");
  updateBeforeOpen = false;
  ui->setupUi (this);

  this->setStyleSheet (stylesheet);
  ui->tableWidget->setColumnCount (NCOLUMNS);
  ui->tableWidget->setHorizontalHeaderLabels (cheadersList);
  ui->tableWidget->sortByColumn (0, Qt::AscendingOrder);
  ui->tableWidget->setStyleSheet (stylesheet);
  ui->tableWidget->verticalScrollBar ()->setStyleSheet (CGLiteral ("background: transparent; background-color:lightgray;"));
  ui->tableWidget->horizontalScrollBar ()->setStyleSheet (CGLiteral ("background: transparent; background-color:lightgray;"));
  ui->importButton->setStyleSheet (buttonstylesheet);
  ui->downloadButton->setStyleSheet (buttonstylesheet);
  ui->trashButton->setStyleSheet (buttonstylesheet);
  ui->refreshButton->setStyleSheet (buttonstylesheet);
  ui->updateButton->setStyleSheet (buttonstylesheet);
  ui->browserButton->setStyleSheet (buttonstylesheet);
  ui->exitButton->setStyleSheet (buttonstylesheet);
  ui->chartButton->setStyleSheet (buttonstylesheet);
  ui->upToolButton->setStyleSheet (buttonstylesheet);
  ui->downToolButton->setStyleSheet (buttonstylesheet);
  ui->symbolFilterComboBox->setStyleSheet (stylesheet % stylesheet3 % CGLiteral ("combobox-popup: 0"));
  ui->tableWidget->setColumnHidden (4, true);
  ui->tableWidget->setColumnHidden (8, true);
  ui->tableWidget->setColumnHidden (10, true);
  ui->tableWidget->setColumnHidden (11, true);
  ui->tableWidget->setColumnHidden (12, true);
  ui->tableWidget->setColumnHidden (13, true);
  ui->tableWidget->setColumnWidth(0, 10);
  // ui->tableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
  ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
  reloadSymbols ();

  for (qint32 counter = 0; counter < NCOLUMNS - 1; counter ++)
    colwidth += ui->tableWidget->columnWidth (counter);

  filter += CGLiteral ("ALL");
  filter += CGLiteral ("^");
  for (char c='A'; c <= 'Z'; c++)
    filter += QString (QChar (c));
  for (char c='0'; c <= '9'; c++)
    filter += QString (QChar (c));
  ui->symbolFilterComboBox->addItems (filter);
  ui->symbolFilterComboBox->setMaxVisibleItems (10);

  this->resize (colwidth + 40, height ());
  browser = new DataBrowserDialog (this);

  // connect to signals
  connect (ui->chartButton, SIGNAL (clicked ()), this,
           SLOT (chartButton_clicked ()));
  connect (ui->importButton, SIGNAL (clicked ()), this,
           SLOT (importButton_clicked ()));
  connect (ui->downloadButton, SIGNAL (clicked ()), this,
           SLOT (downloadButton_clicked ()));
  connect (ui->refreshButton, SIGNAL (clicked ()), this,
           SLOT (refreshButton_clicked ()));
  connect (ui->exitButton, SIGNAL (clicked ()), this,
           SLOT (exitButton_clicked ()));
  connect (ui->trashButton, SIGNAL (clicked ()), this,
           SLOT (trashButton_clicked ()));
  connect (ui->updateButton, SIGNAL (clicked ()), this,
           SLOT (updateButton_clicked ()));
  connect (ui->browserButton, SIGNAL (clicked ()), this,
           SLOT (browserButton_clicked ()));
  connect (ui->downToolButton, SIGNAL (clicked ()), this,
           SLOT (downButton_clicked ()));
  connect (ui->upToolButton, SIGNAL (clicked ()), this,
           SLOT (upButton_clicked ()));
  connect(ui->tableWidget, SIGNAL(doubleClicked(const QModelIndex &)), this,
          SLOT(symbol_double_clicked ()));
  connect(ui->symbolFilterComboBox, SIGNAL(currentIndexChanged ( const QString &)),
          this, SLOT(filter_combol_changed (const QString &)));

  // correctWidgetFonts (this);
  if (parent != nullptr)
    setParent (parent);

  // correctTitleBar (this);
}

// destructor
DataManagerDialog::~DataManagerDialog ()
{
  cleartable ();
  delete ui;
}

// clear table
void
DataManagerDialog::cleartable ()
{
  const int nrows = ui->tableWidget->rowCount ();

  for (int row = 0; row < nrows; row ++)
    for (int col = 0; col < NCOLUMNS; col ++)
      delete ui->tableWidget->takeItem(row,col);
}

// fill table column
void
DataManagerDialog::fillcolumn (const QStringList &list, const int col)
{
  for (qint32 counter = 0; counter < list.size (); counter++)
  {
    QTableWidgetItem *item;

    item = new QTableWidgetItem (QTableWidgetItem::Type);
    item->setText (list[counter]);
    ui->tableWidget->setItem(counter,col,item);
  }
}

// reload all symbols
void
DataManagerDialog::reloadSymbols ()
{
  int rc;
  QString SQLCommand;

  symbolList.clear ();
  descList.clear ();
  marketList.clear ();
  sourceList.clear ();
  timeframeList.clear ();
  datefromList.clear ();
  datetoList.clear ();
  currencyList.clear ();
  keyList.clear ();
  adjustedList.clear ();
  baseList.clear ();
  pathList.clear ();
  formatList.clear ();
  lastupdateList.clear ();

  SQLCommand = CGLiteral ("select SYMBOL, DESCRIPTION, MARKET, SOURCE, TIMEFRAME, LASTUPDATE, ") %
                CGLiteral ("DATEFROM, DATETO, CURRENCY, KEY, ADJUSTED, BASE, DNLSTRING, FORMAT ") %
                CGLiteral ("from SYMBOLS_ORDERED where SYMBOL like '") % symFilter % CGLiteral ("%';");

  rc = selectfromdb(SQLCommand.toUtf8 (), sqlcb_symbol_table, this);
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DBACCESS));
    this->hide ();
    return;
  }

  cleartable ();
  ui->tableWidget->setSortingEnabled (false);
  ui->tableWidget->setRowCount (symbolList.size ());
  fillcolumn (symbolList, 0);
  fillcolumn (descList, 1);
  fillcolumn (marketList, 2);
  fillcolumn (sourceList, 3);
  fillcolumn (timeframeList, 4);
  fillcolumn (datefromList, 5);
  fillcolumn (datetoList, 6);
  fillcolumn (currencyList, 7);
  fillcolumn (keyList, 8);
  fillcolumn (adjustedList, 9);
  fillcolumn (baseList, 10);
  fillcolumn (pathList, 11);
  fillcolumn (formatList, 12);
  fillcolumn (lastupdateList, 13);

  ui->tableWidget->resizeColumnToContents (0);
  ui->tableWidget->resizeColumnToContents (1);
  ui->tableWidget->resizeColumnToContents (2);
  ui->tableWidget->resizeColumnToContents (3);
  ui->tableWidget->resizeColumnToContents (4);
  ui->tableWidget->resizeColumnToContents (5);
  ui->tableWidget->resizeColumnToContents (6);
  ui->tableWidget->resizeColumnToContents (7);
  ui->tableWidget->resizeColumnToContents (9);
  ui->tableWidget->viewport()->update();

  ui->tableWidget->setSortingEnabled (true);
}

/// Signals
///
// importButton_clicked ()
void
DataManagerDialog::importButton_clicked ()
{
  ui->tableWidget->clearSelection ();
  loadcsvdialog->show ();
}

// downloadButton_clicked ()
void
DataManagerDialog::downloadButton_clicked ()
{
  ui->tableWidget->clearSelection ();
  downloaddatadialog->show ();
}

// refreshButton_clicked ()
void
DataManagerDialog::refreshButton_clicked ()
{
  ui->tableWidget->clearSelection ();
  reloadSymbols ();
}

// exitButton_clicked ()
void
DataManagerDialog::exitButton_clicked ()
{
  ui->tableWidget->clearSelection ();
  this->hide ();
}

// sqlite3_exec callback for retieving sqlite_master table
static int
sqlcb_sqlite_master (void *classptr, int argc, char **argv, char **column)
{
  DataManagerDialog *dialog = static_cast <DataManagerDialog *> (classptr);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString::fromUtf8(column[counter]);
    colname = colname.toUpper ();
    if (colname == QLatin1String ("TYPE"))
      dialog->sqlite_master_type << QString::fromUtf8 (argv[counter]);
    if (colname == QLatin1String ("NAME"))
      dialog->sqlite_master_name << QString::fromUtf8 (argv[counter]);
  }

  return 0;
}

// form a DROP SQL sentence for the given table or view
QString
DataManagerDialog::formSQLDropSentence (QString table, qint32 *nentries)
{
  QString query, SQLCommand = "";
  int rc;

  *nentries = 0;
  query = CGLiteral ("select type, name from sqlite_master where name like '") %
          table % CGLiteral ("%';");
  rc = selectfromdb(query.toUtf8(), sqlcb_sqlite_master, this);

  if (rc == SQLITE_OK)
    for (qint32 counter = 0, maxcounter = sqlite_master_type.size ();
         counter < maxcounter; counter ++)
    {
      if (sqlite_master_type[counter] == "table" ||
          sqlite_master_type[counter] == "view")
      {

        (*nentries) ++;
        SQLCommand += CGLiteral ("DROP ") %  sqlite_master_type[counter] %
                      CGLiteral (" IF EXISTS ") % sqlite_master_name[counter] % CGLiteral (";");
        SQLCommand.append ('\n');
        SQLCommand += CGLiteral ("DELETE FROM CHART_SETTINGS WHERE KEY = '") %
                      sqlite_master_name[counter] % CGLiteral ("'; ");
        SQLCommand.append ('\n');
        SQLCommand += CGLiteral ("DELETE FROM SYMBOLS WHERE KEY = '") %
                      sqlite_master_name[counter] % CGLiteral ("'; ");
        SQLCommand.append ('\n');
        SQLCommand += CGLiteral ("DROP TABLE IF EXISTS template_") %
                      sqlite_master_name[counter] % CGLiteral (";");
        SQLCommand.append ('\n');
      }
    }
  SQLCommand += CGLiteral ("DELETE FROM SYMBOLS WHERE KEY LIKE '") % table % CGLiteral ("%'; ");
  return SQLCommand;
}

// trashButton_clicked ()
void
DataManagerDialog::trashButton_clicked ()
{
  QScopedPointer<WaitDialog> waitdlg(new WaitDialog(this));
  QString SQLCommand, table, base;
  QStringList selected_tables, selected_bases;
  qint32 entries = 0;
  int row, maxrow, rc;

  selected_tables.clear ();
  selected_bases.clear ();
  sqlite_master_name.clear ();
  sqlite_master_type.clear ();

  maxrow = ui->tableWidget->rowCount ();
  for (row = 0; row < maxrow; row ++)
    if (ui->tableWidget->item (row, 0)->isSelected ())
    {
      selected_tables << ui->tableWidget->item (row, 8)->text ();
      selected_bases << ui->tableWidget->item (row, 10)->text ();
    }

  ui->tableWidget->clearSelection ();

  if (selected_tables.size () == 0)
  {
    showMessage ("Select symbols first please.");
    return;
  }

  if (showOkCancel ("Delete selected entries ? ") == false)
    return;

  if (showOkCancel ("Entries depended on other entries (eg: entries for adjusted prices) will be deleted too. Are you sure ? ") == false)
    return;

  correctWidgetFonts (qobject_cast <QDialog *> (waitdlg.data ()));
  waitdlg->setMessage (QString::fromUtf8 ("Deleting. Please wait..."));
  waitdlg->show ();
  qApp->processEvents ();
  delay (1);

  maxrow = selected_tables.size ();
  for (row = 0; row < maxrow; row ++)
  {
    QString tmpSQL;
    table = selected_tables.at (row);
    base = selected_bases.at (row);
    tmpSQL = formSQLDropSentence (table, &entries);
    if (row % 10 != 0)
      SQLCommand += tmpSQL;
    else
    {
      rc = updatedb (SQLCommand);
      SQLCommand += tmpSQL;
    }
  }

  rc = updatedb (SQLCommand);
  if (rc != SQLITE_OK)
  {
    waitdlg->hide ();
    setGlobalError(CG_ERR_DELETE_DATA, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DELETE_DATA));
  }

  refreshButton_clicked ();
}

// updateButton_clicked ()
void
DataManagerDialog::updateButton_clicked ()
{
  QStringList symbol, timeframe, currency, feed, dateto, adjusted,
              base, key, name, market, path, format;
  QString prevbase = CGLiteral ("");
  CG_ERR_RESULT result = CG_ERR_OK;
  int row, maxrow, errcounter = 0;
  bool adjbool = true;

  maxrow = ui->tableWidget->rowCount ();
  for (qint32 counter = 0; counter < maxrow; counter ++)
  {
    if (ui->tableWidget->item (counter, 0)->isSelected ())
      for (row = 0; row < maxrow; row ++)
      {
        if (ui->tableWidget->item (counter, 10)->text () ==
            ui->tableWidget->item (row, 8)->text ())
          if (!base.contains (ui->tableWidget->item (counter, 10)->text (), Qt::CaseSensitive))
          {
            symbol << ui->tableWidget->item (row, 0)->text ();
            name << ui->tableWidget->item (row, 1)->text ();
            market << ui->tableWidget->item (row, 2)->text ();
            timeframe << ui->tableWidget->item (row, 4)->text ();
            currency << ui->tableWidget->item (row, 7)->text ();
            feed << ui->tableWidget->item (row, 3)->text ();
            dateto << ui->tableWidget->item (row, 6)->text ();
            key << ui->tableWidget->item (row, 8)->text ();
            adjusted << ui->tableWidget->item (row, 9)->text ();
            base << ui->tableWidget->item (row, 10)->text ();
            path << ui->tableWidget->item (row, 11)->text ();
            format << ui->tableWidget->item (row, 12)->text ();
          }
      }
  }

  ui->tableWidget->clearSelection ();
  if (feed.size () == 0)
  {
    if (!updateBeforeOpen)
      showMessage ("Select symbols first please.");
    return;
  }

  if (!updateBeforeOpen)
    if (showOkCancel ("Update selected entries ? ") == false)
      return;

  GlobalProgressBar = progressdialog->getProgressBar ();
  GlobalProgressBar->setValue (0);
  progressdialog->setMessage (CGLiteral ("Please wait..."));
  progressdialog->show ();
  qApp->processEvents ();
  delay (1);

  maxrow = feed.size ();
  for (row = 0; row < maxrow; row ++)
  {
    GlobalProgressBar->setValue (0);
    progressdialog->setMessage (CGLiteral ("Updating data for symbol: ") % symbol.at (row));
    qApp->processEvents ();

    if (feed.at (row) == QLatin1String ("YAHOO"))
    {
      result = Yahoo->downloadData (symbol.at (row),
                                    timeframe.at (row),
                                    currency.at (row),
                                    CGLiteral ("UPDATE"),
                                    adjbool);
      if (result != CG_ERR_OK)
      {
        errcounter ++;
        if (maxrow == 1)
        {
          progressdialog->hide ();
          showMessage (errorMessage (result));
          return;
        }
      }
    }

    if (feed.at (row) == QLatin1String ("IEX"))
    {
      result = IEX->downloadData (symbol.at (row),
                                  timeframe.at (row),
                                  currency.at (row),
                                  CGLiteral ("UPDATE"),
                                  adjbool);
      if (result != CG_ERR_OK)
      {
        errcounter ++;
        if (maxrow == 1)
        {
          progressdialog->hide ();
          showMessage (errorMessage (result));
          return;
        }
      }
    }

    if (feed.at (row) == QLatin1String ("TWELVEDATA"))
    {
      QString type = CGLiteral ("stocks");
      result = TwelveData->downloadData (symbol.at (row),
                                         type,
                                         timeframe.at (row),
                                         currency.at (row),
                                         CGLiteral ("UPDATE"),
                                         adjbool);
      if (result != CG_ERR_OK)
      {
        errcounter ++;
        if (maxrow == 1)
        {
          progressdialog->hide ();
          showMessage (errorMessage (result));
          return;
        }
      }
    }

    if (feed.at (row) == QLatin1String ("ALPHAVANTAGE"))
    {
      result = AlphaVantage->downloadData (symbol.at (row),
                                           timeframe.at (row),
                                           currency.at (row),
                                           CGLiteral ("UPDATE"),
                                           adjbool);
      if (result != CG_ERR_OK)
      {
        errcounter ++;
        if (maxrow == 1)
        {
          progressdialog->hide ();
          showMessage (errorMessage (result));
          return;
        }
      }
    }

    if (feed.at (row) == QLatin1String ("CSV") && path.at (row) != "" && format.at (row) != "")
    {
      SymbolEntry symboldata;

      symboldata.csvfile = path.at (row);
      symboldata.symbol = symbol.at (row);
      symboldata.name = name.at (row);
      symboldata.currency = currency.at (row);
      symboldata.format = format.at (row);
      symboldata.timeframe = timeframe.at (row);
      symboldata.source = CGLiteral ("CSV");

      if (QLatin1String ("YAHOO CSV") == symboldata.format)
        symboldata.adjust = true;
      else
        symboldata.adjust = false;

      if (market.at (row).isEmpty ())
        symboldata.market = CGLiteral ("NONE");
      else
        symboldata.market = market.at (row);

      symboldata.tablename = symboldata.symbol % CGLiteral ("_") %
                              symboldata.market % CGLiteral ("_") %
                              symboldata.source % CGLiteral ("_");

      symboldata.tablename += symboldata.timeframe;
      symboldata.tmptablename = CGLiteral ("TMP_") % symboldata.tablename;
      symboldata.dnlstring = symboldata.csvfile;

      result = csv2sqlite (&symboldata, CGLiteral ("CREATE"));
      if (result != CG_ERR_OK)
      {
        errcounter ++;
        if (maxrow == 1)
        {
          progressdialog->hide ();
          showMessage (errorMessage (result));
          return;
        }
      }
    }

    if (feed.at (row) == QLatin1String ("XLS") && path.at (row) != "")
    {
      SymbolEntry symboldata;

      symboldata.csvfile = path.at (row);
      symboldata.symbol = symbol.at (row);
      symboldata.name = name.at (row);
      symboldata.currency = currency.at (row);
      symboldata.format = CGLiteral ("MICROSOFT EXCEL");
      symboldata.timeframe = timeframe.at (row);
      symboldata.source = CGLiteral ("XLS");
      symboldata.adjust = false;

      if (market.at (row).isEmpty ())
        symboldata.market = CGLiteral ("NONE");
      else
        symboldata.market = market.at (row);

      symboldata.tablename = symboldata.symbol % CGLiteral ("_") %
                              symboldata.market % CGLiteral ("_") %
                              symboldata.source % CGLiteral ("_");

      symboldata.tablename += symboldata.timeframe;
      symboldata.tmptablename = CGLiteral ("TMP_") % symboldata.tablename;
      symboldata.dnlstring = symboldata.csvfile;

      result = csv2sqlite (&symboldata, CGLiteral ("CREATE"));
      if (result != CG_ERR_OK)
      {
        errcounter ++;
        if (maxrow == 1)
        {
          progressdialog->hide ();
          showMessage (errorMessage (result));
          return;
        }
      }
    }

    if (progressdialog->getCancelRequestFlag ())
    {
      progressdialog->hide ();
      showMessage (CGLiteral ("Update canceled. "));
      refreshButton_clicked ();
      return;
    }
  }

  progressdialog->hide ();
  if (!updateBeforeOpen)
    showMessage (CGLiteral ("Update completed with ") +
                 QString::number (errcounter) % CGLiteral (" errors."));
  refreshButton_clicked ();
}

// browserButton_clicked ()
void
DataManagerDialog::browserButton_clicked ()
{
  QString tablename, symbol, timeframe, name, adjusted, base, wtitle;
  int row, maxrow, selected = 0;

  maxrow = ui->tableWidget->rowCount ();
  for (row = 0; row < maxrow; row ++)
    if (ui->tableWidget->item (row, 0)->isSelected ())
    {
      if (selected == 0)
      {
        tablename = ui->tableWidget->item (row, 8)->text ();
        symbol = ui->tableWidget->item (row, 0)->text ();
        name = ui->tableWidget->item (row, 1)->text ();
        timeframe = ui->tableWidget->item (row, 4)->text ();
        adjusted = ui->tableWidget->item (row, 9)->text ();
        base = ui->tableWidget->item (row, 10)->text ();
        selected = row + 1;
      }
    }

  if (selected == 0)
  {
    showMessage ("Select a symbol first please.");
    return;
  }

  wtitle = symbol % CGLiteral (" ") % name % CGLiteral (" ");
  if (adjusted == QLatin1String ("NO"))
    wtitle += CGLiteral ("Raw");
  else
    wtitle += CGLiteral ("Adj");

  browser->setWindowTitle (wtitle);
  browser->setTableName (tablename);
  browser->show ();
}

// callback for sqlite3_exec
static int
sqlcb_table_data (void *classptr, int argc, char **argv, char **column)
{
  DataManagerDialog *dmd = static_cast <DataManagerDialog *> (classptr);
  TableDataClass tdc;

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString::fromUtf8(column[counter]);
    colname = colname.toUpper ();
    // key, symbol,  timeframe, description, adjusted, base, market, source
    if (colname == QLatin1String ("KEY"))
      tdc.tablename = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("SYMBOL"))
      tdc.symbol = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("TIMEFRAME"))
      tdc.timeframe = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("DESCRIPTION"))
      tdc.name = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("ADJUSTED"))
      tdc.adjusted = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("BASE"))
      tdc.base = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("MARKET"))
      tdc.market = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("SOURCE"))
      tdc.source = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("LASTUPDATE"))
      tdc.lastupdate = QString (argv[counter]).toUpper ();
    if (colname == QLatin1String ("CURRENCY"))
      tdc.currency = QString (argv[counter]);
  }
  dmd->TDVector += tdc;

  return 0;
}

// fill the TableDataVector
CG_ERR_RESULT
DataManagerDialog::fillTableDataVector (QString base, QString adjusted)
{
  QString query;
  int rc;

  query = CGLiteral ("SELECT key, symbol,  timeframe, description, adjusted, base, market, source, ") %
          CGLiteral ("lastupdate, currency FROM symbols WHERE base = '") % base % CGLiteral ("' AND ") %
          CGLiteral ("ADJUSTED = '") % adjusted % CGLiteral ("' ORDER BY tfresolution ASC;");

  TDVector.clear ();
  rc = selectfromdb(query.toUtf8(), sqlcb_table_data, this);
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DBACCESS));
    return CG_ERR_DBACCESS;
  }

  return CG_ERR_OK;
}

// chartButton_clicked ()
void
DataManagerDialog::chartButton_clicked ()
{
  QStringList tablename, symbol, timeframe, name, adjusted, base;
  int row, maxrow;

  maxrow = ui->tableWidget->rowCount ();
  for (row = 0; row < maxrow; row ++)
    if (ui->tableWidget->item (row, 0)->isSelected ())
    {
      tablename << ui->tableWidget->item (row, 8)->text ();
      symbol << ui->tableWidget->item (row, 0)->text ();
      name << ui->tableWidget->item (row, 1)->text ();
      timeframe << ui->tableWidget->item (row, 4)->text ();
      adjusted << ui->tableWidget->item (row, 9)->text ();
      base << ui->tableWidget->item (row, 10)->text ();
    }

  maxrow = tablename.size ();
  if (maxrow == 0)
  {
    showMessage ("Select symbols first please.");
    return;
  }

  updateBeforeOpen = true;
  for (row = 0; row < maxrow; row ++)
  {
    QStringList symkeys;
    int index = -1;

    if (fillTableDataVector (base.at (row), adjusted.at (row)) != CG_ERR_OK)
      return;

    symkeys = (qobject_cast <MainWindow*> (parent ())->getTabKeys ("Chart"));
    if (symkeys.size () != 0)
    {
      for (qint32 counter = 0; counter < symkeys.size (); counter ++)
        if (TDVector[0].tablename == symkeys[counter])
          index = counter;
    }

    if (index != -1)
    {
      this->hide ();
      (qobject_cast <MainWindow*> (parent ()))->tabWidget->setCurrentIndex (index);
    }
    else
    {
      if (qAbs (TDVector[0].lastupdate.toLongLong () -
                (QDateTime::currentMSecsSinceEpoch() / 1000)) > 7200 &&
          Application_Settings->options.autoupdate)
        updateButton_clicked ();
      this->hide ();
      (qobject_cast <MainWindow*> (parent ()))->addChart (TDVector);
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 100);
  }
  updateBeforeOpen = false;
}

/// Events
///
// resize
void
DataManagerDialog::resizeEvent (QResizeEvent * event)
{
  QSize newsize;

  if (event->oldSize () == event->size ())
    return;

  newsize = event->size ();
  ui->exitButton->move (newsize.width () - 50, ui->exitButton->y ());
  ui->upToolButton->move (newsize.width () - 50, ui->upToolButton->y ());
  ui->downToolButton->move (newsize.width () - 50, ui->downToolButton->y ());
  ui->tableWidget->resize (newsize.width () - 60, newsize.height () - 90);
  ui->filterFrame->move ((newsize.width () / 2) - 150, newsize.height () - 29);
}

// show
void
DataManagerDialog::showEvent (QShowEvent * event)
{
  if (!event->spontaneous ())
    ui->tableWidget->clearSelection ();
}

// change
void
DataManagerDialog::changeEvent (QEvent * event)
{
  if (event->spontaneous ())
    refreshButton_clicked ();
}

// delete key
void
DataManagerDialog::keyPressEvent (QKeyEvent * event)
{
  if (event->key () == Qt::Key_Delete)
    trashButton_clicked ();
}

/// Slots
// symbol double clicked
void
DataManagerDialog::symbol_double_clicked ()
{
  chartButton_clicked ();
}

// down
void
DataManagerDialog::downButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->tableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
  ui->tableWidget->setFocus (Qt::MouseFocusReason);
}

// up
void
DataManagerDialog::upButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->tableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
  ui->tableWidget->setFocus (Qt::MouseFocusReason);
}

// filter changed
void
DataManagerDialog::filter_combol_changed (const QString &f)
{
  if (f == QLatin1String ("ALL"))
    symFilter.truncate (0);
  else
    symFilter = f;

  reloadSymbols ();
}
