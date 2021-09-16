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

#include <QGraphicsTextItem>

#include "defs.h"
#include "priceupdater.h"
#include "stockticker.h"
#include "mainwindow.h"

/// PriceWorkerTicker
// constructor
PriceWorkerTicker::PriceWorkerTicker ()
{
  parentObject = nullptr;
  state = 0;
  runflag = 1;
}

// destructor
PriceWorkerTicker::~PriceWorkerTicker ()
{
  runflag = 0;
}

// process slot
void
PriceWorkerTicker::process()
{
  const int sleepms = 50;
  RTPriceList lrtprice;
  qint32 counter = 0;

  state = 1;
  while (runflag.fetchAndAddAcquire (0) == 1)
  {
    if (counter == 0)
    {
      QStringList lsymbol, lfeed;
      qint32 max;

      if (loadTickerSymbols (lsymbol, lfeed) == CG_ERR_OK)
      {
        symbol = lsymbol;
        datafeed = lfeed;
      }

      max = symbol.size ();

      lrtprice.clear ();
      lrtprice.reserve (max);

      for (qint32 counter2 = 0; counter2 < max && runflag.fetchAndAddAcquire (0); counter2 ++)
      {
        RTPrice dummy;
        if (runflag.fetchAndAddAcquire (0) == 1)
        {
          lrtprice += dummy;
          if (runflag.fetchAndAddAcquire (0) == 1)
          {
            if (datafeed.at (counter2).toUpper () == QLatin1String ("YAHOO"))
              Yahoo->getRealTimePrice (symbol.at (counter2), lrtprice[counter2], YahooFeed::HTTP);
            else if (datafeed.at (counter2).toUpper () == QLatin1String ("IEX"))
              IEX->getRealTimePrice (symbol.at (counter2), lrtprice[counter2]);
            else if (datafeed.at (counter2).toUpper () == QLatin1String ("TWELVEDATA"))
              TwelveData->getRealTimePrice (symbol.at (counter2), CGLiteral ("stocks"), lrtprice[counter2]);
            else if (datafeed.at (counter2).toUpper () == QLatin1String ("ALPHAVANTAGE"))
              AlphaVantage->getRealTimePrice (symbol.at (counter2), lrtprice[counter2], AlphaVantageFeed::CSV);
          }
        }
      }

      if (runflag.fetchAndAddAcquire (0) == 1)
      {
        for (qint32 counter3 = 0; counter3 < lrtprice.size (); counter3 ++)
        {
          if (lrtprice[counter3].price.toFloat () == 0)
            lrtprice.removeAt(counter3);
        }
        rtprice = lrtprice;
        if (parentObject != nullptr) parentObject->emitUpdateTicker (rtprice);
      }
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
PriceWorkerTicker::terminate ()
{
  runflag = 0;
}

/// PriceUpdater
// constructor
PriceUpdater::PriceUpdater (QString symbol, QString feed, QTACObject *parent)
{
  if (parent != nullptr)
    setParent (parent);

  tickerworker = nullptr;
  worker = new PriceWorker (symbol, feed);
  if (parent != nullptr)
  {
    worker->setParentObject (parent);
    parent->onlineprice = true;
  }
  worker->moveToThread(&thread);
  connect(&thread, SIGNAL(started()), worker, SLOT(process()));
  thread.start();
  thread.setPriority (QThread::LowestPriority);
}

PriceUpdater::PriceUpdater (StockTicker *parent)
{
  if (parent != nullptr)
  {
    setParent (parent);
    setObjectName (CGLiteral ("PriceUpdater"));
  }

  worker = nullptr;
  
  tickerworker = new PriceWorkerTicker ();
  if (parent != nullptr)
    tickerworker->setParentObject (parent);

  tickerworker->moveToThread(&thread);
  connect(&thread, SIGNAL(started()), tickerworker, SLOT(process()));
  thread.start();
  thread.setPriority (QThread::LowestPriority);
}

// destructor
PriceUpdater::~PriceUpdater ()
{
  if (worker != nullptr)
  {
    worker->terminate ();
    thread.quit ();
    thread.wait ();
    delete worker;
  }

  if (tickerworker != nullptr)
  {
    tickerworker->terminate ();
    thread.quit ();
    thread.wait ();
    delete tickerworker;
  }
}
