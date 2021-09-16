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

#include <QApplication>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDateTime>
#include "feediex.h"
#include "common.h"

// constructor
IEXFeed::IEXFeed (QObject *parent)
{
  if (parent != nullptr)
    setParent (parent);

  populated = false;

  netservice = MasterNetService;
}

// destructor
IEXFeed::~IEXFeed ()
{

  return;
}

// validate symbol
bool
IEXFeed::validSymbol (QString symbol)
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
// eg old api: https://api.iextrading.com/1.0/stock/aapl/quote
// eg iex cloud: https://cloud.iexapis.com/v1/stock/aapl/quote/?token=TOKEN
QString
IEXFeed::symbolURL (QString symbol)
{
  QString urlstr = CGLiteral ("");

  if (symbol.size () == 0)
    return urlstr;

  urlstr = CGLiteral ("https://cloud.iexapis.com/v1/stock/") % symbol %
           CGLiteral ("/quote") %
           CGLiteral ("/?token=") % Application_Settings->options.iexapikey;
  return urlstr;
}

QString
IEXFeed::realTimePriceURL (QString symbol)
{
  return symbolURL (symbol);
}

// return symbol statistics URL from http
QString
IEXFeed::symbolStatsURL (QString symbol)
{
  QString urlstr = CGLiteral ("");

  if (symbol.size () == 0)
    return urlstr;

  urlstr = CGLiteral ("https://cloud.iexapis.com/v1/stock/");
  urlstr += symbol;
  urlstr += CGLiteral ("/stats");
  urlstr += CGLiteral ("/?token=") % Application_Settings->options.iexapikey;
  return urlstr;
}

// return historical download URL
// eg old api:
// https://api.iextrading.com/1.0/stock/GE/chart/5y
// eg iex cloud:
// https://cloud.iexapis.com/v1/stock/aapl/chart/5y/?token=TOKEN
QString
IEXFeed::downloadURL (QString symbol)
{
  QString downstr = CGLiteral ("");

  if (symbol.size () == 0)
    return downstr;

  downstr = CGLiteral ("https://cloud.iexapis.com/v1/stock/") %
            symbol % CGLiteral ("/chart/5y") %
            CGLiteral ("/?token=") % Application_Settings->options.iexapikey;

  return downstr;
}

// return update URL
QString
IEXFeed::updateURL (QString symbol)
{
  return downloadURL (symbol);
}

// get real time price
CG_ERR_RESULT
IEXFeed::getRealTimePrice (QString symbol, RTPrice & rtprice)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  QDateTime timestamp;
  QTemporaryFile tempFile;      // temporary file
  RTPrice realtimeprice;        // real time price
  QTextStream in;
  QString url, line;
  CG_ERR_RESULT result = CG_ERR_OK;

  url = realTimePriceURL (symbol);

  // open temporary file
  if (!tempFile.open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    goto getRealTimePrice_end;
  }
  tempFile.resize (0);

  result = netservice->httpGET (url, tempFile, nullptr);
  if (result != CG_ERR_OK)
    goto getRealTimePrice_end;

  in.setDevice (&tempFile);
  in.seek (0);
  line = in.readAll ();

  if (line.size () != 0)
  {
    QStringList node, value;

    if (json_parse (line, &node, &value, nullptr))
    {
      const qint32 n = node.size ();
      for (qint32 counter = 0; counter < n; counter ++)
      {
        if (node.at (counter) == QLatin1String ("latestPrice"))
          realtimeprice.price = QString::number (value.at (counter).toFloat ());

        if (node.at (counter) == QLatin1String ("change"))
          realtimeprice.change = QString::number (value.at (counter).toFloat ());

        if (node.at (counter) == QLatin1String ("changePercent"))
          realtimeprice.prcchange =
            QString::number ((value.at (counter).simplified().toFloat () * 100)) % QString ("%");

        if (node.at (counter) == QLatin1String ("previousClose"))
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

        if (node.at (counter) == QLatin1String ("iexVolume"))
        {
          realtimeprice.volume = value[counter].replace (CGLiteral (","), CGLiteral (""));
          if (realtimeprice.volume.right (1) == QLatin1String ("M"))
            realtimeprice.volume.replace (CGLiteral ("M"), CGLiteral ("000000"));
          if (realtimeprice.volume.right (1) == QLatin1String ("K"))
            realtimeprice.volume.replace (CGLiteral ("K"), CGLiteral ("000"));
        }

        if (node.at (counter) == QLatin1String ("latestUpdate"))
        {
          const unsigned int s = value.at (counter).left (10).toUInt ();
          timestamp.setTime_t(s);
          realtimeprice.date = timestamp.toString("yyyy-MM-dd");
          realtimeprice.time = timestamp.toString("hh:mm:ss");
        }
      }
    }
    else
      result = CG_ERR_INVALID_DATA;
  }

getRealTimePrice_end:
  setGlobalError(result, __FILE__, __LINE__);
  realtimeprice.symbol = symbol;
  realtimeprice.feed = CGLiteral ("IEX");
  rtprice = realtimeprice;
  tempFile.close ();
  if (result == CG_ERR_OK)
    updatePrice (rtprice);
  return result;
}

// check if symbol exists
bool
IEXFeed::symbolExistence (QString & symbol, QString & name, QString & market)
{
  if (netservice == nullptr)
    return false;

  QTemporaryFile tempFile;      // temporary file
  QTextStream in;
  QString urlstr, line;
  QStringList token;
  CG_ERR_RESULT ioresult = CG_ERR_OK;
  bool result = false;

  symbol = symbol.trimmed ();
  if (!validSymbol (symbol))
    goto symbolExistence_end;

  urlstr = symbolURL (symbol);
  if (urlstr.size () == 0)
    goto symbolExistence_end;

  Symbol = symbol;
  symbolName = name;

  // open temporary file
  if (!tempFile.open ())
  {
    ioresult = CG_ERR_CREATE_TEMPFILE;
    goto symbolExistence_end;
  }
  tempFile.resize (0);

  ioresult = netservice->httpGET (urlstr, tempFile, nullptr);
  if (ioresult != CG_ERR_OK)
    goto symbolExistence_end;

  in.setDevice (&tempFile);
  in.seek (0);
  line = in.readAll ();

  if (line.size () != 0)
  {
    QStringList node, value;

    symbolName = Market = CGLiteral ("");
    if (json_parse (line, &node, &value, nullptr))
    {
      for (qint32 counter = 0; counter < node.size (); counter ++)
      {
        if (node.at (counter) == QLatin1String ("companyName"))
          symbolName = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("primaryExchange"))
          Market = value.at (counter).simplified();
      }
      name = symbolName;
      market = Market;

      result = true;
    }
    else
      ioresult = CG_ERR_INVALID_DATA;
  }
  else
    result = false;

symbolExistence_end:
  setGlobalError(ioresult, __FILE__, __LINE__);

  if (result == false)
    Symbol.truncate (0);

  tempFile.close ();

  return result;
}

// download historical data
CG_ERR_RESULT
IEXFeed::downloadData (QString symbol, QString timeframe, QString currency,
                       QString task, bool adjust)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  QTemporaryFile tempFile;      // temporary file
  QString url;
  CG_ERR_RESULT result = CG_ERR_OK;

  if (Application_Settings->options.iexapikey.isEmpty ())
  {
    result = CG_ERR_NO_API_KEY;
    goto downloadData_end;
  }

  if (GlobalProgressBar != nullptr)
    GlobalProgressBar->setValue (0);

  // check symbol existence
  if (symbol != Symbol)
  {
    if (!symbolExistence (symbol, symbolName, Market))
    {
      result = GlobalError.fetchAndAddAcquire (0);
      if (result == CG_ERR_OK)
        result = CG_ERR_NOSYMBOL;
      return result;
    }
  }

  // open temporary file
  if (!tempFile.open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }
  tempFile.resize (0);

  // fill symbol entry
  entry.symbol = Symbol;
  entry.timeframe = timeframe;
  entry.csvfile = tempFile.fileName ();
  entry.source = CGLiteral ("IEX");
  entry.format = CGLiteral ("IEX JSON");
  entry.currency = currency;
  entry.name = symbolName;
  entry.market = Market;
  entry.adjust = adjust;
  entry.tablename = entry.symbol % CGLiteral ("_") %
                    entry.market % CGLiteral ("_") %
                    entry.source % CGLiteral ("_");
  entry.tablename += entry.timeframe;
  entry.tmptablename = CGLiteral ("TMP_") % entry.tablename;
  url = downloadURL (symbol);
  entry.dnlstring = url;

  result = netservice->httpGET (url, tempFile, nullptr);
  if (result != CG_ERR_OK)
    goto downloadData_end;

  if (GlobalProgressBar != nullptr)
    GlobalProgressBar->setValue (33);
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);

  if (tempFile.size () != 0)
  {
    result = downloadStats (Symbol);
    if (result != CG_ERR_OK)
      goto downloadData_end;

    if (GlobalProgressBar != nullptr)
    {
      GlobalProgressBar->setValue (50);
      qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);
    }

    entry.BookValue = BookValue;
    entry.MarketCap = MarketCap;
    entry.EBITDA = EBITDA;
    entry.PE = PE;
    entry.PEG = PEG;
    entry.Yield = Yield;
    entry.EPScy = EPScy;
    entry.EPSny = EPSny;
    entry.ESh = ESh;
    entry.PS = PS;
    entry.PBv = PBv;

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

downloadData_end:
  tableName = entry.tablename;
  setGlobalError(result, __FILE__, __LINE__);
  tempFile.close ();
  return result;
}

// download statistics
CG_ERR_RESULT
IEXFeed::downloadStats (QString symbol)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  QTemporaryFile tempFile;      // temporary file
  QTextStream in;
  QString url, line;
  CG_ERR_RESULT result = CG_ERR_OK;

  url = symbolStatsURL (symbol);

  // open temporary file
  if (!tempFile.open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    goto downloadStats_end;
  }
  tempFile.resize (0);

  result = netservice->httpGET (url, tempFile, nullptr);
  if (result != CG_ERR_OK)
    goto downloadStats_end;

  in.setDevice (&tempFile);
  in.seek (0);
  line = in.readAll ();

  if (line.size () != 0)
  {
    QStringList node, value;
    if (json_parse (line, &node, &value, nullptr))
    {
      const qint32 n = node.size ();
      for (qint32 counter = 0; counter < n; counter ++)
      {
        if (node.at (counter) == QLatin1String ("bookvalue"))
          BookValue = value.at (counter);
        else if (node.at (counter) == QLatin1String ("marketcap"))
          MarketCap = QString("%1").arg (value.at (counter).toFloat (), 0, 'f', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("EBITDA"))
          EBITDA = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("peRatioHigh"))
          PE = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("peg"))
          PEG = value.at (counter);
        else if (node.at (counter) == QLatin1String ("dividendYield"))
          Yield = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("latestEPS"))
          EPScy = QString("%1").arg (value.at (counter).toDouble (), 0, 'f', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("epsny"))
          EPSny = QString("%1").arg (value.at (counter).toDouble (), 0, 'f', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("es"))
          ESh = value.at (counter);
        else if (node.at (counter) == QLatin1String ("priceToSales"))
          PS = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("priceToBook"))
          PBv = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
      }
    }
    else
      result = CG_ERR_INVALID_DATA;
  }

downloadStats_end:
  setGlobalError(result, __FILE__, __LINE__);
  tempFile.close ();
  return result;
}

IEXSymbolVector
IEXFeed::getSymbols ()
{
  if (netservice == nullptr)
    return IEXSymbols;

  if (IEXSymbols.size () > 0)
    return IEXSymbols;

  QTemporaryFile tempFile;      // temporary file
  QTextStream in;
  QString url, line;

  url = CGLiteral ("https://api.iextrading.com/1.0/ref-data/symbols");

  // open temporary file
  if (!tempFile.open ())
    goto getSymbols_end;
  tempFile.resize (0);

  if (CG_ERR_OK != netservice->httpGET (url, tempFile, nullptr))
    goto getSymbols_end;

  in.setDevice (&tempFile);
  in.seek (0);
  line = in.readAll ();

  if (line.size () != 0)
  {
    QStringList node, value, jsonsymbols;
    line.replace ("[{", "{");
    line.replace ("}]", "}");
    jsonsymbols = line.split("},");

    foreach (QString sym, jsonsymbols)
    {
      sym += CGLiteral ("}");

      IEXSymbol symbol;
      if (json_parse (sym, &node, &value, nullptr))
      {
        const qint32 n = node.size ();
        symbol.symbol = symbol.name = CGLiteral ("");

        for (qint32 counter = 0; counter < n; counter ++)
        {
          if (node.at (counter) == QLatin1String ("symbol"))
            symbol.symbol = value.at (counter);
          else if (node.at (counter) == QLatin1String ("name"))
            symbol.name = value.at (counter).left (50);
        }
      }

      symbol.name.replace ("'", "`");
      IEXSymbols += symbol;
    }
  }

getSymbols_end:
  tempFile.close ();
  return IEXSymbols;
}

// populate the symlist
CG_ERR_RESULT
IEXFeed::populateSymlist ()
{
  if (populated)
    return CG_ERR_OK;

  IEXSymbolVector symbols;
  int result = CG_ERR_OK;

  symbols = getSymbols ();

  if (symbols.size () == 0)
    return CG_ERR_INVALID_DATA;

  QString SQL = "";

  SQL = CGLiteral ("DELETE FROM symlist_iex;");
  SQL.append ('\n');
  foreach (const IEXSymbol symbol, symbols)
  {
    QString insertsym;
    if (symbol.symbol.size () > 0 && symbol.name.size () > 0)
    {
      insertsym = CGLiteral ("INSERT INTO symlist_iex (SYMBOL, DESCRIPTION, MARKET, URL1, URL2) VALUES ('") %
                  symbol.symbol % CGLiteral ("','") % symbol.name %
                  CGLiteral ("','N/A','','');");
      SQL += insertsym;
      SQL.append ('\n');
    }
  }
  SQL += CGLiteral ("UPDATE VERSION SET IEXUPDATE = strftime('%s', 'now'); ");

  // execute sql
  int rc = updatedb (SQL);
  if (rc != SQLITE_OK)
  {
    result = CG_ERR_TRANSACTION;
    setGlobalError(result, __FILE__, __LINE__);

    return result;
  }

  IEXSymbols.clear ();
  populated = true;
  return result;
}
