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

#include <QObject>

#include "netservice.h"

void tablename_normal (QString & tname);

namespace Ui
{
  class YahooFeed;
}

class YahooFeed: public QObject
{
  Q_OBJECT
  
public:
  explicit YahooFeed (QObject *parent = 0); // constructor
  ~YahooFeed (void);	     // destructor

  enum YAHOO_API { HTTP, YQL, JSON };

  // functions
  bool symbolExistence (QString & symbol,
                        QString & name,
                        QString & market,
                        QString & currency); // check if symbol exists
 
  bool symbolExistencejson (QString & symbol,
                            QString & name,
                            QString & market,
                            QString & currency); // check if symbol exists from json                       

  CG_ERR_RESULT downloadData (QString symbol,    // download
                              QString timeframe, // historical
                              QString currency,  // data
                              QString task,
                              bool    adjust);
  CG_ERR_RESULT downloadStats (QString symbol, YAHOO_API api); // download statistics
  CG_ERR_RESULT downloadStatsjson (QString symbol); // download statistics
  CG_ERR_RESULT getRealTimePrice (QString symbol,
                                  RTPrice & rtprice, YAHOO_API api); // get real time price
  CG_ERR_RESULT getRealTimePricejson (QString symbol, RTPrice & rtprice); // get real time price                                  
  QString getTableName () const Q_DECL_NOEXCEPT
  {
	QString tname = tableName;
	tablename_normal (tname);  
    return tname;
  };   // get the table name of the last operation
  QString getSymbolName () const Q_DECL_NOEXCEPT
  {
    return symbolName;
  }; // get the name of the last symbol retrieved

  bool validSymbol (QString &symbol);	// validate yahoo symbol

private:
  NetService *netservice; // netservice 
  QString Symbol;		// symbol
  QString symbolName; 	// symbolname
  QString Market;		// market
  QString Currency;		// currency
  QString tableName;	// table name of the last operation
  SymbolEntry entry;	// symbol entry

  // functions
  QString symbolURL (QString &symbol); // returns symbol check URL
  QString symbolURLjson (QString &symbol); // returns symbol check URL
  QString symbolCurrencyURL (QString &symbol); // returns symbol's currency URL
  QString symbolStatsURL (QString &symbol); // returns symbol statistics URL
  QString downloadURL (QString &symbol, QString &timeframe, QString &crumb); // download URL
  QString updateURL (QString &symbol, QString &timeframe, QString &datefrom); // update URL
  QString realTimePriceURL (QString &symbol); // real time price URL
  QString getCrumb (const QString & namein);
};
