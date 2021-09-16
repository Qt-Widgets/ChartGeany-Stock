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

/* Possible solutions
   https://query2.finance.yahoo.com/v10/finance/quoteSummary/IBM?modules=financialData
   https://query1.finance.yahoo.com/v7/finance/quote?symbols=IBM
*/

#include <memory>

#include <QApplication>
#include <QTextStream>
#include <QTemporaryFile>
#include <QDateTime>
#include "feedyahoo.h"
#include "common.h"
#include "chartapp.h"

using std::unique_ptr;

static QString cookie (""), crumb ("");

// constructor
YahooFeed::YahooFeed (QObject *parent)
{
  tableName = Currency = symbolName = CGLiteral ("");

  if (parent != nullptr)
    setParent (parent);

  netservice = MasterNetService;
}

// destructor
YahooFeed::~YahooFeed ()
{

}

// validate symbol
bool
YahooFeed::validSymbol (QString &symbol)
{
  for (auto chr : symbol)
    if (!((chr >= 'A' && chr <= 'Z') ||
          (chr >= 'a' && chr <= 'z') ||
          (chr >= '0' && chr <= '9') ||
           chr == '.' || chr == '='  ||
           chr == '^' || chr == '-'))
      return false;
      
  return true;
}

// return symbol check URL
// eg: http://finance.yahoo.com/d/quotes.csv?s=IBM&f=nx
// yql:
// select * from csv where
// url='http://download.finance.yahoo.com/d/quotes.csv?s=IBM&f=nx'
// and columns='name,exchange'
QString
YahooFeed::symbolURL (QString &symbol)
{
  return symbolURLjson (symbol);
}

// https://query1.finance.yahoo.com/v7/finance/quote?symbols=IBM
QString
YahooFeed::symbolURLjson (QString &symbol)
{
  if (symbol.size () == 0)
    return CGLiteral ("");

  const QString
  urlstr = CGLiteral ("https://query1.finance.yahoo.com/v7/finance/quote?symbols=") %
           symbol;

  return urlstr;
}

// http://finance.yahoo.com/d/quotes.csv?s=RIO.AX&f=c4
QString
YahooFeed::symbolCurrencyURL (QString &symbol)
{
  return symbolURL (symbol);
}

// return symbol statistics URL from http
// eg: http://finance.yahoo.com/d/quotes.csv?s=IBM&f=b4j1j4rr5ye7e8ep5p6
// dividends
// eg: http://real-chart.finance.yahoo.com/table.csv?s=ABB&g=v
QString
YahooFeed::symbolStatsURL (QString &symbol)
{
  return symbolURL (symbol);
}

// return historical download URL
// eg:
// http://ichart.finance.yahoo.com/table.csv?s=GE&a=0&b=1&c=1960&d=0&e=31&f=2010&g=d&ignore=.csv DAY
// http://ichart.finance.yahoo.com/table.csv?s=GE&a=0&b=1&c=1960&d=0&e=31&f=2010&g=w&ignore=.csv WEEK
// http://ichart.finance.yahoo.com/table.csv?s=GE&a=0&b=1&c=1960&d=0&e=31&f=2010&g=m&ignore=.csv MONTH
//
// **** YQL Syntax ****
// select * from csv where url='http://ichart.finance.yahoo.com/table.csv?s=GE&a=0&b=1&c=1960&d=0&e=31&f=2010&g=d'
// **** YQL URL ****
//
// YQL XML output for one year
// http://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20yahoo.finance.historicaldata%20where%20symbol%20in%20%28%27GE%27%29%20and%20startDate%20=%20%271962-01-01%27%20and%20endDate%20=%20%271962-12-31%27&diagnostics=true&env=store://datatables.org/alltableswithkeys
// YQL JSON output
// http://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20csv%20where%20url%3D%27http%3A%2F%2Freal-chart.finance.yahoo.com%2Ftable.csv%3Fs%3DIBM%26d%3D9%26e%3D22%26f%3D2014%26g%3Dd%26a%3D0%26b%3D2%26c%3D1962%26ignore%3D.csv%27&format=json&callback=
// *** New Yahoo Historical Data API
// curl 'https://query1.finance.yahoo.com/v7/finance/download/%5EGSPC?period1=1492463852&period2=1495055852&interval=1d&events=history&crumb=XXXXXXX' -H 'user-agent: Mozilla/5.0 (compatible; MSIE 10.0; Windows NT 6.2; Trident/6.0)' -H 'cookie: B=YYYYYY;' -H 'referer: https://finance.yahoo.com/quote/%5EGSPC/history?p=%5EGSPC'
QString
YahooFeed::downloadURL (QString &symbol, QString &timeframe, QString &crumb)
{
  Q_UNUSED (timeframe);

  if (symbol.size () == 0)
    return CGLiteral ("");

  QDateTime today (QDate::currentDate()),
            startdate (QDate(1971, 1, 1));

  const QString
  downstr = CGLiteral ("https://query1.finance.yahoo.com/v7/finance/download/") % symbol %
            CGLiteral ("?period1=") % QString::number (startdate.toMSecsSinceEpoch() / 1000) %
            CGLiteral ("&period2=") %
            QString::number (today.toMSecsSinceEpoch() / 1000) %
            CGLiteral ("&interval=1d") %
            CGLiteral ("&events=history&crumb=") % crumb;

  return downstr;
}

// return real time price URL using http api
// eg:
// http://finance.yahoo.com/d/quotes.csv?s=RIO.AX&f=l1c6p2
QString
YahooFeed::realTimePriceURL (QString &symbol)
{
  return symbolURL (symbol);
}

// return update URL
QString
YahooFeed::updateURL (QString &symbol, QString &timeframe, QString &datefrom)
{
  Q_UNUSED (timeframe);

  if (symbol.size () == 0)
    return CGLiteral ("");

  const QStringList
  column = datefrom.split(CGLiteral ("-"), QString::KeepEmptyParts);

  const QString fromYear = column[0],
                fromMonth = CGLiteral ("0"),
                fromDay = column[2];

  const QString updstr =
    CGLiteral ("http://ichart.yahoo.com/table.csv?s=") %
    symbol % CGLiteral ("&a=") %
    fromMonth % CGLiteral ("&b=") %
    fromDay % CGLiteral ("&c=") %
    fromYear % CGLiteral ("&d=") %
    Month % CGLiteral ("&e=") %
    Day % CGLiteral ("&f=") %
    Year % CGLiteral ("&g=d") %
    CGLiteral ("&ignore=.csv");

  return updstr;
}

// get real time price
CG_ERR_RESULT
YahooFeed::getRealTimePrice (QString symbol, RTPrice & rtprice, YAHOO_API api)
{
  Q_UNUSED (api)

  return getRealTimePricejson (symbol, rtprice);
}

// get real time price
CG_ERR_RESULT
YahooFeed::getRealTimePricejson (QString symbol, RTPrice &realtimeprice)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  CG_ERR_RESULT result = CG_ERR_OK;

  // temporary file
  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
  {
    result = CG_ERR_NOMEM;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }

  // open temporary file
  if (!tempFile.get ()->open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }
  tempFile.get ()->resize (0);

  QString url = realTimePriceURL (symbol);
  result = netservice->httpGET (url, *tempFile.get (), nullptr);
  if (result != CG_ERR_OK)
  {
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }

  unique_ptr <QTextStream> in (new QTextStream (tempFile.get ()));
  in.get ()->seek (0);

  QString line = in.get ()->readAll ();

  if (line.size () != 0)
  {
    QStringList node, value;
    if (json_parse (line, &node, &value, nullptr))
    {
      for (qint32 counter = 0; counter < node.size (); counter ++)
      {
        if (node.at (counter).toLower () == QLatin1String ("regularmarketprice"))
        {
          realtimeprice.price = value[counter].remove ("+");
          realtimeprice.price = QString::number (realtimeprice.price.toDouble (), 'g', 8);
        }

        if (node.at (counter).toLower () == QLatin1String ("regularmarketchange"))
        {
          realtimeprice.change = value[counter].remove ("+");
          realtimeprice.change = QString::number (realtimeprice.change.toDouble (), 'g', 4);
        }

        if (node.at (counter).toLower () == QLatin1String ("regularmarketchangepercent"))
        {
          realtimeprice.prcchange = value[counter].simplified().remove ("+");
          realtimeprice.prcchange =
            QString::number (realtimeprice.prcchange.toFloat (), 'f', 2) % CGLiteral ("%");
        }

        if (node.at (counter).toLower () == QLatin1String ("regularmarketopen"))
        {
          realtimeprice.open = value[counter].simplified();
          realtimeprice.open = value[counter].remove ("+");
        }

        if (node.at (counter).toLower () == QLatin1String ("regularmarketdayhigh"))
        {
          realtimeprice.high = value[counter].simplified();
          realtimeprice.high = value[counter].remove ("+");
        }

        if (node.at (counter).toLower () == QLatin1String ("regularmarketdaylow"))
        {
          realtimeprice.low = value[counter].simplified();
          realtimeprice.low = value[counter].remove ("+");
        }

        if (node.at (counter).toLower () == QLatin1String ("regularmarketvolume"))
        {
          realtimeprice.volume = value[counter].simplified();
          realtimeprice.volume = value[counter].remove ("+");
        }

        if (node.at (counter).toLower () == QLatin1String ("regularmarkettime"))
        {
          QDateTime timestamp;
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)          
          const unsigned int s = value.at (counter).left (10).toUInt ();
#else
          const unsigned int s = value.at (counter).leftRef (10).toUInt ();
#endif          
          timestamp.setTime_t(s);
          realtimeprice.date = timestamp.toString("yyyy-MM-dd");
          realtimeprice.time = timestamp.toString("hh:mm:ss");
        }
      }
    }
    else
      result = CG_ERR_INVALID_DATA;

  }
  else
    result = CG_ERR_INVALID_DATA;

  setGlobalError(result, __FILE__, __LINE__);
  tempFile.get ()->close ();

  realtimeprice.symbol = symbol;
  realtimeprice.feed = CGLiteral ("YAHOO");

  if (result == CG_ERR_OK)
    updatePrice (realtimeprice);

  return result;
}

// download statistics
CG_ERR_RESULT
YahooFeed::downloadStats (QString symbol, YAHOO_API api)
{
  Q_UNUSED (api)

  return downloadStatsjson (symbol);
}

// download statistics
CG_ERR_RESULT
YahooFeed::downloadStatsjson (QString symbol)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  CG_ERR_RESULT result = CG_ERR_OK;

  // temporary file
  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
  {
    result = CG_ERR_NOMEM;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }

  // open temporary file
  if (!tempFile.get ()->open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }
  tempFile.get ()->resize (0);

  QString url = symbolStatsURL (symbol);
  result = netservice->httpGET (url, *tempFile.get (), nullptr);
  if (result != CG_ERR_OK)
  {
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }

  unique_ptr <QTextStream> in (new QTextStream (tempFile.get ()));
  in.get ()->seek (0);

  QString line = in.get ()->readAll ();

  if (line.size () != 0)
  {
    QStringList node, value;
    if (json_parse (line, &node, &value, nullptr))
    {
      for (qint32 counter = 0; counter < node.size (); counter ++)
      {
        if (node.at (counter) == QLatin1String ("bookValue"))
          entry.BookValue = QString("%1").arg (value.at (counter).toDouble (), 0, 'f', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("marketCap"))
          entry.MarketCap = QString("%1").arg (value.at (counter).toFloat (), 0, 'f', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("ebitda"))
          entry.EBITDA = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("trailingPE"))
          entry.PE = QString("%1").arg (value.at (counter).toFloat (), 0, 'f', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("peg"))
          entry.PEG = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("trailingAnnualDividendYield"))
          entry.Yield = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("epsTrailingTwelveMonths"))
          entry.EPScy = QString("%1").arg (value.at (counter).toDouble (), 0, 'f', 5).simplified ();
        else if (node.at (counter) == QLatin1String ("epsForward"))
          entry.EPSny = QString("%1").arg (value.at (counter).toDouble (), 0, 'f', 5).simplified ();
        else if (node.at (counter) == QLatin1String ("es"))
          entry.ESh = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("ps"))
          entry.PS = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
        else if (node.at (counter) == QLatin1String ("priceToBook"))
          entry.PBv = QString("%1").arg (value.at (counter).toFloat (), 0, 'g', 2).simplified ();
      }
    }
    else
      result = CG_ERR_INVALID_DATA;

  }
  setGlobalError(result, __FILE__, __LINE__);
  tempFile.get ()->close ();

  return  result;
}

// check if symbol exists
bool
YahooFeed::symbolExistence (QString & symbol, QString & name, QString & market,
                            QString & currency)
{
  return symbolExistencejson (symbol, name, market, currency);
}

// check if symbol exists from json
bool
YahooFeed::symbolExistencejson (QString & symbol, QString & name,
                                QString & market, QString & currency)
{
  if (netservice == nullptr)
    return false;

  CG_ERR_RESULT ioresult = CG_ERR_OK;
  bool result = false;

  Symbol.truncate (0);

  symbol = symbol.trimmed ();
  if (!validSymbol (symbol))
  {
    setGlobalError(ioresult, __FILE__, __LINE__);
    return result;
  }

  QString urlstr = symbolURL (symbol);
  if (urlstr.size () == 0)
  {
    setGlobalError(ioresult, __FILE__, __LINE__);
    return result;
  }

  Symbol = symbol;
  symbolName = name;

  // temporary file
  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
  {
    setGlobalError(CG_ERR_NOMEM, __FILE__, __LINE__);
    return result;
  }

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

  unique_ptr <QTextStream> in (new QTextStream (tempFile.get ()));
  in.get ()->seek (0);

  QString line = in.get ()->readAll ();
  if (line.size () != 0)
  {
    QStringList node, value;

    if (json_parse (line, &node, &value, nullptr))
    {
      symbolName = Market = Currency = CGLiteral ("");
      for (qint32 counter = 0; counter < node.size (); counter ++)
      {
        if (node.at (counter) == QLatin1String ("shortName"))
          symbolName = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("longName") &&
            symbolName.isEmpty ())
          symbolName = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("exchange"))
          Market = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("currency"))
          Currency = value.at (counter).simplified();

        if (node.at (counter) == QLatin1String ("financialCurrency") &&
            Currency.isEmpty ())
          Currency = value.at (counter).simplified();
      }

      name = symbolName.remove ("amp;");
      market = Market;
      currency = Currency;

      result = true;
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

// get Yahoo's crumb cookie hash
QString
YahooFeed::getCrumb (const QString & namein)
{
  unique_ptr<QFile> tFile (new QFile (namein));
  if (!tFile.get ()->open (QIODevice::ReadOnly|QIODevice::Text))
    return CGLiteral ("");

  unique_ptr <QTextStream> in (new QTextStream (tFile.get ()));
  QString ypage;
  ypage.reserve (1024*1024);
  ypage = in.get ()->read (1024*1024);
  int pos =
    ypage.indexOf (CGLiteral ("\"CrumbStore\":{\"crumb\":\""), 0, Qt::CaseInsensitive) +
    QString ("\"CrumbStore\":{\"crumb\":\"").size ();

  tFile.get ()->close ();

  return ypage.mid (pos, 11);;
}

// download historical data
CG_ERR_RESULT
YahooFeed::downloadData (QString symbol, QString timeframe, QString currency,
                         QString task, bool adjust)
{
  if (netservice == nullptr)
    return CG_ERR_NETWORK;

  tableName.truncate (0);

  CG_ERR_RESULT result = CG_ERR_OK;

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

  // temporary file
  unique_ptr<QTemporaryFile> tempFile (new (std::nothrow) QTemporaryFile);
  if (tempFile.get () == nullptr)
  {
    result = CG_ERR_NOMEM;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }

  // open temporary file
  if (!tempFile.get ()->open ())
  {
    result = CG_ERR_CREATE_TEMPFILE;
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }
  tempFile.get ()->resize (0);

  // get cookie and crumb
  // if (cookie.isEmpty ())
  // {
  Cookies cookies;
  result = netservice->httpGET (CGLiteral ("https://finance.yahoo.com/quote/") %
                                symbol % CGLiteral ("/history"),
                                *tempFile.get (), &cookies);
  if (result != CG_ERR_OK)
    return result;

  crumb = getCrumb (tempFile.get ()->fileName ());
  tempFile.get ()->resize (0);

  foreach (const QNetworkCookie ncookie, cookies)
    if (QString (ncookie.name ()) == QLatin1String ("B"))
      cookie = CGLiteral ("B=") % QString (ncookie.value ());
  // }
  // fill symbol entry
  entry.symbol = Symbol;
  entry.timeframe = timeframe;
  entry.csvfile = tempFile.get ()->fileName ();
  entry.source = CGLiteral("YAHOO");
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
  QString url = downloadURL (symbol, timeframe, crumb);
  entry.dnlstring = url;
  tableName = entry.tablename;

  netservice->setCookie (cookie);
  result = netservice->httpGET (url, *tempFile.get (), nullptr);
  if (result != CG_ERR_OK)
  {
    setGlobalError(result, __FILE__, __LINE__);
    return result;
  }

  if (GlobalProgressBar != nullptr)
    GlobalProgressBar->setValue (33);

  qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);

  if (tempFile.get ()->size () != 0)
  {
    result = downloadStats (Symbol, YahooFeed::HTTP /* YahooFeed::YQL */);
    if (result != CG_ERR_OK)
    {
      setGlobalError(result, __FILE__, __LINE__);
      return result;
    }

    if (GlobalProgressBar != nullptr)
      GlobalProgressBar->setValue (50);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents, 10);

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

  setGlobalError(result, __FILE__, __LINE__);
  tempFile.get ()->close ();
  return result;
}
