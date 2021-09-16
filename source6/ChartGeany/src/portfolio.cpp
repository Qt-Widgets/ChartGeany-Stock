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

#include <QTextStream>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QScrollBar>
#include "ui_portfolio.h"
#include "portfolio.h"
#include "mainwindow.h"
#include "common.h"

typedef QList<QTableWidgetItem *> RowCells;
static const int TRNCOLUMNS = 10;
static const int PNCOLUMNS = 14;

/// PriceWorkerPortfolio
// constructor
PriceWorkerPortfolio::PriceWorkerPortfolio (int pfid)
{
  parentObject = nullptr;
  state = 0;
  runflag = 1;
  pf_id = pfid;
}

// destructor
PriceWorkerPortfolio::~PriceWorkerPortfolio ()
{
  runflag = 0;
}

// process slot
void
PriceWorkerPortfolio::process()
{
  const int sleepms = 50;
  qint32 counter = 0;

  state = 1;
  while (runflag.fetchAndAddAcquire (0) == 1)
  {
    if (counter == 0)
    {
      QStringList lsymbol, lfeed;
      qint32 max;
      if (loadPortfolioSymbols (lsymbol, lfeed, pf_id) == CG_ERR_OK)
      {
        symbol = lsymbol;
        datafeed = lfeed;
      }

      max = symbol.size ();

      for (qint32 counter2 = 0; counter2 < max && runflag.fetchAndAddAcquire (0); counter2 ++)
      {
        if (runflag.fetchAndAddAcquire (0) == 1)
        {
          RTPrice rtprice;

          if (datafeed[counter2].toUpper () == QLatin1String ("YAHOO"))
            Yahoo->getRealTimePrice (symbol[counter2], rtprice, YahooFeed::HTTP);
          else if (datafeed[counter2].toUpper () == QLatin1String ("IEX"))
            IEX->getRealTimePrice (symbol[counter2], rtprice);
          else if (datafeed[counter2].toUpper () == QLatin1String ("TWELVEDATA"))
            TwelveData->getRealTimePrice (symbol[counter2], "stocks", rtprice);
          else if (datafeed[counter2].toUpper () == QLatin1String ("ALPHAVANTAGE"))
            AlphaVantage->getRealTimePrice (symbol[counter2], rtprice, AlphaVantageFeed::CSV);
        }
      }

      if (parentObject != nullptr) parentObject->emitUpdatePortfolioPrices (pf_id);
    }

    if (runflag.fetchAndAddAcquire (0) == 1)
    {
      Sleeper::msleep(sleepms);
      counter += sleepms;
      if (counter >= (Application_Settings->options.nettimeout * 2200))
        counter = 0;
    }
  }

  state = 0;
}

// terminate slot
void
PriceWorkerPortfolio::terminate ()
{
  runflag = 0;
}

/// Portfolio Totals
// constructor
PortfolioTotals::PortfolioTotals (QWidget * parent): QWidget (parent)
{
  graphicsView = new QGraphicsView (this);
  scene = new QTCGraphicsScene (this);
  graphicsView->setScene (scene);
  pixtotals = nullptr;

  graphicsView->setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
  graphicsView->setCacheMode (QGraphicsView::CacheBackground);
  graphicsView->setAlignment (Qt::AlignVCenter | Qt::AlignHCenter);
  graphicsView->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOff);
  graphicsView->setHorizontalScrollBarPolicy (Qt::ScrollBarAlwaysOff);

  scene->setItemIndexMethod (QTCGraphicsScene::NoIndex);
  scene->setBackgroundBrush (QBrush(Qt::white, Qt::SolidPattern));

  totalslabel = new QGraphicsTextItem;
  totalslabel->setFont (QFont (DEFAULT_FONT_FAMILY));
  totalslabel->setVisible (true);
  totalslabel->setPos (0, 0);
}

// set feed
void
PortfolioTotals::setFeed (QString text)
{
  feed = text;
}

// set currency
void
PortfolioTotals::setCurrency (QString text)
{
  currency = text;
}

// destructor
PortfolioTotals::~PortfolioTotals ()
{
  delete totalslabel;
}

// sqlite3_exec callback for retrieving totals
static int
sqlcb_totals (void *classptr, int argc, char **argv, char **column)
{
  QStringList *list = static_cast <QStringList *> (classptr);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    if (QString (argv[0]) == QLatin1String ("Cash") ||
        QString (argv[0]) == QLatin1String ("Total"))
    {
      list->append (QString (column[counter]).toUpper ());
      list->append (QString (argv[counter]));
    }
  }

  return 0;
}

// reload totals
void
PortfolioTotals::reloadTotals (const QString &SQL)
{
  QStringList totals;
  QString prcchange, // daily % change
          daychange, // daily change
          prcgain,   // overall % change
          gain,      // overall change
          total,     // total value
          pstr = 
            CGLiteral ("<table border=0 cellpadding=0 cellspacing=0 align='center'><tbody><tr>");
  int idx;

  const int rc = selectfromdb(SQL.toUtf8(), sqlcb_totals, static_cast <void *> (&totals));
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DBACCESS));
    return;
  }

  idx = totals.indexOf ("Total");
  if (idx < 0)
    return;

  prcchange = totals.at (totals.indexOf ("PRCCHANGE", idx) + 1);
  daychange = totals.at (totals.indexOf ("DAYCHANGE", idx) + 1);
  prcgain   = totals.at (totals.indexOf ("PRCGAIN", idx) + 1);
  gain      = totals.at (totals.indexOf ("GAIN", idx) + 1);
  total     = totals.at (totals.indexOf ("MARKETVALUE", idx) + 1);

  QLocale c(QLocale::English, QLocale::UnitedStates);
  double d;
  const char     green[] = "<td nowrap='nowrap' bgcolor=white><font size = 4 color=darkgreen><b>%1</b></font></td>",
                 red[] = "<td nowrap='nowrap' bgcolor=white><font size = 4 color=darkred><b>%1</b></font></td>",
//               white[] = "<td nowrap='nowrap' bgcolor=white><font size = 4 color=white><b>%1</b></font></td>",
//               thistle[] = "<td nowrap='nowrap' bgcolor=white><font size = 4 color=thistle><b>%1</b></font></td>",
//               darkgray[] = "<td nowrap='nowrap' bgcolor=white><font size = 4 color=darkgray><b>%1</b></font></td>",
                 black[] = "<td nowrap='nowrap' bgcolor=white><font size = 4 color=black><b>%1</b></font></td>",
                 space[] = "<td nowrap='nowrap' bgcolor=white><font size = 4 color=white>&nbsp;</font></td>",
                 *fmt;

  d = prcchange.toDouble ();
  prcchange = c.toString (d, 'f', 2) % CGLiteral ("%");

  if (d < 0)
    fmt = red;
  else if (d > 0)
    fmt = green;
  else
    fmt = black;

  d = daychange.toDouble ();
  if (feed == QLatin1String ("YAHOO") && currency == QLatin1String ("GBP"))
    d /= 100;
  daychange = c.toString (d, 'f', 2);

  pstr += 
    QString (space) %
	QString (black).arg ("Day:") %
	QString (space) %
    QString (fmt).arg (daychange) %
    QString (space) %
    QString (fmt).arg ("(" % prcchange % ")") %
    QString (space) %
    QString (space);

  d = prcgain.toDouble ();
  prcgain = c.toString (d, 'f', 2) % CGLiteral ("%");

  if (d < 0)
    fmt = red;
  else if (d > 0)
    fmt = green;
  else
    fmt = black;

  d = gain.toDouble ();
  if (feed == QLatin1String ("YAHOO") && currency == QLatin1String ("GBP"))
    d /= 100;
  gain = c.toString (d, 'f', 2);

  pstr += 
    QString (space) %
    QString (black).arg ("Overall:") %
    QString (space) %
    QString (fmt).arg (gain) %
	QString (space) %
    QString (fmt).arg ("(" % prcgain % ")") %
	QString (space) %
    QString (space);

  d = total.toDouble ();
  if (feed == QLatin1String ("YAHOO") && currency == QLatin1String ("GBP"))
    d /= 100;
  total = c.toString (d, 'f', 2);

  fmt = black;
  pstr += 
    QString (space) %
    QString (space) %
    QString (fmt).arg ("Total: " + total) %
    QString (space) %
    CGLiteral ("</tr></tbody></table>");

  qreal x = 0, y = 0, w = 0, h = 0, xpos;
  foreach (QGraphicsItem *item, scene->items ())
    scene->removeItem (item);

  totalslabel->setHtml (pstr);
  totalslabel->setVisible (false);
  scene->addItem (totalslabel);
  totalslabel->boundingRect ().getRect (&x, &y, &w, &h);
  QPixmap srcPixmap(w, h);
  QPainter tmpPainter(&srcPixmap);
  totalslabel->document()->drawContents(&tmpPainter);
  tmpPainter.end();
  scene->removeItem (totalslabel);
  fPixmap = srcPixmap.copy (5, 5, w - 10, h - 10);

  /*  center alignment: keep it here. DO NOT DELETE!
  xpos = graphicsView->sceneRect().width () - fPixmap.rect ().width ();
  if (xpos <= 0)
    xpos = 0;
  else
    xpos /= 2;
  */

  xpos = 0;

  pixtotals = scene->addPixmap (fPixmap);
  pixtotals->setPos (xpos, 5);
}

// events
// resize
void
PortfolioTotals::resizeEvent (QResizeEvent * event)
{
  Q_UNUSED (event)
  QSize newsize;

  newsize = event->size ();
  graphicsView->resize (newsize.width () - 1, 40);
  scene->setSceneRect (0, 0, newsize.width () - 10, 35);

  if (pixtotals != nullptr)
  {
    /*  center alignment: keep it here. DO NOT DELETE!
        qreal xpos = graphicsView->sceneRect().width () - fPixmap.rect ().width ();
        if (xpos <= 0)
          xpos = 0;
        else
          xpos /= 2;
    */
    qreal xpos = 0;
    pixtotals->setPos (xpos, 5);
  }
}

/// Portfolio
// constructor
Portfolio::Portfolio (int pfid, QWidget * parent):
  QWidget (parent), ui (new Ui::Portfolio)
{
  const QString
  stylesheet = QLatin1String ("background: transparent; background-color: white;"),
  stylesheet2 = QLatin1String ("background: transparent; background-color: white; color:black"),
  stylesheet3 = QLatin1String ("selection-background-color: blue;");
  
  symFilter = CGLiteral ("%");
   
  trcheadersList << CGLiteral (" Id ")
                 << CGLiteral (" Date ")
                 << CGLiteral (" Type ")
                 << CGLiteral (" Symbol ")
                 << CGLiteral (" Quantity ")
                 << CGLiteral (" Price ")
                 << CGLiteral (" Commission ")
                 << CGLiteral (" Commission Type ")
                 << CGLiteral (" Amount ")
                 << CGLiteral (" Notes ");

  pcheadersList  << CGLiteral (" Symbol ")
                 << CGLiteral (" Price ")
                 << CGLiteral (" Change ")
                 << CGLiteral (" % Change ")
                 << CGLiteral (" Day Change ")
                 << CGLiteral (" Gain ")
                 << CGLiteral (" % Gain ")
                 << CGLiteral (" Volume ")
                 << CGLiteral (" Quantity ")
                 << CGLiteral (" Price Paid ")
                 << CGLiteral (" Market Value ")
                 << CGLiteral (" Date ")
                 << CGLiteral (" Time ")
                 << CGLiteral (" Order Column ");

  colheaderList << CGLiteral (" Visible Columns ");

  worker = nullptr;
  pf_id = pfid;
  ui->setupUi (this);
  ui->chartButton->hide ();

  ui->optionsFrame->hide ();
  ui->transactionFrame->hide ();
  ui->transactionFrame->move (5, 5);
  ui->portfolioFrame->move (5, 5);
  ui->optionsFrame->move (5, 5);

  ui->addButton->setStyleSheet (stylesheet2);
  ui->editButton->setStyleSheet (stylesheet2);
  ui->deleteButton->setStyleSheet (stylesheet2);
  ui->transactionButton->setStyleSheet (stylesheet2);
  ui->portfolioButton->setStyleSheet (stylesheet2);
  ui->expandButton->setStyleSheet (stylesheet2);
  ui->chartButton->setStyleSheet (stylesheet2);
  ui->setPriceButton->setStyleSheet (stylesheet2);
  ui->optionsButton->setStyleSheet (stylesheet2);
  ui->lastupdateLbl->setStyleSheet (stylesheet2);
  ui->exportToolButton->setStyleSheet (stylesheet2);
  ui->importToolButton->setStyleSheet (stylesheet2);
  ui->trDownToolButton->setStyleSheet (stylesheet2);
  ui->trUpToolButton->setStyleSheet (stylesheet2);
  ui->downToolButton->setStyleSheet (stylesheet2);
  ui->upToolButton->setStyleSheet (stylesheet2);
  ui->colDownToolButton->setStyleSheet (stylesheet2);
  ui->colUpToolButton->setStyleSheet (stylesheet2);
  ui->colPortfolioButton->setStyleSheet (stylesheet2);

  ui->transactionTableWidget->setColumnCount (TRNCOLUMNS);
  ui->transactionTableWidget->setHorizontalHeaderLabels (trcheadersList);
  ui->transactionTableWidget->setStyleSheet (stylesheet);
  ui->transactionTableWidget->verticalScrollBar ()->setStyleSheet ("background: transparent; background-color:lightgray;");
  ui->transactionTableWidget->horizontalScrollBar ()->setStyleSheet ("background: transparent; background-color:lightgray;");
  ui->transactionTableWidget->horizontalHeader()->setStretchLastSection(true);
  ui->transactionTableWidget->horizontalHeader()->setResizeMode(7, QHeaderView::Stretch);

  ui->portfolioTableWidget->setColumnCount (PNCOLUMNS);
  ui->portfolioTableWidget->setHorizontalHeaderLabels (pcheadersList);
  ui->portfolioTableWidget->setStyleSheet (stylesheet);
  ui->portfolioTableWidget->verticalScrollBar ()->setStyleSheet ("background: transparent; background-color:lightgray;");
  ui->portfolioTableWidget->horizontalScrollBar ()->setStyleSheet ("background: transparent; background-color:lightgray;");
  ui->portfolioTableWidget->horizontalHeader()->setStretchLastSection(true);
  ui->portfolioTableWidget->horizontalHeader()->setResizeMode(7, QHeaderView::Stretch);
  ui->portfolioTableWidget->setColumnHidden (13, true);

  ui->columnsTableWidget->setColumnCount (1);
  ui->columnsTableWidget->setHorizontalHeaderLabels (colheaderList);
  ui->columnsTableWidget->setStyleSheet (stylesheet);
  ui->columnsTableWidget->verticalScrollBar ()->setStyleSheet ("background: transparent; background-color:lightgray;");
  ui->columnsTableWidget->horizontalScrollBar ()->setStyleSheet ("background: transparent; background-color:lightgray;");
  ui->columnsTableWidget->horizontalHeader()->setStretchLastSection(true);
  ui->columnsTableWidget->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);

  ui->columnsTableWidget->setRowCount (pcheadersList.size ());
  for (qint32 counter = 0; counter < pcheadersList.size () - 1; counter ++)
  {
    QWidget *pWidget;
    QCheckBox *pCheckBox;
    QHBoxLayout *pLayout;

    pWidget = new QWidget(ui->columnsTableWidget);
    pWidget->setMinimumSize (200, 48);
    pCheckBox = new QCheckBox (pcheadersList[counter], pWidget);
    pCheckBox->setMinimumSize (200, 48);
    pCheckBox->setCheckState(Qt::Checked);
    pCheckBox->setStyleSheet ("font: bold 14px;");
    pLayout = new QHBoxLayout(pWidget);
    pLayout->addWidget(pCheckBox);
    pLayout->setAlignment(Qt::AlignCenter);
    pLayout->setContentsMargins(0,0,0,0);
    pWidget->setLayout(pLayout);
    ui->columnsTableWidget->setCellWidget (counter, 0, pWidget);
    ui->columnsTableWidget->setRowHeight(counter, 48);
  }

  ui->symbolFilterComboBox->setStyleSheet (stylesheet % stylesheet3 % "combobox-popup: 0");
  ui->symbolFilterComboBox->setMaxVisibleItems (10);

  // connect to signals
  connect (ui->transactionButton, SIGNAL (clicked ()), this,
           SLOT (transactionButton_clicked ()));
  connect (ui->optionsButton, SIGNAL (clicked ()), this,
           SLOT (optionsButton_clicked ()));
  connect (ui->setPriceButton, SIGNAL (clicked ()), this,
           SLOT (setPriceButton_clicked ()));
  connect (ui->chartButton, SIGNAL (clicked ()), this,
           SLOT (chartButton_clicked ()));
  connect (ui->portfolioButton, SIGNAL (clicked ()), this,
           SLOT (portfolioButton_clicked ()));
  connect (ui->colPortfolioButton, SIGNAL (clicked ()), this,
           SLOT (colPortfolioButton_clicked ()));
  connect (ui->addButton, SIGNAL (clicked ()), this,
           SLOT (addButton_clicked ()));
  connect (ui->editButton, SIGNAL (clicked ()), this,
           SLOT (editButton_clicked ()));
  connect (ui->deleteButton, SIGNAL (clicked ()), this,
           SLOT (deleteButton_clicked ()));
  connect (ui->importToolButton, SIGNAL (clicked ()), this,
           SLOT (importButton_clicked ()));
  connect (ui->exportToolButton, SIGNAL (clicked ()), this,
           SLOT (exportButton_clicked ()));
  connect (ui->trDownToolButton, SIGNAL (clicked ()), this,
           SLOT (trDownButton_clicked ()));
  connect (ui->trUpToolButton, SIGNAL (clicked ()), this,
           SLOT (trUpButton_clicked ()));
  connect(ui->transactionTableWidget, SIGNAL(doubleClicked(const QModelIndex &)), this,
          SLOT(transaction_double_clicked ()));
  connect(ui->symbolFilterComboBox, SIGNAL(currentIndexChanged ( const QString &)),
          this, SLOT(filter_combol_changed (const QString &)));
  connect (ui->downToolButton, SIGNAL (clicked ()), this,
           SLOT (downButton_clicked ()));
  connect (ui->upToolButton, SIGNAL (clicked ()), this,
           SLOT (upButton_clicked ()));
  connect (ui->expandButton, SIGNAL (clicked ()), this,
           SLOT (expandButton_clicked ()));
  connect(ui->portfolioTableWidget, SIGNAL(doubleClicked(const QModelIndex &)), this,
          SLOT(position_double_clicked ()));
  connect (ui->colDownToolButton, SIGNAL (clicked ()), this,
           SLOT (colDownButton_clicked ()));
  connect (ui->colUpToolButton, SIGNAL (clicked ()), this,
           SLOT (colUpButton_clicked ()));

  trdlg = new AddTransactionDialog (pf_id, this);
  prcdlg = new AddPriceDialog (this);
  ptotals = new PortfolioTotals (ui->portfolioFrame);
  ptotals->move (180, 10);

  correctWidgetFonts (this);
  reloadTransactions ();
  reloadSymbols ();
  evfilter = new PortfolioEventFilter (this);
  installEventFilter(evfilter);

  // start price worker
  worker = new PriceWorkerPortfolio (pf_id);
  worker->setParentObject (this);
  worker->moveToThread(&thread);
  connect(&thread, SIGNAL(started()), worker, SLOT(process()));
  thread.start();
  thread.setPriority (QThread::LowestPriority);
  connect(this,SIGNAL(updatePortfolioPrices (int)),
          this, SLOT (updatePortfolioPricesSlot (int)));

  reloadPositions ();
  ui->portfolioFrame->show ();
}

// destructor
Portfolio::~Portfolio ()
{
  // stop price worker
  if (worker != nullptr)
  {
    worker->terminate ();
    thread.quit ();
    thread.wait ();
    delete worker;
  }

  cleartransactiontable ();
  clearpositiontable ();

  removeEventFilter (evfilter);
  delete evfilter;
  delete ui;
}

/// member functions
// set the data feed
void
Portfolio::setFeed (QString text)
{
  feed = text;
  ptotals->setFeed (feed);

  if (feed == QLatin1String ("NONE"))
  {
    ui->chartButton->hide ();
    ui->setPriceButton->show ();
    return;
  }

  ui->chartButton->show ();
  ui->setPriceButton->hide ();

};

// clear transaction table
void
Portfolio::cleartransactiontable ()
{
  int nrows = ui->transactionTableWidget->rowCount ();
  for (int row = 0; row < nrows; row ++)
    for (int col = 0; col < TRNCOLUMNS; col ++)
       delete ui->transactionTableWidget->takeItem(row,col);
       
  ui->transactionTableWidget->setRowCount (0);
  ui->transactionTableWidget->setSortingEnabled (false);
}

// emit update signal
void
Portfolio::emitUpdatePortfolioPrices (int pfid)
{
  emit updatePortfolioPrices (pfid);
}

// clear position table
void
Portfolio::clearpositiontable ()
{
  int row, nrows, col, ncols = PNCOLUMNS;

  nrows = ui->portfolioTableWidget->rowCount ();
  for (row = 0; row < nrows; row ++)
    for (col = 0; col < ncols; col ++)
      delete ui->portfolioTableWidget->takeItem(row,col);
  ui->portfolioTableWidget->setRowCount (0);
  ui->portfolioTableWidget->setSortingEnabled (false);
}

// sqlite3_exec callback for retrieving transactions
static int
sqlcb_transactions (void *classptr, int argc, char **argv, char **column)
{
  QLocale c(QLocale::English, QLocale::UnitedStates);

  QTableWidget *tw = static_cast <QTableWidget *> (classptr);
  const int rcount = tw->rowCount ();
  tw->setRowCount (rcount + 1);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    const QString colname = QString (column[counter]).toUpper ();
    QTableWidgetItem *item;

    if (colname == QLatin1String ("TR_ID"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 0, item);
    }

    if (colname == QLatin1String ("TR_DATE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 1, item);
    }

    if (colname == QLatin1String ("TRTYPE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 2, item);
    }

    if (colname == QLatin1String ("SYMBOL"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 3, item);
    }

    if (colname == QLatin1String ("QUANTITY"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 4, item);
    }

    if (colname == QLatin1String ("PRICE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 5, item);
    }

    if (colname == QLatin1String ("COMMISSION"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 6, item);
    }

    if (colname == QLatin1String ("COMMTYPE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 7, item);
    }

    if (colname == QLatin1String ("AMOUNT"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setTextAlignment(Qt::AlignCenter);
      item->setText (c.toString (strtod (argv[counter], nullptr), 'f', 2));
      tw->setItem (rcount, 8, item);
    }

    if (colname == QLatin1String ("NOTES"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 9, item);
    }
  }

  return 0;
}

// reload transactions
void
Portfolio::reloadTransactions ()
{
  cleartransactiontable ();

  const QString SQL = 
        QLatin1String ("SELECT * FROM pftrans_") % QString::number(pf_id) %
        QLatin1String ("full WHERE symbol LIKE '") % symFilter %
        QLatin1String ("';");
  
  const int rc = selectfromdb(SQL.toUtf8(), sqlcb_transactions,
                              static_cast <void *> (ui->transactionTableWidget));
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DBACCESS));
    return;
  }

  ui->transactionTableWidget->sortByColumn(1, Qt::DescendingOrder);
  ui->transactionTableWidget->setSortingEnabled (true);
  ui->transactionTableWidget->viewport()->update();
}

// sqlite3_exec callback for exporting transactions
static int
sqlcb_exptransactions (void *classptr, int argc, char **argv, char **column)
{
  QStringList *data;
  QLocale c(QLocale::English, QLocale::UnitedStates);
  QString colname;

  data = static_cast <QStringList *> (classptr);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    colname = QString (column[counter]).toUpper ();

    if (colname == QLatin1String ("TRTYPE"))
      *data += QString (argv[counter]);

    if (colname == QLatin1String ("SYMBOL"))
      *data += QString (argv[counter]);

    if (colname == QLatin1String ("TR_DATE"))
      *data += QString (argv[counter]);

    if (colname == QLatin1String ("QUANTITY"))
      *data += QString (argv[counter]);

    if (colname == QLatin1String ("PRICE"))
      *data += QString (argv[counter]);

    if (colname == QLatin1String ("COMMISSION"))
      *data += QString (argv[counter]);

    if (colname == QLatin1String ("NOTES"))
      *data += QString (argv[counter]);

    if (colname == QLatin1String ("COMMTYPE"))
      *data += QString (argv[counter]);
  }

  return 0;
}

// export transactions
CG_ERR_RESULT
Portfolio::exportTransactions (const QString & filename)
{
  QFile csv;
  QStringList data;
  QString csvline;
  int rc;

  data.clear ();
  const QString SQL =
    CGLiteral ("SELECT * FROM pftrans_") % QString::number(pf_id) %
    CGLiteral ("full;");

  rc = selectfromdb(SQL.toUtf8(), sqlcb_exptransactions, static_cast <void *> (&data));
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    return CG_ERR_DBACCESS;
  }

  csv.setFileName (filename);
  if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    return CG_ERR_OPEN_FILE;
  QTextStream out (&csv);
  out.setCodec("UTF-8");

  const QString csvheader =
    CGLiteral ("TYPE|SYMBOL|DATE|QUANTITY|PRICE|COMMISSION|NOTES|COMMTYPE");

  out << csvheader << CGLiteral ("\n");

  for (qint32 counter1 = 0; counter1 < data.size (); counter1 += 8)
  {
    csvline.truncate (0);
    for (qint32 counter2 = counter1; counter2 < (counter1 + 8); counter2 ++)
    {
      data[counter2] = data[counter2].replace ("|", " ");
      csvline += data[counter2] % CGLiteral ("|");
    }
    csvline = csvline.left (csvline.size () -1 );
    out << csvline << CGLiteral ("\n");
  }
  csv.close ();

  return CG_ERR_OK;
}

// import transactions
static QString
validateTransactions (QStringList & transaction)
{
  Q_DECL_CONSTEXPR qint32 cols = 8;
  QString result = "";

  transaction.removeAt (0);  // remove the header
  for (qint32 counter = 0; counter < transaction.size (); counter ++)
  {
    QStringList token, dtoken;
    token = transaction[counter].split ("|", QString::KeepEmptyParts);

    // check number of columns
    if (token.size () < cols)
    {
      result = "Less than " % QString::number (cols) % "columns in line " % QString::number (counter + 1) % ".";
      return result;
    }

    // check transaction type
    if (ComboItems->transactiontypeList.indexOf (token[0]) == -1)
    {
      result = "Unknown transaction type in line " % QString::number (counter + 1) % ".";
      return result;
    }

    // check commission type
    if (ComboItems->commissiontypeList.indexOf (token[7]) == -1)
    {
      result = "Unknown commission type in line " % QString::number (counter + 1) % ".";
      return result;
    }

    // check date
    dtoken = token[2].split (CGLiteral ("-"), QString::KeepEmptyParts);
    if (dtoken.size () != 3)
    {
      result = "Unknown date format in line " % QString::number (counter + 1) % ".";
      return result;
    }

    if (dtoken[0].size () != 4 || dtoken[1].size () != 2 || dtoken[2].size () != 2)
    {
      result = "Unknown date format in line " % QString::number (counter + 1) % ".";
      return result;
    }

    if (dtoken[1].toInt () < 1 || dtoken[1].toInt () > 12)
    {
      result = "Invalid month in line " % QString::number (counter + 1) % ".";
      return result;
    }

    if (dtoken[2].toInt () < 1 || dtoken[2].toInt () > 31)
    {
      result = "Invalid day in line " % QString::number (counter + 1) % ".";
      return result;
    }

  }
  return result;
}

CG_ERR_RESULT
Portfolio::importTransactions (const QString & filename, bool del)
{
  QFile rawcsv;
  QString inputline, vresult;
  QStringList trlines;

  inputline.reserve (512);
  rawcsv.setFileName (filename);
  if (rawcsv.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    QTextStream in (&rawcsv);
    in.setCodec("UTF-8");
    in.seek (0);
    while (!in.atEnd ())
    {
      inputline = in.readLine (0);
      trlines += inputline;
    }
    rawcsv.close ();
  }
  else
    return CG_ERR_OPEN_FILE;

  if (trlines.size () < 2)
    return CG_ERR_OK;

  vresult = validateTransactions (trlines);
  if (vresult != "")
    showMessage (vresult % QString (" Import aborted."));
  else
  {
    QString SQL;
    int rc;

    if (del)
      SQL = CGLiteral ("DELETE FROM transactions WHERE pf_id = ") %
            QString::number (pf_id) % CGLiteral (";");

    foreach (const QString line, trlines)
    {
      QStringList token;
      token = line.split ("|", QString::KeepEmptyParts);
      SQL.append ('\n');
      SQL +=
        CGLiteral ("INSERT INTO transactions\
        (PF_ID,TRTYPE,SYMBOL,TR_DATE,QUANTITY,PRICE,COMMISSION,NOTES,COMMTYPE)\
        VALUES (") % QString::number (pf_id) % CGLiteral (",'") %
        token[0] % CGLiteral ("','") %
        token[1] % CGLiteral ("','") %
        token[2] % CGLiteral ("',") %
        token[3] % CGLiteral (",") %
        token[4] % CGLiteral (",") %
        token[5] % CGLiteral (",'") %
        token[6] % CGLiteral ("','") %
        token[7] % CGLiteral ("');");
    }
    rc = updatedb (SQL);
    if (rc != SQLITE_OK)
      return CG_ERR_TRANSACTION;
    reloadTransactions ();
  }

  return CG_ERR_OK;
}

struct position_data_loading
{
  QTableWidget *table;
  bool GBp2GBP;
};

// sqlite3_exec callback for retrieving transactions
static int
sqlcb_positions (void *classptr, int argc, char **argv, char **column)
{
  QLocale c(QLocale::English, QLocale::UnitedStates);
  QColor green = QColor(Qt::darkGreen), red = QColor(Qt::red),
         background;
  bool sumflag = false, cashflag = false;

  background.setRgb ( 235, 235, 235 );

  position_data_loading *s = static_cast <position_data_loading *> (classptr);
  QTableWidget *tw = s->table;
  const int rcount = tw->rowCount ();
  tw->setRowCount (rcount + 1);

  for (qint32 counter = 0; counter < argc; counter ++)
  {
    const QString colname = QString (column[counter]).toUpper ();
    QTableWidgetItem *item;

    if (colname == QLatin1String ("SYMBOL"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      item->setForeground(QColor(Qt::darkBlue));
      item->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
      item->setText (QString (argv[counter]));
      tw->setItem (rcount, 0, item);
      if (QString (argv[counter]) == QLatin1String ("Cash"))
      {
        background.setRgb ( 220, 220, 220 );
        cashflag = true;
      }
      else
        cashflag = false;
      if (QString (argv[counter]) == QLatin1String ("Total"))
      {
        green.setRgb ( 0, 255, 0 );
        red.setRgb ( 255, 0, 0 );
        background.setRgb ( 220, 220, 220 );
        sumflag = true;
      }
      else
        sumflag = false;
      item->setBackground (background);
    }

    if (colname == QLatin1String ("PRICE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      if (cashflag||sumflag)
        item->setText (QString (""));
      else
      {
        if (!s->GBp2GBP)
          item->setText (c.toString (strtod (argv[counter], nullptr), 'f', 4));
        else
          item->setText (c.toString (strtod (argv[counter], nullptr) / 100, 'f', 4));
      }

      item->setBackground (background);
      tw->setItem (rcount, 1, item);
    }

    if (colname == QLatin1String ("CHANGE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      const double d = strtod (argv[counter], nullptr);

      if (d < 0)
        item->setForeground(red);
      else if (d > 0)
        item->setForeground(green);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      if (cashflag||sumflag)
        item->setText (QString (""));
      else
      {
        if (!s->GBp2GBP)
          item->setText (c.toString (strtod (argv[counter], nullptr), 'f', 4));
        else
          item->setText (c.toString (strtod (argv[counter], nullptr) / 100, 'f', 4));
      }

      item->setBackground (background);
      tw->setItem (rcount, 2, item);
    }

    if (colname == QLatin1String ("PRCCHANGE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      const double d = strtod (argv[counter], nullptr);

      if (d < 0)
        item->setForeground(QColor(Qt::red));
      else if (d > 0)
        item->setForeground(QColor(Qt::darkGreen));

      item->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      if (cashflag)
        item->setText (QString (""));
      else
        item->setText (c.toString (strtod (argv[counter], nullptr), 'f', 2) % CGLiteral ("%"));
      item->setBackground (background);
      tw->setItem (rcount, 3, item);
    }

    if (colname == QLatin1String ("DAYCHANGE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      const double d = strtod (argv[counter], nullptr);

      if (d < 0)
        item->setForeground(QColor(Qt::red));
      else if (d > 0)
        item->setForeground(QColor(Qt::darkGreen));

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      if (cashflag)
        item->setText (QString (""));
      else
      {
        if (!s->GBp2GBP)
          item->setText (c.toString (strtod (argv[counter], nullptr), 'f', 2));
        else
          item->setText (c.toString (strtod (argv[counter], nullptr) / 100, 'f', 2));
      }

      item->setBackground (background);
      tw->setItem (rcount, 4, item);
    }

    if (colname == QLatin1String ("GAIN"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      const double d = strtod (argv[counter], nullptr);

      if (d < 0)
        item->setForeground(QColor(Qt::red));
      else if (d > 0)
        item->setForeground(QColor(Qt::darkGreen));

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      if (cashflag)
        item->setText (QString (""));
      else
      {
        if (!s->GBp2GBP)
          item->setText (QString (" ") + c.toString (strtod (argv[counter], nullptr), 'f', 2) + QString (" "));
        else
          item->setText (QString (" ") + c.toString (strtod (argv[counter], nullptr) / 100, 'f', 2) + QString (" "));
      }
      item->setBackground (background);
      tw->setItem (rcount, 5, item);
    }

    if (colname == QLatin1String ("PRCGAIN"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      const double d = strtod (argv[counter], nullptr);

      if (d < 0)
        item->setForeground(QColor(Qt::red));
      else if (d > 0)
        item->setForeground(QColor(Qt::darkGreen));

      item->setTextAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      if (cashflag)
        item->setText (QString (""));
      else
        item->setText (QString (" ") + c.toString (d, 'f', 2) % CGLiteral ("%") % QString (" "));
      item->setBackground (background);
      tw->setItem (rcount, 6, item);
    }

    if (colname == QLatin1String ("VOLUME"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      if (cashflag||sumflag)
        item->setText (QString (""));
      else
        item->setText (c.toString (atoll (argv[counter])));
      item->setBackground (background);
      tw->setItem (rcount, 7, item);
    }

    if (colname == QLatin1String ("QUANTITY"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);
      const double d = strtod (argv[counter], nullptr);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      if (cashflag||sumflag)
        item->setText (QString (""));
      else
        item->setText (QString (" ") + c.toString (d, 'f', 4) % QString (" "));
         
      item->setBackground (background);
      tw->setItem (rcount, 8, item);
    }

    if (colname == QLatin1String ("PRICEPAID"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      if (cashflag||sumflag)
        item->setText (QString (""));
      else
      {
        if (!s->GBp2GBP)
          item->setText (QString (" ") + c.toString (strtod (argv[counter], nullptr), 'f', 4) + QString (" "));
        else
          item->setText (QString (" ") + c.toString (strtod (argv[counter], nullptr) / 100, 'f', 4) + QString (" "));
      }

      item->setBackground (background);
      tw->setItem (rcount, 9, item);
    }

    if (colname == QLatin1String ("MARKETVALUE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);

      item->setForeground(QColor(Qt::darkMagenta));
      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      // item->setText (QString ("%1").arg (strtod (argv[counter], nullptr), 10, 'f', 2));

      if (!s->GBp2GBP)
        item->setText (c.toString (strtod (argv[counter], nullptr), 'f', 2));
      else
        item->setText (c.toString (strtod (argv[counter], nullptr) / 100, 'f', 2));

      item->setBackground (background);
      tw->setItem (rcount, 10, item);
    }

    if (colname == QLatin1String ("DATE"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      item->setText (" " % QString (argv[counter]) % " ");
      item->setBackground (background);
      tw->setItem (rcount, 11, item);
    }

    if (colname == QLatin1String ("TIME"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      item->setText (QString (argv[counter]));
      item->setBackground (background);
      tw->setItem (rcount, 12, item);
    }

    if (colname == QLatin1String ("ORDERCOL"))
    {
      item = new QTableWidgetItem (QTableWidgetItem::Type);

      item->setTextAlignment(Qt::AlignRight|Qt::AlignVCenter);
      item->setText (QString (argv[counter]));
      item->setBackground (background);
      tw->setItem (rcount, 13, item);
    }
  }

  return 0;
}

// reload positions
void
Portfolio::reloadPositions ()
{
  clearpositiontable ();

  position_data_loading pdl;
  pdl.table = ui->portfolioTableWidget;
  if (feed == "YAHOO" && currency == "GBP")
    pdl.GBp2GBP = true;
  else
    pdl.GBp2GBP = false;

  const QString SQL =
    CGLiteral ("SELECT * FROM pftrans_") % QString::number(pf_id) % 
    CGLiteral ("cview6 ORDER BY ordercol;");
  const int rc = selectfromdb(SQL.toUtf8(), sqlcb_positions,
                    static_cast <void *> (&pdl));
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DBACCESS));
    return;
  }

  ui->portfolioTableWidget->sortByColumn(13, Qt::AscendingOrder);
  ui->portfolioTableWidget->setSortingEnabled (false);
  ui->portfolioTableWidget->viewport()->update();

  ptotals->reloadTotals (SQL);
}

// sqlite3_exec callback for retrieving distinct symbols
static int
sqlcb_symbols (void *classptr, int argc, char **argv, char **column)
{
  QStringList *symbols = static_cast <QStringList *> (classptr);
  for (qint32 counter = 0; counter < argc; counter ++)
  {
    const QString colname = QString (column[counter]).toUpper ();
    if (colname == "SYMBOL")
      if (QString (argv[counter]).size () > 0)
        symbols->append (QString (argv[counter]));
  }

  return 0;
}

// reload symbols
void
Portfolio::reloadSymbols ()
{
  QStringList symbols;

  const QString SQL =
    CGLiteral ("SELECT DISTINCT symbol FROM pftrans_") % QString::number(pf_id) %
    CGLiteral ("full WHERE symbol <> ' ' ORDER BY symbol;");
  symbols.append (QString ("NO FILTER"));
  const int rc = selectfromdb (SQL.toUtf8(), sqlcb_symbols, static_cast <void *> (&symbols));
  if (rc != SQLITE_OK)
  {
    setGlobalError (CG_ERR_DBACCESS, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DBACCESS));
    return;
  }

  const QString csymbol = ui->symbolFilterComboBox->currentText ();
  ui->symbolFilterComboBox->clear ();
  ui->symbolFilterComboBox->addItems (symbols);
  const int idx = ui->symbolFilterComboBox->findText (csymbol);
  if (idx != -1) ui->symbolFilterComboBox->setCurrentIndex (idx);
}

// reload spacific transaction
static int
sqlcb_reloadtransaction (void *data, int argc, char **argv, char **column)
{
  RowCells *cells;
  cells = static_cast <RowCells *> (data);
  for (qint32 counter = 0; counter < argc; counter ++)
  {
    const QString colname = QString (column[counter]).toUpper ();
    if (colname == QLatin1String ("TR_DATE"))
      (*cells)[1]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("TRTYPE"))
      (*cells)[2]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("SYMBOL"))
      (*cells)[3]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("QUANTITY"))
      (*cells)[4]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("PRICE"))
      (*cells)[5]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("COMMISSION"))
      (*cells)[6]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("COMMTYPE"))
      (*cells)[7]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("AMOUNT"))
      (*cells)[8]->setText (QString (argv[counter]));
    if (colname ==  QLatin1String ("NOTES"))
      (*cells)[9]->setText (QString (argv[counter]));
  }

  return 0;
}

void
Portfolio::reloadTransaction (int tr_id)
{
  QString SQL;
  RowCells rowcells;

  if (tr_id != -1)
  {
    int rc;

    rowcells = ui->transactionTableWidget->selectedItems ();
    SQL = CGLiteral ("SELECT * FROM transactions WHERE tr_id = ") %
          QString::number (tr_id) % ";";
    rc = selectfromdb(SQL.toUtf8(), sqlcb_reloadtransaction,
                      static_cast <void *> (&rowcells));
    if (rc != SQLITE_OK)
    {
      setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
      showMessage (errorMessage (CG_ERR_DBACCESS));
      return;
    }
  }

  reloadTransactions ();
  reloadSymbols ();
  return;
}

/// Events
///
// show
void
Portfolio::showEvent(QShowEvent * event)
{
  Q_UNUSED (event);
  QString SQL = CGLiteral ("SELECT * FROM pftrans_") % QString::number(pf_id) %
                CGLiteral ("cview6 ORDER BY ordercol;");
  ptotals->reloadTotals (SQL);
}

// resize
void
Portfolio::resizeEvent (QResizeEvent * event)
{
  QSize newsize;

  if (event->oldSize () == event->size ())
    return;

  newsize = event->size ();
  ui->portfolioFrame->resize (newsize.width (), newsize.height ());
  ui->transactionFrame->resize (newsize.width (), newsize.height ());
  ui->optionsFrame->resize (newsize.width (), newsize.height ());
  ui->portfolioButton->move (newsize.width () - 50, 10);
  ui->colPortfolioButton->move (newsize.width () - 50, 10);

  newsize = ui->portfolioFrame->size ();
  ui->upToolButton->move (newsize.width () - 50, ui->upToolButton->y ());
  ui->downToolButton->move (newsize.width () - 50, ui->downToolButton->y ());
  ui->portfolioTableWidget->resize (newsize.width () - 60, newsize.height () - 90);
  ui->lastupdateLbl->move (5, newsize.height () - 30);
  ptotals->resize (newsize.width () - 240, 40);

  newsize = ui->transactionFrame->size ();
  ui->trUpToolButton->move (newsize.width () - 50, ui->trUpToolButton->y ());
  ui->trDownToolButton->move (newsize.width () - 50, ui->trDownToolButton->y ());
  ui->transactionTableWidget->resize (newsize.width () - 60, newsize.height () - 100);
  ui->filterFrame->move ((newsize.width () / 2) - 175, newsize.height () - 40);

  newsize = ui->optionsFrame->size ();
  ui->colUpToolButton->move (newsize.width () - 50, ui->trUpToolButton->y ());
  ui->colDownToolButton->move (newsize.width () - 50, ui->trDownToolButton->y ());
  ui->columnsTableWidget->resize (newsize.width () - 60, newsize.height () - 100);
}

/// Slots
// optionsButton_clicked ()
void
Portfolio::optionsButton_clicked ()
{
  ui->portfolioFrame->hide ();
  ui->optionsFrame->show ();
}

// transactionButton_clicked ()
void
Portfolio::transactionButton_clicked ()
{
  ui->portfolioFrame->hide ();
  ui->transactionFrame->show ();
}

// setPriceButton_clicked ()
void
Portfolio::setPriceButton_clicked ()
{
  QString symbol, volume, price, change;

  for (qint32 row = 0; row < ui->portfolioTableWidget->rowCount (); row ++)
  {
    if (ui->portfolioTableWidget->item (row, 0)->isSelected ())
    {
      symbol = ui->portfolioTableWidget->item (row, 0)->text ();
      volume = ui->portfolioTableWidget->item (row, 7)->text ();
      price = ui->portfolioTableWidget->item (row, 1)->text ();
      change = ui->portfolioTableWidget->item (row, 2)->text ();
    }
  }

  if (symbol.isEmpty () || symbol == QLatin1String ("Cash") || 
      symbol == QLatin1String ("Total"))
  {
    showMessage ("Select a position first please.");
    return;
  }

  prcdlg->setDefaults (symbol, price, change, volume);
  prcdlg->show ();
}

// portfolioButton_clicked ()
void
Portfolio::portfolioButton_clicked ()
{
  ui->transactionFrame->hide ();
  reloadPositions ();
  ui->portfolioFrame->show ();
}

// addButton_clicked ()
void
Portfolio::addButton_clicked ()
{
  trdlg->setAddMode ();
  trdlg->show ();
}

// editButton_clicked ()
void
Portfolio::editButton_clicked ()
{
  int tr_id = -1;

  for (int row = 0; row < ui->transactionTableWidget->rowCount (); row ++)
  {
    if (ui->transactionTableWidget->item (row, 0)->isSelected ())
    {

      tr_id = ui->transactionTableWidget->item (row, 0)->text ().toInt ();
    }
  }

  if (tr_id == -1)
  {
    showMessage ("Select a transaction first please.");
    return;
  }

  trdlg->setEditMode (tr_id);
  trdlg->show ();
}

// deleteButton_clicked ()
void
Portfolio::deleteButton_clicked ()
{
  QString SQL;
  int tr_id = -1, rc;

  for (int row = 0; row < ui->transactionTableWidget->rowCount (); row ++)
  {
    if (ui->transactionTableWidget->item (row, 0)->isSelected ())
      tr_id = ui->transactionTableWidget->item (row, 0)->text ().toInt ();
  }

  if (tr_id == -1)
  {
    showMessage ("Select a transaction first please.");
    return;
  }

  if (showOkCancel ("Delete selected transaction ? ") == false)
    return;

  SQL = CGLiteral ("DELETE FROM transactions WHERE tr_id = ") %
        QString::number (tr_id) % ";";

  rc = updatedb (SQL);
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_TRANSACTION, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_TRANSACTION));
    return;
  }

  reloadTransactions ();
  reloadSymbols ();
}

// transaction import
void
Portfolio::importButton_clicked (void)
{
  QFileDialog *fileDialog = new QFileDialog;
  QString fileName;
  CG_ERR_RESULT result;
  bool del;

  correctTitleBar (fileDialog);
  correctWidgetFonts (fileDialog);

  fileName = fileDialog->getOpenFileName (this, "Import transactions", "", "Text files (*.csv)");

  if (fileName.isEmpty ())
    goto importButton_clicked_end;

  if (fileName.mid (fileName.size () - 4).toLower () != QLatin1String (".csv"))
    fileName += ".csv";

  del = showOkCancel ("Delete existing transactions?");

  result = importTransactions (fileName, del);
  if (result != CG_ERR_OK)
  {
    showMessage (errorMessage (result));
    goto importButton_clicked_end;
  }

  reloadSymbols ();
  showMessage ("Import completed");

importButton_clicked_end:
  delete fileDialog;
}

// transaction export
void
Portfolio::exportButton_clicked (void)
{
  QFileDialog *fileDialog = new QFileDialog;
  QString fileName;
  CG_ERR_RESULT result;

  correctTitleBar (fileDialog);
  correctWidgetFonts (fileDialog);

  fileName = fileDialog->getSaveFileName(this, "Export transactions", ptitle, "Text files (*.csv)");

  if (fileName.isEmpty ())
    goto exportButton_clicked_end;

  if (fileName.mid (fileName.size () - 4).toLower () != QLatin1String (".csv"))
    fileName += CGLiteral (".csv");

  result = exportTransactions (fileName);
  if (result != CG_ERR_OK)
  {
    showMessage (errorMessage (result));
    goto exportButton_clicked_end;
  }
  showMessage ("Export completed");

exportButton_clicked_end:
  delete fileDialog;
}

// transaction down
void
Portfolio::trDownButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->transactionTableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
  ui->transactionTableWidget->setFocus (Qt::MouseFocusReason);
}

// transaction up
void
Portfolio::trUpButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->transactionTableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
  ui->transactionTableWidget->setFocus (Qt::MouseFocusReason);
}

// transaction double click
void
Portfolio::transaction_double_clicked ()
{
  editButton_clicked ();
}

// filter changed
void
Portfolio::filter_combol_changed (const QString &f)
{
  QString oldSymFilter = symFilter;

  if (f == QLatin1String ("NO FILTER"))
    symFilter = CGLiteral ("%");
  else
    symFilter = f;

  reloadTransactions ();
}

// portfolio down
void
Portfolio::downButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->portfolioTableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
  ui->portfolioTableWidget->setFocus (Qt::MouseFocusReason);
}

// portfolio up
void
Portfolio::upButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->portfolioTableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
  ui->portfolioTableWidget->setFocus (Qt::MouseFocusReason);
}

// columns down
void
Portfolio::colDownButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->columnsTableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
  ui->columnsTableWidget->setFocus (Qt::MouseFocusReason);
}

// columns up
void
Portfolio::colUpButton_clicked (void)
{
  QScrollBar *vScrollBar = ui->columnsTableWidget->verticalScrollBar();
  vScrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
  ui->columnsTableWidget->setFocus (Qt::MouseFocusReason);
}

// update portfolio prices
void
Portfolio::updatePortfolioPricesSlot (int pfid)
{
  if (pfid == pf_id)
  {
    QString lastupd;

    lastupd = CGLiteral ("Last update: ") % 
              QDateTime::currentDateTime().toString ("yyyy-MM-dd hh:mm:ss");
    ui->lastupdateLbl->setText (lastupd);
    reloadPositions ();
  }
}

// double click on position
void
Portfolio::position_double_clicked (void)
{
  for (qint32 row = 0; row < ui->portfolioTableWidget->rowCount (); row ++)
  {
    if (ui->portfolioTableWidget->item (row, 0)->isSelected ())
    {
      QString symbol;
      symbol = ui->portfolioTableWidget->item (row, 0)->text ();
      if (symbol != QLatin1String ("Cash") && 
          symbol != QLatin1String ("Total"))
      {
        ui->symbolFilterComboBox->setCurrentIndex
        (ui->symbolFilterComboBox->findText (symbol));
        transactionButton_clicked ();
      }
    }
  }
}

// expand / restore
void
Portfolio::expandButton_clicked (void)
{
  MainWindow *mainwindow;

  mainwindow = (qobject_cast <MainWindow *>
                (this->parentWidget ()->parentWidget ()->parentWidget ()->parentWidget ()));

  if (mainwindow->expandedChart ())
    mainwindow->setExpandChart (false);
  else
    mainwindow->setExpandChart (true);
  return;
}

// chart button
void
Portfolio::chartButton_clicked (void)
{
  DataManagerDialog dmd;
  TableDataVector tdv;
  QString symbol = "", tablename;
  CG_ERR_RESULT result;

  for (qint32 row = 0; row < ui->portfolioTableWidget->rowCount (); row ++)
  {
    if (ui->portfolioTableWidget->item (row, 0)->isSelected ())
      symbol = ui->portfolioTableWidget->item (row, 0)->text ();
  }

  if (symbol.isEmpty () || symbol == "Cash" || symbol == "Total")
  {
    showMessage ("Select a position first please.");
    return;
  }

  progressdialog->show ();
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);

  if (feed == QLatin1String ("YAHOO"))
  {
    GlobalProgressBar = progressdialog->getProgressBar ();
    GlobalProgressBar->setValue (0);
    progressdialog->setMessage ("Updating data for symbol: " % symbol);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);

    result = Yahoo->downloadData (symbol, CGLiteral ("DAY"), 
                                  currency, CGLiteral ("DOWNLOAD"), false);
    if (result != CG_ERR_OK)
    {
      progressdialog->hide ();
      showMessage (errorMessage (result));
      return;
    }

    tdv = dmd.getTableDataVector (Yahoo->getTableName (), "NO");
  }
  else 
  if (feed == QLatin1String ("IEX"))
  {
    GlobalProgressBar = progressdialog->getProgressBar ();
    GlobalProgressBar->setValue (0);
    progressdialog->setMessage ("Updating data for symbol: " % symbol);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);

    result = IEX->downloadData (symbol, "DAY", currency, "DOWNLOAD", false);
    if (result != CG_ERR_OK)
    {
      progressdialog->hide ();
      showMessage (errorMessage (result));
      return;
    }

    tdv = dmd.getTableDataVector (IEX->getTableName (), "NO");
  }
  else 
  if (feed == QLatin1String ("TWELVEDATA"))
  {
    GlobalProgressBar = progressdialog->getProgressBar ();
    GlobalProgressBar->setValue (0);
    progressdialog->setMessage ("Updating data for symbol: " % symbol);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);

    result = TwelveData->downloadData (symbol, "STOCKS", "DAY", currency, "DOWNLOAD", false);
    if (result != CG_ERR_OK)
    {
      progressdialog->hide ();
      showMessage (errorMessage (result));
      return;
    }

    tdv = dmd.getTableDataVector (TwelveData->getTableName (), "NO");
  }
  else 
  if (feed == QLatin1String ("ALPHAVANTAGE"))
  {
    GlobalProgressBar = progressdialog->getProgressBar ();
    GlobalProgressBar->setValue (0);
    progressdialog->setMessage ("Updating data for symbol: " % symbol);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 25);

    result = AlphaVantage->downloadData (symbol, "DAY", currency, "DOWNLOAD", false);
    if (result != CG_ERR_OK)
    {
      progressdialog->hide ();
      showMessage (errorMessage (result));
      return;
    }

    tdv = dmd.getTableDataVector (AlphaVantage->getTableName (), "NO");
  }
  else
    return;
  
  
  if (tdv.size () != 4)
  {
    progressdialog->hide ();
    showMessage (errorMessage (GlobalError.fetchAndAddAcquire (0)));
    return;
  }
  

  QStringList symkeys;
  int index = -1;

  symkeys = ((qobject_cast <MainWindow*> (parent ()->parent ()->parent ()->parent ()))->getTabKeys ("Chart"));
  if (symkeys.size () != 0)
  {
    for (qint32 counter = 0; counter < symkeys.size (); counter ++)
      if (tdv[0].tablename == symkeys[counter])
        index = counter;
  }

  progressdialog->hide ();
  if (index != -1)
    (qobject_cast <MainWindow*> (parent ()->parent ()->parent ()->parent ()))->tabWidget->setCurrentIndex (index);
  else
    (qobject_cast <MainWindow*> (parent ()->parent ()->parent ()->parent ()))->addChart (tdv);
}

// portfolioButton_clicked ()
void
Portfolio::colPortfolioButton_clicked ()
{

  for (qint32 counter = 0; counter < ui->columnsTableWidget->rowCount (); counter ++)
  {
    QList<QWidget *> allWidgets = ui->columnsTableWidget->cellWidget(counter, 0)->findChildren<QWidget *> ();
    foreach (const QWidget *w, allWidgets)
    {
      if (QString (w->metaObject()->className()) == QString ("QCheckBox"))
      {
        QCheckBox *cb;
        cb = (QCheckBox *) w;

        if (cb->isChecked ())
          ui->portfolioTableWidget->setColumnHidden (counter, false);
        else
          ui->portfolioTableWidget->setColumnHidden (counter, true);
      }
    }
  }

  ui->portfolioTableWidget->resizeColumnsToContents();
  ui->portfolioTableWidget->horizontalHeader()->setStretchLastSection(true);

  ui->optionsFrame->hide ();
  ui->portfolioFrame->show ();
}

// event filter
PortfolioEventFilter::PortfolioEventFilter (QObject * parent)
{
  Q_UNUSED (parent);
}

PortfolioEventFilter::~PortfolioEventFilter ()
{

}

bool
PortfolioEventFilter::eventFilter(QObject *obj, QEvent *event)
{
  if (event->type() == QEvent::KeyPress)
  {
    QKeyEvent *keyEvent = static_cast <QKeyEvent *> (event);

    // tab
    if (keyEvent->key() == Qt::Key_Tab)
    {
      QTabWidget *tabWidget = (QTabWidget *) obj->parent ()->parent ();
      
      const int idx = tabWidget->currentIndex () + 1;
      
      if (idx == tabWidget->count ())
        return QObject::eventFilter(obj, event);

      tabWidget->setCurrentIndex (idx);
      tabWidget->update ();
      return QObject::eventFilter(obj, event);
    }

    // backtab
    if (keyEvent->key() == Qt::Key_Backtab)
    {
      QTabWidget *tabWidget = (QTabWidget *) obj->parent ()->parent ();
      
      int idx = tabWidget->currentIndex ();
      if (idx == 0)
        return QObject::eventFilter(obj, event);

      idx --;
      tabWidget->setCurrentIndex (idx);
      tabWidget->update ();
      return QObject::eventFilter(obj, event);
    }
  }
  return QObject::eventFilter(obj, event);
}
