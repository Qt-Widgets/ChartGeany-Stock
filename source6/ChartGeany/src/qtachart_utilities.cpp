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

#include <QtCore/qmath.h>
#include "qtachart_core.h"

// max decimal points
qreal
maxfractionals (const FrameVector *HLOC) Q_DECL_NOEXCEPT
{
  qreal maxd = 0;
  
  for (const QTAChartFrame &Frame : *HLOC) 
  {
    maxd = qMax ((qreal) fracdig (Frame.Low), (qreal) fracdig (Frame.High));
    maxd = qMax (maxd, (qreal) fracdig (Frame.Open));
    maxd = qMax (maxd, (qreal) fracdig (Frame.Close));
  }
  
  qreal result = (qreal) ((qreal) 1 / qPow ((qreal) 10, (qreal) maxd));
  if (result < 0.01f)
    result = 0.01f;
  
  return result;  
}

// get chart's data
void *
getData (QTAChart * chart) Q_DECL_NOEXCEPT
{
  if (chart == NULL)
    return NULL;

  return static_cast <void *> ((void *) chart->chartdata);
}
