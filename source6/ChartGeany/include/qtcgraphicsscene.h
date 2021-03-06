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

#include <QGraphicsScene>
#include <QMutex>

// QTCGraphicsScene
class QTCGraphicsScene : public QGraphicsScene
{
  Q_OBJECT
  	
public:
  explicit QTCGraphicsScene (QObject * parent = 0) : QGraphicsScene (parent) { };

  void
  qtcAddItem (QGraphicsItem *item)
  {
    protectScene.lock ();
    addItem (item);
    protectScene.unlock ();
  }

  QGraphicsProxyWidget *
  qtcAddWidget (QWidget * widget, const Qt::WindowFlags wFlags = 0)
  {
    protectScene.lock ();
    QGraphicsProxyWidget *wid = addWidget (widget, wFlags);
    protectScene.unlock ();
    return wid;
  }

  QGraphicsLineItem *
  qtcAddLine (const qreal x1, const qreal y1, 
              const qreal x2, const qreal y2, const QPen & pen = QPen())
  {
    protectScene.lock ();
    QGraphicsLineItem *line = addLine (x1, y1, x2, y2, pen);
    protectScene.unlock ();
    return line;
  }

private:
  QMutex protectScene;

};
