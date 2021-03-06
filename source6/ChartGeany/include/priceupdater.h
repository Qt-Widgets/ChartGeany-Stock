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

#pragma once

#include "priceworker.h"

class StockTicker;

// Price worker for a symbol list
class PriceWorkerTicker : public QObject
{
  Q_OBJECT

public:
  PriceWorkerTicker (void); //constructor
  ~PriceWorkerTicker (void);  // destructor
  bool isRunning ()
  {
    return (bool) state.fetchAndAddAcquire (0);
  }; // returns running state
  void setParentObject (StockTicker *obj)
  {
    parentObject = obj;
  }; // set the parent object

public slots:
  void process(void);       // thread process
  void terminate (void);    // thread terminate

signals:

private:
  StockTicker *parentObject;    // parent object
  QAtomicInt runflag;     // set false to terminate execution
  QAtomicInt state;   // true if running
  QStringList datafeed;   // data feed
  QStringList symbol;     // symbol
  RTPriceList rtprice; // real time price
};

class PriceUpdater: public QObject
{
  Q_OBJECT

public:
  explicit PriceUpdater (StockTicker *parent); // constructor
  PriceUpdater (QString symbol, QString feed, QTACObject *parent); // constructor
  ~PriceUpdater (void);      // destructor

private:
  QThread thread;   // worker thread
  PriceWorker *worker; // worker class
  PriceWorkerTicker *tickerworker; // worker class

};
