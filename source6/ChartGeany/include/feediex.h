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

#include <QVector>
#include <QObject>

#include "netservice.h"

void tablename_normal (QString & tname);

namespace Ui
{
  class IEXFeed;
}

typedef struct
{
  QString symbol;
  QString name;
} IEXSymbol;

typedef QVector < IEXSymbol > IEXSymbolVector;  	

class IEXFeed: public QObject
{
  Q_OBJECT
  
public:
  explicit IEXFeed (QObject *parent = 0); // constructor
  ~IEXFeed (void);	     // destructor

  // functions
  bool symbolExistence (QString & symbol,
                        QString & name,
                        QString & market); // check if symbol exists

  CG_ERR_RESULT downloadData (QString symbol,    // download
                              QString timeframe, // historical
                              QString currency,  // data
                              QString task,
                              bool    adjust);
  CG_ERR_RESULT downloadStats (QString symbol); // download statistics
  CG_ERR_RESULT getRealTimePrice (QString symbol,
                                  RTPrice & rtprice); // get real time price
  QString getTableName () const Q_DECL_NOEXCEPT
  {
    QString tname = tableName;
	tablename_normal (tname);  
    return tname;
  };   // get the table name of the last operation

  bool validSymbol (QString symbol); // validate IEX symbol
  IEXSymbolVector getSymbols (); // get symbol list 
  CG_ERR_RESULT populateSymlist (); // populate the symlist table

private:
  // variables and classes
  NetService *netservice; // netservice 
  RTPrice realtimeprice;// real time price
  QString Symbol;		// symbol
  QString symbolName; 	// symbolname
  QString Market;		// market
  QString Currency;		// currency
  QString BookValue;	// book value
  QString MarketCap;	// market capitalization
  QString EBITDA;		// EBITDA
  QString PE;			// Price/Earnings
  QString PEG;			// Price/Earnings
  QString Yield;		// Yield
  QString EPScy;		// EPS current year
  QString EPSny;		// EPS next year
  QString ESh;			// Earnings/Share
  QString PS;			// Price/Sales
  QString PBv;			// Price/Book Value
  QString tableName;	// table name of the last operation
  SymbolEntry entry;	// symbol entry
  IEXSymbolVector IEXSymbols; // symbol list
  bool populated; // true if symlist table is populated

  // functions
  QString symbolURL (QString symbol); // returns symbol check URL
  QString symbolStatsURL (QString symbol); // returns symbol statistics URL
  QString downloadURL (QString symbol); // download URL
  QString updateURL (QString symbol); // update URL
  QString realTimePriceURL (QString symbol); // real time price URL
};
