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
#include <QByteArray>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDateTime>
#include "feedav.h"
#include "common.h"

// constructor
AlphaVantageFeed::AlphaVantageFeed (QObject *parent)
{
  tableName.truncate (0);
  symbolName.truncate (0);

  if (parent != nullptr)
    setParent (parent);

  netservice = MasterNetService;
}

// destructor
AlphaVantageFeed::~AlphaVantageFeed ()
{

  return;
}

// validate symbol
bool
AlphaVantageFeed::validSymbol (QString symbol)
{
  for (qint32 counter = 0, max = symbol.size ();
       counter < max; counter ++)
  {
    if (!((symbol[counter] >= 'A' && symbol[counter] <= 'Z') ||
          (symbol[counter] >= 'a' && symbol[counter] <= 'z') ||
          (symbol[counter] >= '0' && symbol[counter] <= '9') ||
          symbol[counter] == '.' || symbol[counter] == '='  ||
          symbol[counter] == '^' || symbol[counter] == '-'))
      return false;
  }

  return true;
}

// return symbol check URL
//https://www.alphavantage.co/query?function=SYMBOL_SEARCH&keywords=MSFT&apikey=key
QString
AlphaVantageFeed::symbolURL (QString symbol)
{
  QString urlstr = CGLiteral ("");

  if (symbol.size () == 0)
    return urlstr;

  urlstr =
    CGLiteral ("https://www.alphavantage.co/query?function=SYMBOL_SEARCH&keywords=");
  urlstr += symbol;
  urlstr += CGLiteral ("&apikey=");
  urlstr += Application_Settings->options.avapikey;
  return urlstr;
}

// alpha vantage does not support this info
QString
AlphaVantageFeed::symbolCurrencyURL (QString symbol)
{
  Q_UNUSED (symbol);
  QString urlstr = CGLiteral ("");

  return urlstr;
}

// return symbol statistics URL
// alpha vantage does not support this info
QString
AlphaVantageFeed::symbolStatsURL (QString symbol)
{
  Q_UNUSED (symbol);
  QString urlstr = CGLiteral ("");

  return urlstr;
}

// return symbol statistics URL
// alpha vantage does not support this info
QString
AlphaVantageFeed::symbolStatsURLjson (QString symbol)
{
  Q_UNUSED (symbol);
  QString urlstr = CGLiteral ("");

  return urlstr;
}

// return historical download URL
// eg: https://www.alphavantage.co/query?function=TIME_SERIES_DAILY_ADJUSTED&
//     symbol=MSFT&outputsize=full&apikey=4PYMW8I7CBQHSSEA&datatype=csv
QString
AlphaVantageFeed::downloadURL (QString symbol)
{
  QString downstr = CGLiteral ("");

  if (symbol.size () == 0)
    return downstr;

  downstr =
    CGLiteral ("https://www.alphavantage.co/query?function=TIME_SERIES_DAILY_ADJUSTED&symbol=");
  downstr += symbol;
  downstr += CGLiteral ("&outputsize=full&datatype=csv&apikey=");
  downstr += Application_Settings->options.avapikey;

  return downstr;
}

// return real time price URL using csv api
QString
AlphaVantageFeed::realTimePriceURL (QString symbol)
{
  return realTimePriceURLjson (symbol) % CGLiteral ("&datatype=csv");
}

// return real time price URL using json api
QString
AlphaVantageFeed::realTimePriceURLjson (QString symbol)
{
  QString urlstr = CGLiteral ("");

  if (symbol.size () == 0)
    return urlstr;

  urlstr =
    CGLiteral ("https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=");
  urlstr += symbol;
  urlstr += CGLiteral ("&apikey=");
  urlstr += Application_Settings->options.avapikey;
  return urlstr;
}

// return update URL
QString
AlphaVantageFeed::updateURL (QString symbol)
{
  return downloadURL (symbol);
}

// get real time price
CG_ERR_RESULT
AlphaVantageFeed::getRealTimePrice (QString symbol, RTPrice & rtprice, API api)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  Q_UNUSED (api);
  QTemporaryFile tempFile;      // temporary file
  RTPrice realtimeprice;        // real time price
  QTextStream in;
  QString url, line;
  QStringList token, token2;
  CG_ERR_RESULT result = CG_ERR_OK;

  url = realTimePriceURLjson (symbol);

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
      for (qint32 counter = 0; counter < node.size (); counter ++)
      {
        if (node.at (counter) == QLatin1String ("01. symbol"))
        {
          realtimeprice.symbol = value.at (counter);
          realtimeprice.feed = CGLiteral ("ALPHAVANTAGE");
        }

        if (node.at (counter) == QLatin1String ("02. open"))
          realtimeprice.open = value.at (counter);

        if (node.at (counter) == QLatin1String ("03. high"))
          realtimeprice.high = value.at (counter);

        if (node.at (counter) == QLatin1String ("04. low"))
          realtimeprice.low = value.at (counter);

        if (node.at (counter) == QLatin1String ("05. price"))
          realtimeprice.price = value.at (counter);

        if (node.at (counter) == QLatin1String ("06. volume"))
          realtimeprice.volume = value.at (counter);

        if (node.at (counter) == QLatin1String ("07. latest trading day"))
        {
          realtimeprice.date = value.at (counter);
          realtimeprice.time = CGLiteral ("00:00.00");
        }

        if (node.at (counter) == QLatin1String ("09. change"))
          realtimeprice.change = value.at (counter);

        if (node.at (counter) == QLatin1String ("10. change percent"))
          realtimeprice.prcchange = value.at (counter);
      }
    }
    else
      result = CG_ERR_INVALID_DATA;
  }
  else
    result = CG_ERR_INVALID_DATA;

getRealTimePrice_end:
  setGlobalError(result, __FILE__, __LINE__);
  realtimeprice.symbol = symbol;
  realtimeprice.feed = CGLiteral ("ALPHAVANTAGE");
  rtprice = realtimeprice;
  tempFile.close ();
  if (result == CG_ERR_OK)
    updatePrice (rtprice);
  return result;
}

// download statistics
// alpha vantage does not support this info
CG_ERR_RESULT
AlphaVantageFeed::downloadStats (QString symbol, API api)
{
  Q_UNUSED (api);
  Q_UNUSED (symbol);

  CG_ERR_RESULT result = CG_ERR_OK;
  return  result;
}

// check if symbol exists
bool
AlphaVantageFeed::symbolExistence (QString & symbol, QString & name, QString & market,
                                   QString & currency)
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

    if (json_parse (line, &node, &value, nullptr))
    {
      if (node.size () < 9)
      {
        result = false;
        goto symbolExistence_end;
      }

      symbolName = Market = Currency = CGLiteral ("");
      for (qint32 counter = 0; counter < node.size () && counter < 9; counter ++)
      {
        if (node.at (counter) == QLatin1String ("1. symbol"))
          Symbol = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("2. name") &&
            symbolName.isEmpty ())
          symbolName = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("4. region"))
          Market = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("8. currency"))
          Currency = value.at (counter).simplified();
      }

      symbol = Symbol;
      name = symbolName;
      market = Market;
      currency = Currency;

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
AlphaVantageFeed::downloadData (QString symbol, QString timeframe, QString currency,
                                QString task, bool adjust)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  QTemporaryFile tempFile;      // temporary file
  QString url;
  CG_ERR_RESULT result = CG_ERR_OK;

  if (Application_Settings->options.avapikey.isEmpty ())
  {
    result = CG_ERR_NO_API_KEY;
    goto downloadData_end;
  }

  if (GlobalProgressBar != nullptr)
    GlobalProgressBar->setValue (0);

  // check symbol existence
  if (symbol != Symbol)
  {
    if (!symbolExistence (symbol, entry.name, entry.market, currency))
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
  entry.source = CGLiteral("ALPHAVANTAGE");
  entry.format = CGLiteral("YAHOO CSV");
  entry.currency = currency;
  entry.name = symbolName;
  entry.market = Market;
  entry.adjust = adjust;
  entry.tablename = entry.symbol % CGLiteral("_") %
                    entry.market % CGLiteral("_") %
                    entry.source % CGLiteral("_");

  entry.tablename += entry.timeframe;
  entry.tmptablename = CGLiteral("TMP_") + entry.tablename;
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
    result = downloadStats (Symbol,  AlphaVantageFeed::CSV /* AlphaVantageFeed::JSON */);
    if (result != CG_ERR_OK)
      goto downloadData_end;

    if (GlobalProgressBar != nullptr)
      GlobalProgressBar->setValue (50);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);

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
      GlobalProgressBar->setValue (66);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);

    if (task == QLatin1String ("DOWNLOAD"))
      result = csv2sqlite (&entry, CGLiteral ("CREATE"));
    else
      result = csv2sqlite (&entry, CGLiteral ("UPDATE"));
  }

  if (GlobalProgressBar != nullptr)
    GlobalProgressBar->setValue (100);
  qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);

downloadData_end:
  tableName = entry.tablename;
  setGlobalError(result, __FILE__, __LINE__);
  tempFile.close ();
  return result;
}
