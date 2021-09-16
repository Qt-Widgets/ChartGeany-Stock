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

// Full implementation of QTACFunctions
#include <algorithm>

#include "common.h"
#include "qtachart_core.h"
#include "ui_qtacfunctions.h"
#include "qtachart_functions.h"

// constructor
QTACFunctions::QTACFunctions (QWidget * parent):
  QWidget (parent), ui (new Ui::QTACFunctions)
{
  ui->setupUi (this);

  button_width = 200;
  button_height = 40;

  layout = new QGridLayout(this);
  this->setLayout(layout);

  if (parent != nullptr)
    setParent (parent);
}

static bool
ButtonCmp(const QPushButton *b1, const QPushButton *b2)
{
  return b1->text() < b2->text();
}

void
QTACFunctions::createButtons (void)
{
  QPushButton *btn;
  appColorDialog *colorDialog;
  DynParamsDialog *ParamDialog;

  btn = addButton (CGLiteral ("SMA"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 14.0);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("EMA"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 14.0);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("MACD"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 9.0);
  ParamDialog->addParam (CGLiteral ("MACD color"), CGLiteral ("MACD color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::yellow).rgb ());
  ParamDialog->addParam (CGLiteral ("Signal color"), CGLiteral ("Signal color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::red).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("MFI"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 14.0);
  ParamDialog->addParam (CGLiteral ("High level"), CGLiteral ("High level"), PARAM_TYPE::DPT_INT, 80);
  ParamDialog->addParam (CGLiteral ("Medium level"), CGLiteral ("Medium level"), PARAM_TYPE::DPT_INT, 50);
  ParamDialog->addParam (CGLiteral ("Low level"), CGLiteral ("Low level"), PARAM_TYPE::DPT_INT, 20);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("ROC"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 10.0);
  ParamDialog->addParam (CGLiteral ("Level"), CGLiteral ("Level"), PARAM_TYPE::DPT_INT, 0);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("RSI"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 14.0);
  ParamDialog->addParam (CGLiteral ("High level"), CGLiteral ("High level"), PARAM_TYPE::DPT_INT, 70);
  ParamDialog->addParam (CGLiteral ("Low level"), CGLiteral ("Low level"), PARAM_TYPE::DPT_INT, 30);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("Slow Stoch"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 5.0);
  ParamDialog->addParam (CGLiteral ("High level"), CGLiteral ("High level"), PARAM_TYPE::DPT_INT, 80);
  ParamDialog->addParam (CGLiteral ("Medium level"), CGLiteral ("Medium level"), PARAM_TYPE::DPT_INT, 50);
  ParamDialog->addParam (CGLiteral ("Low level"), CGLiteral ("Low level"), PARAM_TYPE::DPT_INT, 20);
  ParamDialog->addParam (CGLiteral ("%K color"), CGLiteral ("%K Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->addParam (CGLiteral ("%D color"), CGLiteral ("%D Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::yellow).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("Fast Stoch"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 5.0);
  ParamDialog->addParam (CGLiteral ("High level"), CGLiteral ("High level"), PARAM_TYPE::DPT_INT, 80);
  ParamDialog->addParam (CGLiteral ("Medium level"), CGLiteral ("Medium level"), PARAM_TYPE::DPT_INT, 50);
  ParamDialog->addParam (CGLiteral ("Low level"), CGLiteral ("Low level"), PARAM_TYPE::DPT_INT, 20);
  ParamDialog->addParam (CGLiteral ("%K color"), CGLiteral ("%K Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->addParam (CGLiteral ("%D color"), CGLiteral ("%D Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::yellow).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("W%R"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 14.0);
  ParamDialog->addParam (CGLiteral ("High level"), CGLiteral ("High level"), PARAM_TYPE::DPT_INT, -20);
  ParamDialog->addParam (CGLiteral ("Low level"), CGLiteral ("Low level"), PARAM_TYPE::DPT_INT, -80);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("Bollinger Bands"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 20.0);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::magenta).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("Parabolic SAR"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("ADX"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 13.0);
  ParamDialog->addParam (CGLiteral ("Weak"), CGLiteral ("Weak"), PARAM_TYPE::DPT_INT, 25);
  ParamDialog->addParam (CGLiteral ("Strong"), CGLiteral ("Strong"), PARAM_TYPE::DPT_INT, 50);
  ParamDialog->addParam (CGLiteral ("Very strong"), CGLiteral ("Very strong"), PARAM_TYPE::DPT_INT, 75);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("Aroon"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 25.0);
  ParamDialog->addParam (CGLiteral ("High level"), CGLiteral ("High level"), PARAM_TYPE::DPT_INT, 70);
  ParamDialog->addParam (CGLiteral ("Medium level"), CGLiteral ("Medium level"), PARAM_TYPE::DPT_INT, 50);
  ParamDialog->addParam (CGLiteral ("Low level"), CGLiteral ("Low level"), PARAM_TYPE::DPT_INT, 30);
  ParamDialog->addParam (CGLiteral ("Up color"), CGLiteral ("Up Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::green).rgb ());
  ParamDialog->addParam (CGLiteral ("Down color"), CGLiteral ("Down Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::red).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("CCI"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 20.0);
  ParamDialog->addParam (CGLiteral ("High level"), CGLiteral ("High level"), PARAM_TYPE::DPT_INT, 100);
  ParamDialog->addParam (CGLiteral ("Low level"), CGLiteral ("Low level"), PARAM_TYPE::DPT_INT, -100);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("STDDEV"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 10.0);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("Momentum"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 12.0);
  ParamDialog->addParam (CGLiteral ("Level"), CGLiteral ("Level"), PARAM_TYPE::DPT_INT, 0);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("DMI"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 13.0);
  ParamDialog->addParam (CGLiteral ("Weak"), CGLiteral ("Weak"), PARAM_TYPE::DPT_INT, 25);
  ParamDialog->addParam (CGLiteral ("Strong"), CGLiteral ("Strong"), PARAM_TYPE::DPT_INT, 50);
  ParamDialog->addParam (CGLiteral ("Very strong"), CGLiteral ("Very strong"), PARAM_TYPE::DPT_INT, 75);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  btn = addButton (CGLiteral ("ATR"));
  connect (btn, SIGNAL (clicked ()), this, SLOT (button_clicked ()));
  ParamDialog = new DynParamsDialog (CGLiteral (""), btn);
  colorDialog = new appColorDialog;
  ParamDialog->setColorDialog (colorDialog);
  ParamDialog->addParam (CGLiteral ("Period"), CGLiteral ("Period"), PARAM_TYPE::DPT_INT, 14.0);
  ParamDialog->addParam (CGLiteral ("Color"), CGLiteral ("Color"), PARAM_TYPE::DPT_COLOR, (qreal) QColor (Qt::cyan).rgb ());
  ParamDialog->setObjectName (CGLiteral ("ParamDialog"));
  connect(ParamDialog->buttonBox, SIGNAL(accepted ()), this, SLOT(function_accepted()));
  connect(ParamDialog->buttonBox, SIGNAL(rejected ()), this, SLOT(function_rejected()));

  // sort buttons
  // qSort(Button.begin(), Button.end(), ButtonCmp);
  std::sort(Button.begin(), Button.end(), ButtonCmp);

  // correct fonts
  correctWidgetFonts (this);
}

// destructor
QTACFunctions::~QTACFunctions ()
{

  delete ui;
}

// add a button
QPushButton *
QTACFunctions::addButton (QString text)
{
  QFont fnt;
  QPushButton *btn;
  QString stylesheet;

  stylesheet =
    CGLiteral ("background: transparent; border: 1px solid transparent;border-color: darkgray;");

  btn = new QPushButton (text, this);
  btn->setFixedSize (QSize (button_width, button_height));
  fnt = btn->font ();
  fnt.setPixelSize (16);
  fnt.setBold (true);
  btn->setFont (fnt);
  btn->setStyleSheet (stylesheet);
  btn->setAutoFillBackground (false);
  btn->setFocusPolicy (Qt::NoFocus);
  btn->setObjectName ("Indicator Button");
  Button += btn;
  return btn;
}

// set the reference chart
void
QTACFunctions::setReferenceChart (void *chart)
{
  referencechart = chart;
  parentchart = static_cast <QTAChart *> (chart);
  createButtons ();
}

// get the reference chart
void *
QTACFunctions::getReferenceChart (void)
{
  return referencechart;
}

// add indicator
void
QTACFunctions::addIndicator (DynParamsDialog *paramDialog)
{
  QTAChart *chart = static_cast <QTAChart *> (referencechart);
  QTAChartCore *core = static_cast <QTAChartCore *> (getData (chart));
  QTACObject *indicator = nullptr, *childobj;
  QString fname;
  qint32 period;
  QColor color;

  if (core->CLOSE.size () == 0)
    return;

  fname = paramDialog->getTitle ();

  if (fname == QLatin1String ("SMA"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_CURVE);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), SMA, 0, 0, color, CGLiteral ("Color"));
    indicator->setTitle (fname);
  }

  if (fname == QLatin1String ("EMA"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_CURVE);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), EMA, 0, 0, color, CGLiteral ("Color"));
    indicator->setTitle (fname);
  }

  if (fname == QLatin1String ("Parabolic SAR"))
  {
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_DOT);
    indicator->setAttributes (QTACHART_CLOSE, 1, CGLiteral (""), PSAR, 0, 0, color, CGLiteral ("Color"));
    indicator->setTitle (fname);
  }

  if (fname == QLatin1String ("RSI"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, 0, 100, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), RSI, 0, 100, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("High level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("High level"), DUMMY, 0, 100, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Low level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Low level"), DUMMY, 0, 100, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("MFI"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, 0, 100, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), MFI, 0, 100, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("High level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("High level"), DUMMY, 0, 100, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Low level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Low level"), DUMMY, 0, 100, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Medium level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Medium level"), DUMMY, 0, 100, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("ROC"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), ROC, QREAL_MIN, QREAL_MAX, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), ROC, QREAL_MIN, QREAL_MAX, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Level"), DUMMY, 0, 0, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("W%R"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, -100, 0, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), WILLR, -100, 0, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("High level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("High level"), DUMMY, -100, 0, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Low level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Low level"), DUMMY, -100, 0, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("Slow Stoch"))
  {
    QColor colorD, colorK;
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    colorD = paramDialog->getParam (CGLiteral ("%D color"));
    colorK = paramDialog->getParam (CGLiteral ("%K color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, 0, 100, colorD, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), STOCHSLOWD, 0, 100, colorD, "%D color");
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), STOCHSLOWK, 0, 100, colorK, "%K color");
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("High level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("High level"), DUMMY, 0, 100, colorK, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Low level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Low level"), DUMMY, 0, 100, colorK, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Medium level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Medium level"), DUMMY, 0, 100, colorK, CGLiteral (""));
  }

  if (fname == QLatin1String ("Fast Stoch"))
  {
    QColor colorD, colorK;
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    colorD = paramDialog->getParam (CGLiteral ("%D color"));
    colorK = paramDialog->getParam (CGLiteral ("%K color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, 0, 100, colorD, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), STOCHFASTD, 0, 100, colorD, "%D color");
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), STOCHFASTK, 0, 100, colorK, "%K color");
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("High level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("High level"), DUMMY, 0, 100, colorK, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Low level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Low level"), DUMMY, 0, 100, colorK, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Medium level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Medium level"), DUMMY, 0, 100, colorK, CGLiteral (""));
  }

  if (fname == QLatin1String ("MACD"))
  {
    QColor colorMACD, colorSignal;
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;

    colorMACD = paramDialog->getParam (CGLiteral ("MACD color"));
    colorSignal = paramDialog->getParam (CGLiteral ("Signal color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), MACD, QREAL_MIN, QREAL_MAX, QColor (Qt::white).rgb (), CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), MACD, QREAL_MIN, QREAL_MAX, colorMACD, CGLiteral ("MACD color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), MACDSIGNAL, QREAL_MIN, QREAL_MAX, colorSignal, CGLiteral ("Signal color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_VBARS);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), MACDHIST, QREAL_MIN, QREAL_MAX, QColor (Qt::white).rgb (), CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, 0);
    childobj->setAttributes (QTACHART_CLOSE, 0, CGLiteral (""), DUMMY, 0, 0, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("Bollinger Bands"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_CURVE);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), BBANDSMIDDLE, 0, 0, color, CGLiteral ("Color"));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), BBANDSUPPER, 0, 0, color, CGLiteral ("Color"));
    // childobj->setParamDialog (paramDialog->getPVector (), fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), BBANDSLOWER, 0, 0, color, CGLiteral ("Color"));
    // childobj->setParamDialog (paramDialog->getPVector (), fname);
  }

  if (fname == QLatin1String ("ADX"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), ADX, 0, 100, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), ADX, 0, 100, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Weak")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Weak"), DUMMY, 0, 100, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam ("Strong"));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Strong"), DUMMY, 0, 100, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam ("Very strong"));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Very strong"), DUMMY, 0, 100, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("Aroon"))
  {
    QColor colorUp, colorDown;
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    colorUp = paramDialog->getParam (CGLiteral ("Up color"));
    colorDown = paramDialog->getParam ("Down color");
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, 0, 100, colorDown, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), AROONDOWN, 0, 100, colorDown, "Down color");
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), AROONUP, 0, 100, colorUp, "Up color");
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("High level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("High level"), DUMMY, 0, 100, colorUp, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Low level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Low level"), DUMMY, 0, 100, colorUp, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Medium level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Medium level"), DUMMY, 0, 100, colorUp, CGLiteral (""));
  }

  if (fname == QLatin1String ("CCI"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, QREAL_MIN, QREAL_MAX, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), CCI, QREAL_MIN, QREAL_MAX, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("High level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("High level"), DUMMY, QREAL_MIN, QREAL_MAX, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Low level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Low level"), DUMMY, QREAL_MIN, QREAL_MAX,
                             color, CGLiteral (""));
  }

  if (fname == QLatin1String ("STDDEV"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DUMMY, 0, QREAL_MAX, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), STDDEV, 0, QREAL_MAX, color, CGLiteral ("Color"));
  }

  if (fname == QLatin1String ("Momentum"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), MOMENTUM, QREAL_MIN, QREAL_MAX, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), MOMENTUM, QREAL_MIN, QREAL_MAX, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Level")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Level"), DUMMY, 0, 0, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("DMI"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DMX, 0, 100, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), DMX, 0, 100, color, CGLiteral ("Color"));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam (CGLiteral ("Weak")));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Weak"), DUMMY, 0, 100, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam ("Strong"));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Strong"), DUMMY, 0, 100, color, CGLiteral (""));
    childobj = new QTACObject (indicator, QTACHART_OBJ_HLINE);
    childobj->setHLine (nullptr, paramDialog->getParam ("Very strong"));
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Very strong"), DUMMY, 0, 100, color, CGLiteral (""));
  }

  if (fname == QLatin1String ("ATR"))
  {
    period = (qint32) paramDialog->getParam (CGLiteral ("Period"));
    if (period < 1)
      return;
    color = paramDialog->getParam (CGLiteral ("Color"));
    indicator = new QTACObject (core, QTACHART_OBJ_SUBCHART);
    indicator->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), ATR, 0, QREAL_MAX, color, CGLiteral (""));
    indicator->setTitle (fname);
    childobj = new QTACObject (indicator, QTACHART_OBJ_CURVE);
    childobj->setAttributes (QTACHART_CLOSE, period, CGLiteral ("Period"), ATR, 0, QREAL_MAX, color, CGLiteral ("Color"));
  }

  if (indicator != nullptr)
    indicator->setParamDialog (paramDialog->getPVector (), fname);
}

// resize
void
QTACFunctions::resizeEvent (QResizeEvent * event)
{
  QSize newsize;
  int w, h, counter;

  if (event->oldSize () == event->size ())
    return;

  newsize = event->size ();
  w = newsize.width () - 2;
  h = newsize.height () - 2;
  layout->setGeometry (QRect (0, 0, w, h));
  for (counter = 0; counter < Button.size (); counter ++)
  {
    DynParamsDialog *paramDialog;
    layout->setRowMinimumHeight (counter % 6, Button[counter]->height ());
    layout->setColumnMinimumWidth (counter/6, Button[counter]->width ());
    layout->addWidget (Button[counter], counter % 6, counter / 6, Qt::AlignHCenter);
    paramDialog = Button[counter]->findChild<DynParamsDialog *> (CGLiteral ("ParamDialog"));
    if (paramDialog != nullptr)
    {
      paramDialog->move ((width () - paramDialog->width ()) / 2, 25);
    }
  }
}

// button clicked
void
QTACFunctions::button_clicked (void)
{
  DynParamsDialog *paramDialog;
  QPushButton *btn;
  btn = qobject_cast <QPushButton *> (QObject::sender());
  paramDialog = btn->findChild<DynParamsDialog *> (CGLiteral ("ParamDialog"));
  if (paramDialog != nullptr)
  {
    paramDialog->setReferenceChart (referencechart);
    paramDialog->move ((width () - paramDialog->width ()) / 2, 25);
    // paramDialog->exec ();
    paramDialog->setWindowFlags(Qt::FramelessWindowHint|Qt::Dialog);
    paramDialog->open ();
    qApp->processEvents(QEventLoop::AllEvents, 10);
  }
}

// function accepted
void
QTACFunctions::function_accepted (void)
{
  QTAChart *chart = static_cast <QTAChart *> (referencechart);
  DynParamsDialog *paramDialog;

  paramDialog = qobject_cast <DynParamsDialog *> (QObject::sender()->parent ());
  addIndicator (paramDialog);

  chart->goBack ();
}

// function rejected
void
QTACFunctions::function_rejected (void)
{

}

