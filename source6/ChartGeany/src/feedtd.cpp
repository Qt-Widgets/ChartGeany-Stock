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

#include <QApplication>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDateTime>
#include "feedtd.h"
#include "common.h"

using std::unique_ptr;

// constructor
TwelveDataFeed::TwelveDataFeed (QObject *parent)
{
  if (parent != nullptr)
    setParent (parent);

  populated = false;

  netservice = MasterNetService;
}

// destructor
TwelveDataFeed::~TwelveDataFeed ()
{

  return;
}

// validate symbol
bool
TwelveDataFeed::validSymbol (const QString &symbol)
{
  for (qint32 counter = 0, max = symbol.size ();
       counter < max; counter ++)
  {
    if (!((symbol.at (counter) >= 'A' && symbol.at (counter) <= 'Z') ||
          (symbol.at (counter) >= 'a' && symbol.at (counter) <= 'z') ||
          (symbol.at (counter) >= '0' && symbol.at (counter) <= '9') ||
          symbol.at (counter) == '.' || symbol.at (counter) == '-'))
      return false;
  }

  return true;
}

// return symbol check URL
// eg: https://api.twelvedata.com/stocks?symbol=AAPL
QString
TwelveDataFeed::symbolURL (const QString &symbol, const QString &type)
{
  if (symbol.size () == 0)
    return CGLiteral ("");

  const QString urlstr =
    CGLiteral ("https://api.twelvedata.com/") % type.toLower () %
    CGLiteral ("?symbol=") % symbol %
    ((type == QLatin1String ("STOCKS") ?
      CGLiteral ("&country=usa") :
      CGLiteral ("")));

  return urlstr;
}

QString
TwelveDataFeed::realTimePriceURL (const QString &symbol, const QString &type)
{
  const QString urlstr =
    CGLiteral ("https://api.twelvedata.com/quote?interval=1day&format=json") %
    CGLiteral ("&symbol=") % symbol % CGLiteral ("&apikey=") %
    Application_Settings->options.tdapikey %
    (type == QLatin1String ("STOCKS") ?
     CGLiteral ("&country=usa") :
     CGLiteral (""));

  return urlstr;
}

// return symbol statistics URL from http
QString
TwelveDataFeed::symbolStatsURL (const QString &symbol)
{
  Q_UNUSED (symbol);

  return CGLiteral ("");
}

// return historical download URL
// eg https://api.twelvedata.com/time_series?symbol=GE&interval=1day&apikey=1d3a570bfbca45e98cc2fdf804a8dcae&outputsize=0&start_date=1970-01-01
QString
TwelveDataFeed::downloadURL (const QString &symbol, const QString &market)
{
  if (symbol.size () == 0)
    return CGLiteral ("");

  const QString downstr =
    CGLiteral ("https://api.twelvedata.com/time_series?format=csv&symbol=") %
    symbol % CGLiteral ("&interval=1day&outputsize=5000") %
    CGLiteral ("&apikey=") % Application_Settings->options.tdapikey %
    ((!market.isEmpty ())  ?
      QStringLiteral ("&exchange=") % market :
      QStringLiteral (""));

  return downstr;
}

// return update URL
QString
TwelveDataFeed::updateURL (const QString &symbol, const QString &market)
{
  return downloadURL (symbol, market);
}

// get real time price
CG_ERR_RESULT
TwelveDataFeed::getRealTimePrice (QString symbol, QString type, RTPrice & rtprice)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  CG_ERR_RESULT result = CG_ERR_OK;

  QString url = realTimePriceURL (symbol, type);

  RTPrice realtimeprice;        // real time price
  realtimeprice.symbol = symbol;
  realtimeprice.feed = CGLiteral ("TWELVEDATA");
  rtprice = realtimeprice;

  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
    return CG_ERR_NOMEM;

  // open temporary file
  if (!tempFile->open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }
  tempFile.get ()->resize (0);

  result = netservice->httpGET (url, *tempFile.get (), nullptr);
  if (result != CG_ERR_OK)
  {
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }

  QTextStream in;

  in.setDevice (tempFile.get ());
  in.seek (0);

  QString line = in.readAll ();

  if (line.size () != 0)
  {
    QStringList node, value;

    if (json_parse (line, &node, &value, nullptr))
    {
      const qint32 n = node.size ();
      for (qint32 counter = 0; counter < n; counter ++)
      {
        if (node.at (counter) == QLatin1String ("close"))
          realtimeprice.price = QString::number (value.at (counter).toFloat ());

        if (node.at (counter) == QLatin1String ("change"))
          realtimeprice.change = QString::number (value.at (counter).toFloat ());

        if (node.at (counter) == QLatin1String ("percent_change"))
          realtimeprice.prcchange =
            QString::number ((value.at (counter).simplified().toFloat ())) % QString ("%");

        if (node.at (counter) == QLatin1String ("previous_close"))
          realtimeprice.open = value[counter];

        if (realtimeprice.prcchange.toFloat () >= 0)
        {
          realtimeprice.low = realtimeprice.open;
          realtimeprice.high = realtimeprice.price;
        }
        else
        {
          realtimeprice.low = realtimeprice.price;
          realtimeprice.high = realtimeprice.open;
        }

        if (node.at (counter) == QLatin1String ("volume"))
        {
          realtimeprice.volume = value[counter].replace (CGLiteral (","), CGLiteral (""));
          if (realtimeprice.volume.right (1) == QLatin1String ("M"))
            realtimeprice.volume.replace (CGLiteral ("M"), CGLiteral ("000000"));
          if (realtimeprice.volume.right (1) == QLatin1String ("K"))
            realtimeprice.volume.replace (CGLiteral ("K"), CGLiteral ("000"));
        }

        if (node.at (counter) == QLatin1String ("datetime"))
        {
          realtimeprice.date = value.at (counter);
          realtimeprice.time = CGLiteral ("00:00:00");
        }
      }
    }
    else
      result = CG_ERR_INVALID_DATA;
  }

  setGlobalError(result, __FILE__, __LINE__);
  rtprice = realtimeprice;
  tempFile.get ()->close ();

  if (result == CG_ERR_OK)
    updatePrice (rtprice);

  return result;
}

// check if symbol exists
bool
TwelveDataFeed::symbolExistence (QString & symbol, QString & name,
                                 QString & market, QString & currency,
                                 const QString & type)
{
  if (netservice == nullptr)
    return false;

  CG_ERR_RESULT ioresult = CG_ERR_OK;
  bool result = false;

  symbol = symbol.trimmed ();
  if (!validSymbol (symbol))
    return result;

  const QString urlstr = symbolURL (symbol, type);
  if (urlstr.size () == 0)
    return result;

  Symbol.truncate (0);

  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
    return result;

  // open temporary file
  if (!tempFile.get ()->open ())
  {
    setGlobalError(CG_ERR_CREATE_TEMPFILE, __FILE__, __LINE__);
    return result;
  }
  tempFile.get ()->resize (0);

  ioresult = netservice->httpGET (urlstr, *tempFile.get (), nullptr);
  if (ioresult != CG_ERR_OK)
  {
    setGlobalError(ioresult, __FILE__, __LINE__);
    return result;
  }

  QTextStream in;

  in.setDevice (tempFile.get ());
  in.seek (0);

  QString line = in.readAll ();

  if (line.size () != 0)
  {
    QStringList node, value;

    symbolName = Market = CGLiteral ("");
    if (json_parse (line, &node, &value, nullptr))
    {
      for (qint32 counter = 0; counter < node.size (); counter ++)
      {
        if (node.at (counter) == QLatin1String ("name"))
          symbolName = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("exchange"))
          Market = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("currency"))
          Currency = value.at (counter).simplified();
      }
      name = symbolName;
      market = Market;
      currency = Currency;
      Symbol = symbol;

      if (name.length () > 0)
        result = true;
      else
        result = false;
    }
    else
      ioresult = CG_ERR_INVALID_DATA;
  }
  else
    result = false;

  setGlobalError(ioresult, __FILE__, __LINE__);

  if (result == false)
    Symbol.truncate (0);

  tempFile.get ()->close ();

  return result;
}

// download historical data
CG_ERR_RESULT
TwelveDataFeed::downloadData (QString symbol, QString type, QString timeframe,
                              QString currency, QString task, bool adjust)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  QString url;
  CG_ERR_RESULT result = CG_ERR_OK;

  if (Application_Settings->options.tdapikey.isEmpty ())
    return CG_ERR_NO_API_KEY;

  if (GlobalProgressBar != nullptr)
    GlobalProgressBar->setValue (0);

  // check symbol existence
  if (symbol != Symbol)
  {
    type = CGLiteral ("STOCKS");
    if (!symbolExistence (symbol, symbolName, Market, currency, type))
    {
      type = CGLiteral ("INDICES");
      if (!symbolExistence (symbol, symbolName, Market, currency, type))
      {
        result = GlobalError.fetchAndAddAcquire (0);
        if (result == CG_ERR_OK)
          result = CG_ERR_NOSYMBOL;
        return result;
      }
    }
  }

  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
    return CG_ERR_NOMEM;

  // open temporary file
  if (!tempFile.get ()->open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }
  tempFile.get ()->resize (0);

  // fill symbol entry
  entry.symbol = Symbol;
  entry.timeframe = timeframe;
  entry.csvfile = tempFile.get ()->fileName ();
  entry.source = CGLiteral ("TWELVEDATA");
  entry.format = CGLiteral ("TWELVEDATA CSV");
  entry.currency = currency;
  entry.name = symbolName;
  entry.market = Market;
  entry.adjust = adjust;
  entry.tablename = entry.symbol % CGLiteral ("_") %
                    entry.market % CGLiteral ("_") %
                    entry.source % CGLiteral ("_");
  entry.tablename += entry.timeframe;
  entry.tmptablename = CGLiteral ("TMP_") % entry.tablename;
  url = downloadURL (symbol, Market);
  entry.dnlstring = url;

  result = netservice->httpGET (url, *tempFile.get (), nullptr);
  if (result != CG_ERR_OK)
    return result;

  if (GlobalProgressBar != nullptr)
    GlobalProgressBar->setValue (33);
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);

  if (tempFile.get ()->size () != 0)
  {
    if (GlobalProgressBar != nullptr)
    {
      GlobalProgressBar->setValue (50);
      qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);
    }

    if (GlobalProgressBar != nullptr)
    {
      GlobalProgressBar->setValue (66);
      qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);
    }

    if (task == QLatin1String ("DOWNLOAD"))
      result = csv2sqlite (&entry, CGLiteral ("CREATE"));
    else
      result = csv2sqlite (&entry, CGLiteral ("UPDATE"));

    if (GlobalProgressBar != nullptr)
    {
      GlobalProgressBar->setValue (100);
      qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);
    }
  }

  tableName = entry.tablename;
  setGlobalError(result, __FILE__, __LINE__);
  tempFile.get ()->close ();
  return result;
}

// download statistics
CG_ERR_RESULT
TwelveDataFeed::downloadStats (QString symbol)
{
  Q_UNUSED (symbol);
  return CG_ERR_OK;
}

TwelveDataSymbolVector
TwelveDataFeed::getSymbols ()
{
  if (netservice == nullptr)
    return TwelveDataSymbols;

  if (TwelveDataSymbols.size () > 0)
    return TwelveDataSymbols;

  QString url, inputline;
  QStringList column;

  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
    return TwelveDataSymbols;

  // open temporary file
  if (!tempFile.get ()->open ())
    return TwelveDataSymbols;

  inputline.reserve (512);

  // stocks: we'll keep us stocks only
  tempFile.get ()->resize (0);
  url = CGLiteral ("https://api.twelvedata.com/stocks?format=CSV&country=usa");
  if (CG_ERR_OK != netservice->httpGET (url, *tempFile.get (), nullptr))
    return TwelveDataSymbols;

  QTextStream in (tempFile.get ());
  in.seek (0);

  inputline = in.readLine (0);    // header

  if (!in.atEnd ())
    inputline = in.readLine (0);  // first line

  while (!in.atEnd ())
  {
    column = inputline.split (CGLiteral(";"), QString::KeepEmptyParts);

    if (column.size () >= 6)
    {
      TwelveDataSymbol symbol;

      symbol.symbol 	=  column[0].replace ("'", " ").toUpper ();
      symbol.name   	=  column[1].replace ("'", " ").toUpper ();
      symbol.currency	=  column[2].replace ("'", " ").toUpper ();
      symbol.market		=  column[3].replace ("'", " ").toUpper ();
      symbol.country	=  column[4].replace ("'", " ").toUpper ();
      symbol.type 		=  CGLiteral ("STOCKS");

      TwelveDataSymbols += symbol;
    }

    inputline = in.readLine (0);
  }

  // etfs:
  // Their etf reference is limited,
  // no way to guarantee uniqueness from reference data only,
  // remains out until the situation improves

  /*
    tempFile.resize (0);
    url = CGLiteral ("https://api.twelvedata.com/etf?format=CSV");
    if (CG_ERR_OK != netservice->httpGET (url, *tempFile.get (), nullptr))
      goto getSymbols_end;

    in.seek (0);
    inputline = in.readLine (0);    // header

    if (!in.atEnd ())
      inputline = in.readLine (0);  // first line

    while (!in.atEnd ())
    {
      column = inputline.split (CGLiteral(";"), QString::KeepEmptyParts);
      if (column.size () >= 3)
      {
  	  TwelveDataSymbol symbol;

        symbol.symbol 	=  column[0].replace ("'", " ").toUpper ();
        symbol.name   	=  column[1].replace ("'", " ").toUpper ();
        symbol.currency	=  column[2].replace ("'", " ").toUpper ();
        symbol.market		=  CGLiteral ("ETF");
        symbol.country	=  CGLiteral ("ETF");
        symbol.type 		=  CGLiteral ("ETF");

        TwelveDataSymbols += symbol;
      }

      inputline = in.readLine (0);
    }
  */

// indices: we'll keep us index only
  tempFile.get ()->resize (0);
  url = CGLiteral ("https://api.twelvedata.com/indices?format=CSV");
  if (CG_ERR_OK != netservice->httpGET (url, *tempFile.get (), nullptr))
    return TwelveDataSymbols;

  in.seek (0);
  inputline = in.readLine (0);    // header

  if (!in.atEnd ())
    inputline = in.readLine (0);  // first line

  while (!in.atEnd ())
  {
    column = inputline.split (CGLiteral(";"), QString::KeepEmptyParts);
    if (column.size () >= 4)
    {
      TwelveDataSymbol symbol;

      symbol.symbol 	=  column[0].replace ("'", " ").toUpper ();
      symbol.name   	=  column[1].replace ("'", " ").toUpper ();
      symbol.country	=  column[2].replace ("'", " ").toUpper ();
      symbol.currency	=  column[3].replace ("'", " ").toUpper ();
      symbol.market		=  CGLiteral ("INDEX");
      symbol.type 		=  CGLiteral ("INDICES");

      TwelveDataSymbols += symbol;
    }

    inputline = in.readLine (0);
  }

  tempFile.get ()->close ();
  return TwelveDataSymbols;
}

// populate the symlist
CG_ERR_RESULT
TwelveDataFeed::populateSymlist ()
{
  if (populated)
    return CG_ERR_OK;

  int result = CG_ERR_OK;
  TwelveDataSymbolVector  symbols = getSymbols ();

  if (symbols.size () == 0)
    return CG_ERR_INVALID_DATA;

  QString SQL =
    CGLiteral ("DELETE FROM symlist_twelve_data;") +
    CGLiteral ("DELETE FROM symlist_twelve_data_tmp;");
  SQL.append ('\n');

  int rowcounter = 0, rc;
  foreach (const TwelveDataSymbol symbol, symbols)
  {
    if (symbol.symbol.size () > 0 && symbol.name.size () > 0)
    {
      const QString insertsym =
        CGLiteral ("INSERT INTO symlist_twelve_data_tmp (SYMBOL, DESCRIPTION, MARKET, CURRENCY, COUNTRY, TYPE) VALUES ('") %
        symbol.symbol % CGLiteral ("','") %
        symbol.name % CGLiteral ("','") %
        symbol.market % CGLiteral ("','") %
        symbol.currency % CGLiteral ("','") %
        symbol.country % CGLiteral ("','") %
        symbol.type % CGLiteral ("');");
      SQL += insertsym;
      SQL.append ('\n');

      rowcounter ++;
      if ((rowcounter % 10000) == 0)
      {
        rc = updatedb (SQL);
        if (rc != SQLITE_OK)
        {
          result = CG_ERR_TRANSACTION;
          setGlobalError(result, __FILE__, __LINE__);
          return result;
        }
        SQL.truncate (0);
      }
    }
  }

  SQL +=
    CGLiteral ("DELETE FROM symlist_twelve_data_tmp WHERE ") %
    CGLiteral ("country NOT LIKE 'UNITED STATES%';");
  SQL.append ('\n');

  SQL +=
    CGLiteral ("INSERT INTO symlist_twelve_data (symbol, country, market, currency, description, type) ") %
    CGLiteral ("SELECT symbol, country, market, currency, description, type FROM symlist_twelve_data_tmp ") %
    CGLiteral ("GROUP BY symbol, country, market, currency, description, type;");
  SQL.append ('\n');

  SQL += CGLiteral ("UPDATE VERSION SET TWELVEDATAUPDATE = strftime('%s', 'now'); ");

  // execute sql
  rc = updatedb (SQL);
  if (rc != SQLITE_OK)
  {
    result = CG_ERR_TRANSACTION;
    setGlobalError(result, __FILE__, __LINE__);

    return result;
  }

  SQL = CGLiteral ("INSERT INTO currencies (symbol) ") %
        CGLiteral ("SELECT DISTINCT currency FROM symlist_twelve_data ") %
        CGLiteral ("WHERE LENGTH (currency) = 3 AND ") %
        CGLiteral ("currency NOT IN (SELECT symbol FROM currencies);");

  rc = updatedb (SQL);
  if (rc != SQLITE_OK)
  {
    result = CG_ERR_TRANSACTION;
    setGlobalError(result, __FILE__, __LINE__);

    return result;
  }

  TwelveDataSymbols.clear ();
  populated = true;
  return result;
}
