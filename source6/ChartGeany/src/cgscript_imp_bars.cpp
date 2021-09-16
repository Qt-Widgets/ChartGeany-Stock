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

#include <cstring>
#include "cgscript.h"
#include "qtachart_core.h"

// Bar
extern "C" Q_DECL_EXPORT BarData_t
Bar_imp (const void *ptr, Candle_t ct, TimeFrame_t tf, int shift)
{
  Q_UNUSED (QTACastFromConstVoid)

  QString TF;
  const QTAChartCore *core = static_cast <const QTAChartCore *> (ptr);
  BarData_t rslt;

  std::memset (&rslt, 0, sizeof (rslt));
  switch (tf)
  {
  case TF_DAY:
    TF = CGLiteral ("D");
    break;
  case TF_WEEK:
    TF = CGLiteral ("W");
    break;
  case TF_MONTH:
    TF = CGLiteral ("M");
    break;
  case TF_YEAR:
    TF = CGLiteral ("Y");
    break;
  default:
    return rslt;
  }

  int counter = 0;
  while (core->TIMEFRAME.at (counter).TFSymbol != TF) counter ++;
  
  const FrameVector fvector = (ct == CTYPE_HEIKINASHI) ?
                              core->TIMEFRAME.at (counter).HEIKINASHI :
                              core->TIMEFRAME.at (counter).HLOC;
  
  if (fvector.size () == 0)
    return rslt;

  const int maxshift = fvector.size () - 1;
  if (shift > maxshift)
    shift = maxshift;

  const QTAChartFrame frame = fvector.at (shift);

  rslt.High = frame.High;
  rslt.Low = frame.Low;
  rslt.Open = frame.Open;
  rslt.Close = frame.Close;
  rslt.AdjClose = frame.AdjClose;
  rslt.Volume = frame.Volume;
  rslt.Year = frame.year;
  rslt.Month = frame.month;
  rslt.Day = frame.day;
  rslt.Date[15] = 0;
  memcpy (rslt.Date, frame.Date, 15);
  rslt.Time[15] = 0;
  memcpy (rslt.Time, frame.Time, 15);
  rslt.Id[255] = 0;
  strncpy (rslt.Id, frame.Text.toStdString ().c_str (), 254);

  return rslt;
}

// NBars
extern "C" Q_DECL_EXPORT int
NBars_imp (const void *ptr, Candle_t ct, TimeFrame_t tf)
{
  QString TF;
  const QTAChartCore *core = static_cast <const QTAChartCore *> (ptr);

  switch (tf)
  {
  case TF_DAY:
    TF = CGLiteral ("D");
    break;
  case TF_WEEK:
    TF = CGLiteral ("W");
    break;
  case TF_MONTH:
    TF = CGLiteral ("M");
    break;
  case TF_YEAR:
    TF = CGLiteral ("Y");
    break;
  default:
    return 0;
  }

  int counter = 0;
  while (core->TIMEFRAME.at (counter).TFSymbol != TF) counter ++;

  if (ct == CTYPE_CANDLE)
    return core->TIMEFRAME.at (counter).HLOC.size ();

  return core->TIMEFRAME.at (counter).HEIKINASHI.size ();
}

// NVisibleBars
extern "C" Q_DECL_EXPORT int
NVisibleBars_imp (const void *ptr)
{
  const QTAChartCore *core = static_cast <const QTAChartCore *> (ptr);
  const FrameVector *hloc = (core->chart_style == QTACHART_CANDLE) ?
                            core->HLOC : core->HEIKINASHI;
  const int sb = static_cast <int> (*core->startbar);
  const int hls = static_cast <int> (hloc->size ());
  int rslt = (core->nbars_on_chart + sb);
  rslt = (rslt < hls ? rslt : hls) - sb;

  return rslt;
}

// NewestVisibleBar
extern "C" Q_DECL_EXPORT int
NewestVisibleBar_imp (const void *ptr) Q_DECL_NOEXCEPT
{
  const QTAChartCore *core = static_cast <const QTAChartCore *> (ptr);

  return static_cast <int> (*core->startbar);
}

// OldestVisibleBar
extern "C" Q_DECL_EXPORT int
OldestVisibleBar_imp (const void *ptr) Q_DECL_NOEXCEPT
{
  const QTAChartCore *core = static_cast <const QTAChartCore *> (ptr);

  return static_cast <int> (*core->startbar + NVisibleBars_imp (ptr) - 1);
}
