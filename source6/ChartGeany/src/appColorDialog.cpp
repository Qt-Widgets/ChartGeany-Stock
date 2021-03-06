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

#include "optsize.h"
#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QShowEvent>
#include "appColorDialog.h"
#include "common.h"
#include "cgliteral.h"

// constructor
appColorDialog::appColorDialog (QWidget * parent)
{
  if (parent != nullptr)
    setParent (parent);

  setOption (QColorDialog::DontUseNativeDialog, true);
  setWindowIcon (QIcon (CGLiteral (":/png/images/icons/PNG/cglogo.png")));
  connect (this, SIGNAL (accepted ()), this, SLOT (color_accepted ()));
  connect (this, SIGNAL (rejected ()), this, SLOT (color_rejected ()));
  connect (this, SIGNAL (finished (int)), this, SLOT (dialog_finished (int)));
  setStyleSheet (CGLiteral ("background: transparent; background-color:white;"));
  correctWidgetFonts (this);
}

// destructor
appColorDialog::~appColorDialog ()
{

}

// selected color
QColor
appColorDialog::appSelectedColor (bool *ok) const
{
  *ok = okflag;
  if (okflag)
    appSetOverrideCursor (this, QCursor (Qt::PointingHandCursor));
  return selectedColor ();
}

// color accepted
void
appColorDialog::color_accepted(void) Q_DECL_NOEXCEPT
{
  okflag = true;
}

// color rejected
void
appColorDialog::color_rejected (void) Q_DECL_NOEXCEPT
{
  okflag = false;

}

// dialog finished
void
appColorDialog::dialog_finished (int result)
{
  if (result == 0)
    setCurrentColor (keepCurrentColor);
}

// show event
void
appColorDialog::showEvent (QShowEvent * event)
{
  if (event->spontaneous ())
    return;

  keepCurrentColor = currentColor ();
  okflag = false;

  this->setGeometry( QStyle::alignedRect(
                       Qt::LeftToRight,
                       Qt::AlignCenter,
                       this->size(),
                       availableGeometry (-1)));
}
