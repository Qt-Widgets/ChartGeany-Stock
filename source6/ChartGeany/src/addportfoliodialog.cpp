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

#include <QDateTime>
#include <QShowEvent>
#include <QScrollBar>
#include "ui_addportfoliodialog.h"
#include "addportfoliodialog.h"
#include "common.h"

// constructor
AddPortfolioDialog::AddPortfolioDialog (QWidget * parent):
  QDialog (parent), ui (new Ui::AddPortfolioDialog)
{
  QPalette pal;
  const QString
  stylesheet = CGLiteral ("background: transparent;"),
  stylesheet2 = CGLiteral ("background: transparent; background-color: white;"),
  stylesheet3 = CGLiteral ("selection-background-color: blue");
  QStringList feedlist;

  addmode = true;
  ui->setupUi (this);

  titleEdit = ui->titleEdit;
  descriptionEdit = ui->descriptionEdit;
  datafeedsComboBox = ui->datafeedsComboBox;
  currencyComboBox = ui->currencyComboBox;

  correctButtonBoxFonts (ui->buttonBox, QDialogButtonBox::Save);
  correctButtonBoxFonts (ui->buttonBox, QDialogButtonBox::Cancel);

  connect(ui->buttonBox, SIGNAL(accepted ()), this, SLOT(ok_clicked ()));
  connect(ui->buttonBox, SIGNAL(rejected ()), this, SLOT(cancel_clicked ()));

  pal.setColor (backgroundRole (), Qt::white);
  setPalette (pal);

  ui->titleLbl->setStyleSheet (stylesheet);
  ui->datafeedLbl->setStyleSheet (stylesheet);
  ui->descriptionLbl->setStyleSheet (stylesheet);
  ui->currencyLbl->setStyleSheet (stylesheet);
  ui->titleEdit->setStyleSheet (stylesheet2);
  ui->descriptionEdit->setStyleSheet (stylesheet2);
  ui->descriptionEdit->verticalScrollBar ()->setStyleSheet (CGLiteral ("background: transparent; background-color:lightgray;"));
  ui->datafeedsComboBox->setStyleSheet (stylesheet2 + stylesheet3);
  ui->currencyComboBox->setStyleSheet (stylesheet2  + stylesheet3);

  feedlist = ComboItems->datafeedsList;
  feedlist << CGLiteral ("NONE");
  ui->datafeedsComboBox->addItems (feedlist);

  // currency list
  ComboItems->currencyList.clear ();
  int rc = selectfromdb (ComboItems->currencies_query, sqlcb_currencies, nullptr);
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_DBACCESS, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_DBACCESS));
    this->hide ();
  }
  ui->currencyComboBox->addItems (ComboItems->currencyList);

  // correctWidgetFonts (this);
  if (parent != nullptr)
    setParent (parent);

  correctTitleBar (this);
}

// destructor
AddPortfolioDialog::~AddPortfolioDialog ()
{
  delete ui;
}

/// Signals

// portfolio id call back
static int
sqlcb_portfolioid (void *data, int argc, char **argv, char **column)
{
  qint32 *pf_id = (qint32 *) data;
  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();
    if (colname == QLatin1String ("PF_ID"))
      *pf_id = QString (argv[counter]).toInt ();
  }

  return 0;
}

// save
void
AddPortfolioDialog::ok_clicked ()
{
  QString SQL;
  int rc;

  if (addmode)
  {
    if (ui->titleEdit->text ().replace ("'", " ").trimmed ().isEmpty ())
    {
      showMessage ("Portfolio has no title.");
      return;
    }

    SQL  =
      CGLiteral ("INSERT INTO portfolios (TITLE, DESCRIPTION, DATAFEED, CURRENCY) ") %
      CGLiteral ("VALUES ('") % ui->titleEdit->text ().replace ("'", " ").trimmed () % CGLiteral ("', '") %
      ui->descriptionEdit->toPlainText ().replace ("'", " ").trimmed () % CGLiteral ("', '") %
      ui->datafeedsComboBox->currentText() % CGLiteral ("', '") %
      ui->currencyComboBox->currentText() % CGLiteral ("');");
    rc = updatedb (SQL);
    if (rc != SQLITE_OK)
    {
      if (rc == 2067) //  UNIQUE constraint failed
      {
        showMessage ("Portfolio already exists.");
        return;
      }
      setGlobalError(CG_ERR_INSERT_DATA, __FILE__, __LINE__);
      showMessage (errorMessage (CG_ERR_INSERT_DATA));

      return;
    }

    qint32 pf_id = -1;
    SQL = CGLiteral ("SELECT pf_id FROM portfolios WHERE title = '") %
          ui->titleEdit->text ().replace ("'", " ").trimmed () %
          CGLiteral ("';");
    rc = selectfromdb(SQL.toUtf8(), sqlcb_portfolioid,
                      static_cast <void*> (&pf_id));
    if (rc == SQLITE_OK)
    {
// portfolio transactions
      SQL = CGLiteral ("CREATE VIEW pftrans_") % QString::number(pf_id) % CGLiteral (" AS ") %
            CGLiteral ("SELECT transactions.*, \
              (CASE \
				WHEN transactions.trtype = 'CASH IN' \
				  THEN transactions.quantity - commissionspaid.commpaid \
				WHEN transactions.trtype = 'CASH OUT' \
				  THEN (transactions.quantity + commissionspaid.commpaid) * -1 \
				WHEN transactions.trtype = 'BUY' \
				  THEN ((transactions.quantity * transactions.price) + commissionspaid.commpaid) * -1 \
				WHEN transactions.trtype = 'SELL' \
				  THEN (transactions.quantity * transactions.price) - commissionspaid.commpaid \
				WHEN transactions.trtype = 'DIVIDEND' \
				  THEN (transactions.quantity - commissionspaid.commpaid) \
				ELSE 0 \
			  END) AS AMOUNT, \
			  portfolios.datafeed AS DATAFEED \
			  FROM transactions, commissionspaid, portfolios \
			  WHERE transactions.pf_id = ") % QString::number(pf_id) %
            CGLiteral (" AND transactions.tr_id = commissionspaid.tr_id \
			    AND portfolios.pf_id = transactions.pf_id; ") %
            CGLiteral ("UPDATE portfolios SET dataview = 'pftrans_") % QString::number(pf_id) %
            CGLiteral ("full' WHERE pf_id = ") % QString::number(pf_id) % CGLiteral (";");
      SQL += createportfolioviews ("pftrans_" % QString::number(pf_id));
      rc = updatedb (SQL);
      if (rc != SQLITE_OK)
      {
        setGlobalError(CG_ERR_WRITE_FILE, __FILE__, __LINE__);
        showMessage ("Portfolio creation failed. Please delete it and try again.");
        return;
      }
    }
    else
    {
      setGlobalError(CG_ERR_WRITE_FILE, __FILE__, __LINE__);
      showMessage ("Portfolio creation failed. Please delete it and try again.");
      return;
    }

    this->hide ();
    return;
  }

  SQL = CGLiteral ("UPDATE portfolios SET TITLE = '") % ui->titleEdit->text ().replace ("'", " ").trimmed () %
        CGLiteral ("', DESCRIPTION = '") % ui->descriptionEdit->toPlainText ().replace ("'", " ").trimmed () %
        CGLiteral ("', DATAFEED = '") % ui->datafeedsComboBox->currentText() %
        CGLiteral ("', CURRENCY = '") % ui->currencyComboBox->currentText() %
        CGLiteral ("' WHERE PF_ID = ") % QString::number(pfid) % CGLiteral (";");
  rc = updatedb (SQL);
  if (rc != SQLITE_OK)
  {
    if (rc == 2067) //  UNIQUE constraint failed
    {
      showMessage ("Portfolio already exists.");
      return;
    }
    setGlobalError(CG_ERR_WRITE_FILE, __FILE__, __LINE__);
    showMessage (errorMessage (CG_ERR_WRITE_FILE));

    return;
  }

  this->hide ();
}

// close
void
AddPortfolioDialog::cancel_clicked ()
{

  this->hide ();
}

/// Events
// portfolio id call back
static int
sqlcb_portfolio (void *data, int argc, char **argv, char **column)
{
  AddPortfolioDialog *u = static_cast <AddPortfolioDialog *> (data);
  for (qint32 counter = 0; counter < argc; counter ++)
  {
    QString colname = QString (column[counter]).toUpper ();

    if (colname == QLatin1String ("TITLE"))
      u->titleEdit->setText (QString (argv[counter]));

    if (colname == QLatin1String ("DESCRIPTION"))
      u->descriptionEdit->setPlainText (QString (argv[counter]));

    if (colname == QLatin1String ("CURRENCY"))
      u->currencyComboBox->setCurrentIndex
      (u->currencyComboBox->findText (QString (argv[counter])));

    if (colname == QLatin1String ("DATAFEED"))
      u->datafeedsComboBox->setCurrentIndex
      (u->datafeedsComboBox->findText (QString (argv[counter])));
  }

  return 0;
}

// show
void
AddPortfolioDialog::showEvent (QShowEvent * event)
{
  if (event->spontaneous ())
    return;

// add
  if (addmode)
  {
    this->setWindowTitle ("Add New Portfolio");
    this->setWindowIcon (QIcon (QString (":/png/images/icons/PNG/Add_Symbol.png")));
    ui->titleEdit->setText ("");
    ui->descriptionEdit->setPlainText ("");
    ui->datafeedsComboBox->setCurrentIndex (0);
    ui->currencyComboBox->setCurrentIndex (0);
    return;
  }

// edit
  QString SQL;
  int rc;

  this->setWindowTitle ("Edit Portfolio");
  this->setWindowIcon (QIcon (QString (":/png/images/icons/PNG/Pencil_2.png")));
  SQL = CGLiteral ("SELECT * FROM portfolios WHERE pf_id = ") %
        QString::number(pfid) % CGLiteral (";");
  rc = selectfromdb(SQL.toUtf8(), sqlcb_portfolio, static_cast <void*> (this));
  if (rc != SQLITE_OK)
  {
    setGlobalError(CG_ERR_ACCESS_DATA, __FILE__, __LINE__);
    showMessage ("Cannot retrieve portfolio information.");
    return;
  }
}
