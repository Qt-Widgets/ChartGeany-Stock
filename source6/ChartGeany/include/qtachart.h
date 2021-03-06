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

#include <cstddef>

#include <QWidget>
#include <QList>
#include "defs.h"

#if !defined (Q_OS_WIN) && !defined (Q_CC_SUN)
using std::max_align_t;
#endif

enum
{
  QTACHART_TFDAY = 0,       // timeframe: day
  QTACHART_TFWEEK,          // timeframe: week
  QTACHART_TFMONTH,         // timeframe: month
  QTACHART_LINEAR = 100,    // chart: linear price scale
  QTACHART_LOGARITHMIC,     // chart: logarithmic price scale
  QTACHART_CANDLE = 200,    // chart: candle chart
  QTACHART_HEIKINASHI,      // chart: heikin-ashi candle chart
  QTACHART_BAR,             // chart: bar chart
  QTACHART_LINE             // chart: line chart
};

// frame data as loaded from sqlite table
typedef struct
{
  QString Text;
  qreal High;
  qreal Low;
  qreal Open;
  qreal Close;
  qreal AdjClose;
  qreal Volume;
  quint16 year;
  quint16 month;
  quint16 day;
  char Date[16];
  char Time[16];
} QTAChartFrame;
Q_DECLARE_TYPEINFO (QTAChartFrame, Q_MOVABLE_TYPE);
typedef QVector < QTAChartFrame > FrameVector;

typedef struct  // fundamenta data as loaded from sqlite table
{
  QString bv;
  QString mc;
  QString ebitda;
  QString pe;
  QString peg;
  QString dy;
  QString epscurrent;
  QString epsnext;
  QString esh;
  QString ps;
  QString pbv;
} QTAChartData;
Q_DECLARE_TYPEINFO (QTAChartData, Q_MOVABLE_TYPE);

namespace Ui
{
  class QTAChart;
}

class QTAChart:public QWidget
{
Q_OBJECT public:
  explicit QTAChart (QWidget * parent = 0);
  ~QTAChart (void);
  // valiables

  // functions
  QString getSymbolKey (); // get symbol's database key
  void goBack (void);   // backBtn_clicked (void) implementation
  inline int  getClassError () const Q_DECL_NOEXCEPT
  {
    return classError;
  }; // get class error
  inline QString getTabText () const Q_DECL_NOEXCEPT
  {
    return tabText;
  }; // get parent tab's label
  void loadFrames (QString tablename);  // populate chart's frame vector
  void loadData (QTAChartData data);    // load chart's data
  void restoreBottomText (void); // restores bottom text
  void setAlwaysRedraw (bool);  // always redraw the chart on/off
  void setTitle (QString title, QString subtitle);  // set the chart's title
  void setTabText (QString label)
  {
    tabText = label;
  }; // set parent tab's label
  void setCustomBottomText (QString string); // set the bottom text to custom string
  void setSymbol (QString symbol); // set chart's symbol
  void setSymbolKey (QString symbol); // set chart's symbol database key
  void setFeed (QString feed); // set symbol's data feed
  void setLinear (bool); // set linear scale
  void showVolumes (bool);  // turn volumes show on/off
  void showOnlinePrice (bool);  // turn online price show on/off
  void showGrid (bool);     // turn grid on off

private:
  Ui::QTAChart * ui;

  // variables
  QString tabText;              // parent tab's label
  const void *chartdata;        // chart's data area
  int classError;               // class error

  // functions
private slots:
  void backBtn_clicked (void);
  void propertiesBtn_clicked (void);
  void helpBtn_clicked (void);
  void dataBtn_clicked (void);
  void zoomInBtn_clicked (void);
  void zoomOutBtn_clicked (void);
  void expandBtn_clicked (void);
  void drawBtn_clicked (void);
  void functionBtn_clicked (void);
  void objectsBtn_clicked (void);

  friend void *getData (QTAChart *) Q_DECL_NOEXCEPT;   // internal use
protected:
  // functions
  virtual void resizeEvent (QResizeEvent * event);
  virtual void keyPressEvent (QKeyEvent * event);
  virtual void showEvent (QShowEvent * event);
};
