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

#include <QDialog>
#include <QPixmap>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QIcon>

#include "appColorDialog.h"
#include "debug.h"

// paramenter types
enum class PARAM_TYPE
{
  DPT_UNKNOWN = 0,		// unknown type
  DPT_INT,      		// integer input mask
  DPT_REAL,     		// real input mask
  DPT_COLOR     		// color dialog
};

// dynamic parameter
class DynParam
{
public:
  explicit DynParam (QString name)
  {
    label = paramName = name;
    callback_var = nullptr;
    show = false;
  }

  QString paramName;    // parameter's name
  QString label;        // parameter's label
  PARAM_TYPE type;         // parameter's type
  qreal defvalue;       // parameter's default value
  qreal value;          // parameter's current value
  bool show;            // parameter's show flag
  void *callback_var;   // parameter's callback variable (default nullptr if none)
};

class DPColorButton : public QPushButton
{
  Q_OBJECT

public:
  explicit DPColorButton (QWidget *parent, int paramidx, int buttonidx);
  ~DPColorButton (void);

  int paramidx;
  int buttonidx;
};

typedef QVector <DynParam *> ParamVector;
typedef QVector <QPixmap *> PixmapVector;
typedef QVector <QIcon *> IconVector;
typedef QVector <DPColorButton *> ButtonVector;

namespace Ui
{
  class DynParamsDialog;
}

class DynParamsDialog : public QDialog
{
  Q_OBJECT

public:
  explicit DynParamsDialog (QString title, QWidget *parent = 0); // new indicator constructor
  DynParamsDialog (ParamVector PVector, QString title); // modify constructor
  void DynParamsDialog_constructor_body (void); // constructor body
  ~DynParamsDialog(void);

  QDialogButtonBox *buttonBox;
  QCheckBox *removeCheckBox;
  appColorDialog *colorDialog; // color dialog

  // add a parameter
  void addParam (QString paramName, QString title, PARAM_TYPE type, qreal defvalue);

  // add a parameter with callback variable
  void addParam (QString paramName, QString label, PARAM_TYPE type, qreal defvalue,
                 void *cbvar, bool show);

  // get the title label
  QString getTitle (void) const;

  // get parameter data
  qreal getParam (QString paramName) const;

  // get parameter vector
  ParamVector getPVector (void) const;

  // sets the reference chart
  void setReferenceChart (void *chart);

  // set color dialog
  void setColorDialog (appColorDialog *);

private:
  Ui::DynParamsDialog * ui;

  ParamVector Param;    // parameters' vector
  PixmapVector Pixmap;  // pixmaps' vector
  IconVector Icon;      // icons' vector
  ButtonVector Button;  // buttons' vector
  int param_height;     // height of each parameter
  int Height;           // dialog's height
  int ncolorbuttons;    // number of color buttons
  int paramidx;         // parameter index
  int cbidx;            // color button index
  bool modify;          // create = false, modify = true
  void *referencechart; // reference chart

private slots:
  void color_clicked (void);
  void colorDialog_accepted (void);
  void colorDialog_rejected (void);
  void text_changed(QString);
  void function_accepted (void);
  void function_rejected (void);

protected:
  virtual void showEvent (QShowEvent * event);
  virtual void resizeEvent (QResizeEvent * event);
};
