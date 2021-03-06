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

#include <QApplication>
#include <QTabWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include "qtachart_core.h"

static qreal xpad, ypad;

///
// QTAChartSceneEventFilter
QTAChartSceneEventFilter::QTAChartSceneEventFilter (QObject * parent)
{
  padx = -1;
  pady = -1;
  phase = 0;

  if (parent != nullptr)
    setParent (parent);

  core = qobject_cast <QTAChartCore *> (parent);
}

QTAChartSceneEventFilter::~QTAChartSceneEventFilter ()
{

}

// control drag and add an object on the chart
void
QTAChartSceneEventFilter::dragObjectCtrl (QObject *coreptr, QEvent *event)
{
  Q_UNUSED (coreptr);
  if (core->dragged_obj_type == QTACHART_OBJ_LABEL ||
      core->dragged_obj_type == QTACHART_OBJ_TEXT)
  {
    dragText (core, event);
    return;
  }

  if (core->dragged_obj_type == QTACHART_OBJ_HLINE ||
      core->dragged_obj_type == QTACHART_OBJ_VLINE ||
      core->dragged_obj_type == QTACHART_OBJ_LINE ||
      core->dragged_obj_type == QTACHART_OBJ_FIBO)
  {
    dragHVLine (core, event);
    return;
  }
}

// drag and add a line
void
QTAChartSceneEventFilter::dragHVLine (QObject *coreptr, QEvent *event)
{
  Q_UNUSED (coreptr);

  const int evtype = event->type ();

  // mouse buttons
  if (evtype == QEvent::GraphicsSceneMousePress ||
      evtype == QEvent::GraphicsSceneMouseRelease ||
      evtype == QEvent::GraphicsSceneMouseMove)
  {

    const QGraphicsSceneMouseEvent *qMouse = (QGraphicsSceneMouseEvent *) event;
    const QPointF point = qMouse->scenePos ();
    const qreal x = point.x ();
    const qreal y = point.y ();

    if (core->dragged_obj_type == QTACHART_OBJ_HLINE)
    {
      if (y < core->charttopmost || y > core->chartbottomost)
        return;
      core->hvline->setLine (core->chartleftmost /*+ xpad*/, y + ypad, core->chartrightmost /*+ xpad*/, y + ypad);
      core->setRullerCursor (y);
      core->setBottomText (x);
      if (evtype == QEvent::GraphicsSceneMouseRelease)
      {
        core->addHLine (core->hvline, core->priceOnY (y));
        core->object_drag = false;
        appRestoreOverrideCursor (core->chart);
      }
    }

    if (core->dragged_obj_type == QTACHART_OBJ_VLINE)
    {
      if (x < core->chartleftmost || x > core->chartrightmost)
        return;
      core->hvline->setLine (x + xpad, core->charttopmost /*+ ypad*/, x + xpad, core->chartbottomost /*+ ypad*/);
      core->setRullerCursor (y);
      core->setBottomText (x);
      if (event->type () == QEvent::GraphicsSceneMouseRelease)
      {
        core->addVLine (core->hvline);
        core->object_drag = false;
        appRestoreOverrideCursor (core->chart);
      }
    }

    if (core->dragged_obj_type == QTACHART_OBJ_LINE)
    {
      if (x < core->chartleftmost || x > core->chartrightmost)
        return;

      if (y < core->charttopmost || y > core->chartbottomost)
        return;

      if (core->hvline->line ().x1 () == core->hvline->line ().x2 () &&
          core->hvline->line ().y1 () == core->hvline->line ().y2 () &&
          core->hvline->line ().y2 () == 0)
        phase = 0;
      else
        phase = 1;

      core->setRullerCursor (y);
      core->setBottomText (x);

      if (phase == 1)
      {
        const qreal x1 = core->hvline->line().x1 ();
        const qreal y1 = core->hvline->line().y1 ();

        core->hvline->setLine (x1, y1, x + xpad, y + ypad);
      }

      if (evtype == QEvent::GraphicsSceneMousePress)
      {
        if (phase == 0)
        {
          core->hvline->setLine (x, y, x, y);
          phase = 1;
        }
      }

      if (evtype == QEvent::GraphicsSceneMouseRelease)
      {
        if (phase == 1)
        {
          core->addTLine (core->hvline);
          core->object_drag = false;
          appRestoreOverrideCursor (core->chart);
          phase = 0;
        }
      }
    }

    if (core->dragged_obj_type == QTACHART_OBJ_FIBO)
    {
      if (x < core->chartleftmost || x > core->chartrightmost)
        return;

      if (y < core->charttopmost || y > core->chartbottomost)
        return;

      if (core->hvline->line ().x1 () == core->hvline->line ().x2 () &&
          core->hvline->line ().y1 () == core->hvline->line ().y2 () &&
          core->hvline->line ().y2 () == 0)
        phase = 0;
      else
        phase = 1;

      core->setRullerCursor (y);
      core->setBottomText (x);

      if (phase == 1)
      {
        const qreal y1 = core->hvline->line().y1 ();
        core->hvline->setLine (x + xpad, y1,  x + xpad, y + ypad);
      }

      if (evtype == QEvent::GraphicsSceneMousePress)
      {
        if (phase == 0)
        {
          core->hvline->setLine (x, y, x, y);
          phase = 1;
        }
      }

      if (evtype == QEvent::GraphicsSceneMouseRelease)
      {
        if (phase == 1)
        {
          core->addFibo (core->hvline);
          core->object_drag = false;
          appRestoreOverrideCursor (core->chart);
          phase = 0;
        }
      }
    }
  }
}

// drag and add a Label/Text object
void
QTAChartSceneEventFilter::dragText (QObject *coreptr, QEvent *event)
{
  Q_UNUSED (coreptr)

  if (event->type () == QEvent::GraphicsSceneMousePress ||
      event->type () == QEvent::GraphicsSceneMouseRelease ||
      event->type () == QEvent::GraphicsSceneMouseMove)
  {
    const QGraphicsSceneMouseEvent *qMouse = static_cast <QGraphicsSceneMouseEvent *> (event);
    const QPointF point = qMouse->scenePos ();
    const QRectF rect = core->textitem->boundingRect ();
    qreal x = point.x ();
    qreal y = point.y ();

    if (core->textitem->x() <= 0)
      padx = rect.width () / 2;

    if (padx == -1)
      padx = x - core->textitem->x();

    x -= padx;

    if (x < core->chartleftmost)
    {
      x = core->chartleftmost;
    }
    else
    {
      if (x + rect.width () > core->chartrightmost)
        x = core->chartrightmost - rect.width ();
    }

    if (core->textitem->y() <= 0)
      pady = rect.height () / 2;

    if (pady == -1)
      pady = y - core->textitem->y();

    y -= pady;

    if (y < core->charttopmost)
      y = core->charttopmost;
    else if (y + rect.height () > core->chartbottomost)
      y = core->chartbottomost - rect.height ();

    core->textitem->setPos (x, y);
    core->setBottomText (x);
    if (event->type () == QEvent::GraphicsSceneMouseRelease)
    {
      if (core->dragged_obj_type == QTACHART_OBJ_LABEL)
        core->addLabel (core->textitem, x, y);
      else if (core->dragged_obj_type == QTACHART_OBJ_TEXT)
        core->addText (core->textitem, x, y);

      core->object_drag = false;
      appRestoreOverrideCursor (core->chart);
      padx = pady = -1;
    }
  }
}

bool
QTAChartSceneEventFilter::eventFilter (QObject * object, QEvent * event)
{
  QGraphicsSceneMouseEvent *qMouse;
  QPointF point;
  qreal x, y;

  if (Q_UNLIKELY (core->object_drag))
  {
    dragObjectCtrl (core, event);
    return false;
  }

  if (Q_UNLIKELY (!core->events_enabled))
    return false;

  event->accept ();
  const int evtype = event->type ();

  // mouse buttons
  if (Q_LIKELY (evtype == QEvent::GraphicsSceneMousePress ||
                evtype == QEvent::GraphicsSceneMouseRelease ||
                evtype == QEvent::GraphicsSceneMouseMove))
  {
    qMouse = static_cast <QGraphicsSceneMouseEvent *> (event);
    point = qMouse->scenePos ();

    x = point.x ();
    y = point.y ();

    core->setBottomText (x);
    core->setRullerCursor (y);

    if (Q_LIKELY (x >= core->chartleftmost &&
                  y >= core->charttopmost &&
                  x <= core->chartrightmost + core->right_border_width &&
                  y <= core->chartbottomost))
    {
      core->last_x = x;
    }
    else
    {
      appRestoreOverrideCursor (core->chart);
      core->drag = false;
      return false;
    }
  }
  else
  {
    // mouse wheel
    if (evtype == QEvent::GraphicsSceneWheel)
    {
      const QGraphicsSceneWheelEvent *qWheel =
        static_cast <QGraphicsSceneWheelEvent *> (event);

      if (qWheel->delta () > 0)
        core->chartForward (1); // right
      else
        core->chartBackward (1);    //left

      core->setRullerCursor (core->ruller_cursor_y);
      core->setBottomText (core->last_x);
    }
    return false;
  }

  // mouse button press
  switch (evtype)
  {
  // left button
  case QEvent::GraphicsSceneMousePress:
    if (qMouse->button () == Qt::LeftButton && core->drag == false )
    {
      if (y > core->title_height && x < core->width)
      {
        core->drag = true;
        core->initial_mouse_x = x;
        appSetOverrideCursor (core->chart, QCursor (Qt::ClosedHandCursor));
      }
    }
    break;

  // mouse button release
  case QEvent::GraphicsSceneMouseRelease:
    if (core->drag == true)
    {
      core->drag = false;
      appRestoreOverrideCursor (core->chart);
      return true;
    }
    break;

  // drag
  default:
    if (core->drag == true)
    {
      const qreal diff = core->initial_mouse_x - x;
      const qreal adiff = qAbs (diff);
      const int sense = 4;

      // pointer moved right
      if (adiff >= sense)
      {
        const int bars = (int) (adiff / core->framewidth);
        if (bars > 0)
        {
          if (diff < 0)
            core->chartBackward (bars);
          else      // pointer moved left
            core->chartForward (bars);
          core->initial_mouse_x = point.x ();
          return true;
        }
      }
    }
  }
  return QObject::eventFilter (object, event);
}

///
// QTAChartEventFilter
QTAChartEventFilter::QTAChartEventFilter (QObject * parent)
{
  if (parent != nullptr)
    setParent (parent);
}

QTAChartEventFilter::~QTAChartEventFilter ()
{

}

bool
QTAChartEventFilter::eventFilter (QObject * watched, QEvent * event)
{
  if (watched->inherits ("QGraphicsView"))
  {
    const QGraphicsView *View = qobject_cast <QGraphicsView *> (watched);
    QTAChart *chart = qobject_cast <QTAChart *> (View->parentWidget ());
    QTAChartCore *core = static_cast <QTAChartCore *> (getData (chart));

    if (core == nullptr)
      return false;

    if (!core->events_enabled)
      return false;

    const int evtype = event->type ();
    if (evtype == QEvent::Show)
      core->Scene.setFocus (Qt::OtherFocusReason);

    // keyboard
    if (Q_UNLIKELY (evtype == QEvent::KeyPress ||
                    evtype == QEvent::FocusIn))
    {
      const QKeyEvent *keyEvent = static_cast < QKeyEvent * > (event);
      const Qt::Key keyPressed = static_cast <Qt::Key> (keyEvent->key ());

      appRestoreOverrideCursor (core->chart);
      core->setRullerCursor (core->ruller_cursor_y);
      core->setBottomText (core->last_x);

      // tab
      if (keyPressed == Qt::Key_Tab)
      {
        QTabWidget *tabWidget = qobject_cast <QTabWidget *> (chart->parentWidget ()->parentWidget ());
        const int idx = tabWidget->currentIndex () + 1;

        if (idx == tabWidget->count ())
          return true;

        tabWidget->setCurrentIndex (idx);
        tabWidget->update ();
        return true;
      }
      // backtab
      else if (keyPressed == Qt::Key_Backtab)
      {
        QTabWidget *tabWidget = 
          qobject_cast <QTabWidget *> (chart->parentWidget ()->parentWidget ());
        int idx = tabWidget->currentIndex ();
        
        if (idx == 0)
          return true;

        idx --;
        tabWidget->setCurrentIndex (idx);
        tabWidget->update ();
        return true;
      }
      // left
      else if (keyPressed == Qt::Key_Left)
      {
        core->chartBackward (1);
        return true;
      }
      // right
      else if (keyPressed == Qt::Key_Right)
      {
        core->chartForward (1);
        return true;
      }
      // home
      else if (keyPressed == Qt::Key_Home)
      {
        core->chartBegin ();
        return true;
      }
      // end
      else if (keyPressed == Qt::Key_End)
      {
        core->chartEnd ();
        return true;
      }
      // pgup
      else if (keyPressed == Qt::Key_PageUp)
      {
        core->chartPagePrevious ();
        return true;
      }
      // pgdn
      else if (keyPressed == Qt::Key_PageDown)
      {
        core->chartPageNext ();
        return true;
      }
      // esc
      else if (keyPressed == Qt::Key_Escape)
      {
        if (core->object_drag == true)
        {
          core->object_drag = false;
          if (core->textitem != nullptr)
          {
            delete core->textitem;
            core->textitem = nullptr;
          }

          if (core->hvline != nullptr)
          {
            delete core->hvline;
            core->hvline = nullptr;
          }

          appRestoreOverrideCursor (core->chart);
        }
        return true;
      }
    }
  }

  return QObject::eventFilter (watched, event);
}

///
// QTACObjectEventFilter
QTACObjectEventFilter::QTACObjectEventFilter (QObject * parent)
{
  if (parent == nullptr)
    return;

  setParent (parent);
  core = static_cast <QTAChartCore *>
         (const_cast <void *> ((((qobject_cast <QTACObject *> (parent->parent ()))->chartdata))));
}

QTACObjectEventFilter::~QTACObjectEventFilter ()
{

}

bool
QTACObjectEventFilter::eventFilter (QObject * watched, QEvent * event)
{
  QTACObject *object =
     qobject_cast <QTACObject *> (watched->parent ());
     
  if (object->deleteit)
    return QObject::eventFilter (object, event);

  if (core->object_drag)
    return QObject::eventFilter (object, event);

  const int evtype = event->type ();

  if (evtype == QEvent::GraphicsSceneHoverEnter)
  {
    if (object->type == QTACHART_OBJ_HLINE ||
        object->type == QTACHART_OBJ_VLINE)
    {
      object->title->setOpacity (0.7);
      object->hvline->setOpacity (0.7);
      core->dragged_obj_type = object->type;
      core->events_enabled = false;
    }
    else if (object->type == QTACHART_OBJ_LABEL ||
             object->type == QTACHART_OBJ_TEXT)
    {
      object->text->setOpacity (0.7);
      core->textitem = object->text;
      core->dragged_obj_type = object->type;
      core->events_enabled = false;
    }
    else if (object->type == QTACHART_OBJ_LINE)
    {
      if (watched == object->Edge[0]->pricetxt)
      {
        const LineEdge edge = *object->Edge[1];
        *object->Edge[1]= *object->Edge[0];
        *object->Edge[0] = edge;
        object->Edge[0]->sequence = 1;
        object->Edge[1]->sequence = 2;
        const qreal x1 = object->hvline->line ().x2 ();
        const qreal x2 = object->hvline->line ().x1 ();
        const qreal y1 = object->hvline->line ().y2 ();
        const qreal y2 = object->hvline->line ().y1 ();
        object->hvline->setLine (x1, y1, x2, y2);
        core->hvline = object->hvline;
      }

      object->title->setOpacity (0.7);
      object->hvline->setOpacity (0.7);
      object->Edge[0]->pricetxt->setOpacity (0.7);
      object->Edge[1]->pricetxt->setOpacity (0.7);
      core->dragged_obj_type = object->type;
      core->events_enabled = false;
    }
    else if (object->type == QTACHART_OBJ_FIBO)
    {
      if (watched == object->Edge[0]->pricetxt)
      {
        const LineEdge edge = *object->Edge[1];
        *object->Edge[1]= *object->Edge[0];
        *object->Edge[0] = edge;
        object->Edge[0]->sequence = 1;
        object->Edge[1]->sequence = 2;
        const qreal x1 = object->hvline->line ().x2 ();
        const qreal x2 = object->hvline->line ().x1 ();
        const qreal y1 = object->hvline->line ().y2 ();
        const qreal y2 = object->hvline->line ().y1 ();
        object->hvline->setLine (x1, y1, x2, y2);
        core->hvline = object->hvline;
      }

      object->title->setOpacity (0.7);
      object->hvline->setOpacity (0.7);
      object->Edge[0]->pricetxt->setOpacity (0.7);
      object->Edge[1]->pricetxt->setOpacity (0.7);
      core->dragged_obj_type = object->type;
      core->events_enabled = false;
      for (qint32 counter = 0; counter < object->FiboLevelPrc.size (); counter ++)
      {
        object->FiboLevelLbl[counter].setOpacity (0.7);
        object->FiboLevelPrcLbl[counter].setOpacity (0.7);
        object->FiboLevel[counter]->setOpacity (0.7);
      }
    }
    else if (object->type == QTACHART_OBJ_CURVE || object->type == QTACHART_OBJ_DOT)
    {
      object->title->setOpacity (0.7);
      core->object_drag = false;
      core->events_enabled = true;

      for (qint32 counter = 0; counter < object->ITEMSsize; counter ++)
        object->ITEMS[counter]->setOpacity (0.7);

      for (auto child : object->children)
      {
        for (qint32 counter = 0; counter < child->ITEMSsize; counter ++)
          child->ITEMS[counter]->setOpacity (0.7);
      }
    }
  }
  else if (evtype == QEvent::GraphicsSceneHoverLeave)
  {
    if (object->type == QTACHART_OBJ_HLINE ||
        object->type == QTACHART_OBJ_VLINE)
    {
      object->title->setOpacity (1.0);
      object->hvline->setOpacity (1.0);
      core->object_drag = false;
      core->events_enabled = true;
    }
    else if (object->type == QTACHART_OBJ_LABEL ||
             object->type == QTACHART_OBJ_TEXT)
    {
      object->text->setOpacity (1.0);
      core->object_drag = false;
      core->events_enabled = true;
    }
    else if (object->type == QTACHART_OBJ_LINE)
    {
      object->title->setOpacity (1.0);
      object->hvline->setOpacity (1.0);
      object->Edge[0]->pricetxt->setOpacity (1.0);
      object->Edge[1]->pricetxt->setOpacity (1.0);
      core->object_drag = false;
      core->events_enabled = true;
    }
    else if (object->type == QTACHART_OBJ_FIBO)
    {
      object->title->setOpacity (1.0);
      object->hvline->setOpacity (1.0);
      object->Edge[0]->pricetxt->setOpacity (1.0);
      object->Edge[1]->pricetxt->setOpacity (1.0);
      core->object_drag = false;
      core->events_enabled = true;

      for (qint32 counter = 0; counter < object->FiboLevelPrc.size (); counter ++)
      {
        object->FiboLevelLbl[counter].setOpacity (1.0);
        object->FiboLevelPrcLbl[counter].setOpacity (1.0);
        object->FiboLevel[counter]->setOpacity (1.0);
      }
    }

    if (object->type == QTACHART_OBJ_CURVE || object->type == QTACHART_OBJ_DOT)
    {
      object->title->setOpacity (1.0);
      core->object_drag = false;
      core->events_enabled = true;

      for (qint32 counter = 0; counter < object->ITEMSsize; counter ++)
        object->ITEMS[counter]->setOpacity (1.0);

      for (auto child : object->children)
      {
        for (qint32 counter = 0; counter < child->ITEMSsize; counter ++)
          child->ITEMS[counter]->setOpacity (1.0);
      }
    }
    appRestoreOverrideCursor (core->chart);
  }
  else if (evtype == QEvent::GraphicsSceneMouseDoubleClick)
  {
    appRestoreOverrideCursor (core->chart);
    if (object->type == QTACHART_OBJ_CURVE ||
        object->type == QTACHART_OBJ_DOT)
    {
      core->events_enabled = false;
      bool modrslt = object->modifyIndicator ();
      if (modrslt == false)
        object->setForDelete ();
      core->events_enabled = true;
    }
    else if (object->type == QTACHART_OBJ_HLINE ||
             object->type == QTACHART_OBJ_VLINE)
    {
      bool modrslt  = core->lineobjectdialog->modify (object);
      if (modrslt == false)
      {
        object->setForDelete ();
        core->events_enabled = true;
      }
    }
    else if (object->type == QTACHART_OBJ_LABEL ||
             object->type == QTACHART_OBJ_TEXT)
    {
      bool modrslt  = core->textobjectdialog->modify (object->text);
      if (modrslt == false)
      {
        object->setForDelete ();
        core->events_enabled = true;
      }
    }
    else if (object->type == QTACHART_OBJ_LINE ||
             object->type == QTACHART_OBJ_FIBO)
    {
      bool modrslt  = core->lineobjectdialog->modify (object);
      if (modrslt == false)
      {
        object->setForDelete ();
        core->events_enabled = true;
      }
    }

    if (object->deleteit == false)
      core->draw ();

    core->object_drag = false;
    core->drag = false;
  }
  else if (evtype == QEvent::GraphicsSceneMousePress)
  {
    const QGraphicsSceneMouseEvent *qMouse =
      static_cast <QGraphicsSceneMouseEvent *> (event);
      
    const QPointF point = qMouse->scenePos ();

    if (qMouse->button () == Qt::LeftButton)
    {
      if (object->type == QTACHART_OBJ_HLINE ||
          object->type == QTACHART_OBJ_VLINE ||
          object->type == QTACHART_OBJ_LINE ||
          object->type == QTACHART_OBJ_FIBO)
        if (object->title->opacity () < 1)
        {
          core->object_drag = true;
          core->drag = false;
          core->hvline = object->hvline;
          xpad = core->hvline->line().x2() - point.x ();
          ypad = core->hvline->line().y2() - point.y ();
          appSetOverrideCursor (core->chart, QCursor (Qt::PointingHandCursor));
        }

      if (object->type == QTACHART_OBJ_LABEL ||
          object->type == QTACHART_OBJ_TEXT)
        if (object->text->opacity () < 1)
        {
          core->object_drag = true;
          core->drag = false;
          appSetOverrideCursor (core->chart, QCursor (Qt::PointingHandCursor));
        }
    }
  }

  if (evtype != QEvent::GraphicsSceneMouseDoubleClick)
    return QObject::eventFilter (object, event);
  return true;
}
