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

#include "priceworker.h"

// process slot
void
PriceWorker::process()
{
  const int sleepms = 50;
  CG_ERR_RESULT result = CG_ERR_OK;
  qint32 counter = 0;

  state = 1;
  while (runflag.fetchAndAddAcquire (0) == 1)
  {
    if (counter == 0 && runflag.fetchAndAddAcquire (0))
    {
      if (datafeed.toUpper () == QLatin1String ("YAHOO") && runflag.fetchAndAddAcquire (0))
        result = Yahoo->getRealTimePrice (symbol, rtprice, YahooFeed::HTTP);
      else if (datafeed.toUpper () == QLatin1String ("IEX") && runflag.fetchAndAddAcquire (0))
        result = IEX->getRealTimePrice (symbol, rtprice);
      else if (datafeed.toUpper () == QLatin1String ("TWELVEDATA") && runflag.fetchAndAddAcquire (0))
        result = TwelveData->getRealTimePrice (symbol, "stocks", rtprice);
      else if (datafeed.toUpper () == QLatin1String ("ALPHAVANTAGE") &&
               runflag.fetchAndAddAcquire (0))
        result = AlphaVantage->getRealTimePrice (symbol, rtprice, AlphaVantageFeed::CSV);

      if (result == CG_ERR_OK && runflag.fetchAndAddAcquire (0) && parentObject != nullptr)
        if (parentObject != nullptr) parentObject->emitUpdateOnlinePrice (rtprice);
    }

    if (runflag.fetchAndAddAcquire (0) == 1)
    {
      Sleeper::msleep(sleepms);
      counter += sleepms;
      if (counter >= (Application_Settings->options.nettimeout * 1100))
        counter = 0;
    }
  }
  state = 0;
}

// terminate slot
void
PriceWorker::terminate ()
{
  runflag = 0;
}
