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
  class TwelveDataFeed;
}

typedef struct
{
  QString symbol;
  QString name;
  QString country;
  QString market;
  QString currency;
  QString type;
} TwelveDataSymbol;

typedef QVector < TwelveDataSymbol > TwelveDataSymbolVector;  	

class TwelveDataFeed: public QObject
{
  Q_OBJECT
  
public:
  explicit TwelveDataFeed (QObject *parent = 0); // constructor
  ~TwelveDataFeed (void);	     // destructor

  // functions
  bool symbolExistence (QString & symbol,
                        QString & name,
                        QString & market,
                        QString & currency,
                        const QString & type); // check if symbol exists

  CG_ERR_RESULT downloadData (QString symbol,    // download
							  QString type,		 // historical
                              QString timeframe, // data
                              QString currency,  
                              QString task,
                              bool    adjust);
  CG_ERR_RESULT downloadStats (QString symbol); // download statistics
  CG_ERR_RESULT getRealTimePrice (QString symbol, QString type,
                                  RTPrice &rtprice); // get real time price
  QString getTableName () const Q_DECL_NOEXCEPT
  {
    QString tname = tableName;
	tablename_normal (tname);  
    return tname;
  };   // get the table name of the last operation

  bool validSymbol (const QString &symbol); // validate Twelve Data symbol
  TwelveDataSymbolVector getSymbols (); // get symbol list 
  CG_ERR_RESULT populateSymlist (); // populate the symlist table

private:
  // variables and classes
  NetService *netservice; // netservice 
  RTPrice realtimeprice;// real time price
  QString Symbol;		// symbol
  QString symbolName; 	// symbolname
  QString Market;		// market
  QString Type;			// instrument type
  QString Currency;		// currency
  QString tableName;	// table name of the last operation
  SymbolEntry entry;	// symbol entry
  TwelveDataSymbolVector TwelveDataSymbols; // symbol list
  bool populated; // true if symlist table is populated

  // functions
  QString symbolURL (const QString &symbol, const QString &type); // returns symbol check URL
  QString symbolStatsURL (const QString &symbol); // returns symbol statistics URL
  QString downloadURL (const QString &symbol, const QString &market); // download URL
  QString updateURL (const QString &symbol, const QString &market); // update URL
  QString realTimePriceURL (const QString &symbol, const QString &type); // real time price URL
};
