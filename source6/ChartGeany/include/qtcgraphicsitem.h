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

#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include "defs.h"
#include "qtcgraphicsscene.h"

enum
{
  LineItemType,
  RectItemType,
  EllipseItemType
};

class QTCGraphicsItem : public QGraphicsItem
{
public:
  explicit 
    QTCGraphicsItem (const int reqtype, QGraphicsItem *parent = 0) : type (reqtype)
    {
      if (parent != nullptr)
        setParentItem (parent);
    
      QTCGraphicsItem_constructor_common ();
    }

  ~QTCGraphicsItem ();

  // functions
  // line
  inline QLineF	line () const
  {
    if (Q_LIKELY (type == LineItem))
      return lineItem->line () ;

    return QLineF (0,0,0,0);
  }
  
  inline void setLine (const QLineF & line)
  {
    if (Q_LIKELY (type == LineItem))
      lineItem->setLine (line);
  }
  
  inline void setLine (qreal x1, qreal y1, qreal x2, qreal y2)
  {
    if (Q_LIKELY (type == LineItem))
      lineItem->setLine (x1, y1, x2, y2);
  }

  // rect
  QRectF	rect () const;
  void	    setRect (const QRectF & rectangle);
  void	    setRect (const qreal x, const qreal y, 
                     const qreal width, const qreal height);
  QBrush    brush () const;
  void      setBrush (const QBrush & brush);

  // general
  void		init (QTCGraphicsScene *);
  QRectF    boundingRect () const;
  void      paint (QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

  QPen	    pen () const;
  void      setPen (const QPen & pen);
  qreal     zValue () const;
  void      setZValue (const qreal z);
  qreal     opacity () const;
  void      setOpacity (const qreal opacity);
  QPointF   pos () const;
  void      setPos (const QPointF & pos);
  void      setPos (const qreal x, const qreal y);
  bool      isVisible () const;
  void      setVisible (const bool);
  void      setSize (const qreal x, const qreal y, 
                     const qreal width, const qreal height);
  QList <QGraphicsItem *>  children () const;

private:
  enum
  {
    LineItem,
    RectItem,
    EllipseItem
  };

  QGraphicsLineItem *lineItem;
  QGraphicsRectItem *rectItem;
  QTCGraphicsEllipseItem *ellipseItem;
  QTCGraphicsScene *Scene;
  const int type;
  bool initialized;

  void QTCGraphicsItem_constructor_common ();
};
