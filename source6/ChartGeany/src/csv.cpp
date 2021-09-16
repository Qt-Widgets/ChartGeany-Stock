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

#include <QDate>
#include <QTextStream>
#include <QTemporaryFile>
#include "common.h"
#ifdef Q_OS_LINUX
#include <xls.h>
#else
#include "libxls/xls.h"
#endif
#include <cstdio>
#include <cstdlib>

// create csv file from raw YAHOO finance csv file
static CG_ERR_RESULT
yahoo2csv (const QString & namein, const QString & nameout)
{
  QFile rawcsv, csv;
  QString inputline;
  QStringList values;
  CG_ERR_RESULT retval = CG_ERR_OK;

  inputline.reserve (512);
  rawcsv.setFileName (namein);
  csv.setFileName (nameout);
  if (rawcsv.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
      rawcsv.close();
      return CG_ERR_OPEN_FILE;
    }
    QTextStream in (&rawcsv);
    QTextStream out (&csv);
    in.seek (0);
    inputline = in.readLine (0);    // header
    if (!in.atEnd ())
      inputline = in.readLine (0);  // first line

    while (!in.atEnd ())
    {
      inputline += CGLiteral (",00:00.00");
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
      QStringList column = inputline.split (CGLiteral(","),
                                            QString::KeepEmptyParts);
#else
      QVector<QStringRef> column = inputline.splitRef (CGLiteral(","),
                                   QString::KeepEmptyParts);
#endif

      if (column.size () >= 8)
      {
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
        const QString outputline =
          column[0] % CGLiteral (",") % // date
          column[1] % CGLiteral (",") % // open
          column[2] % CGLiteral (",") % // high
          column[3] % CGLiteral (",") % // low
          column[4] % CGLiteral (",") % // close
          column[6] % CGLiteral (",") % // volume
          column[5] % CGLiteral (",") % // adj close
          column[7]; // time
#else
        const QString outputline =
          column[0].toString () % CGLiteral (",") % // date
          column[1].toString () % CGLiteral (",") % // open
          column[2].toString () % CGLiteral (",") % // high
          column[3].toString () % CGLiteral (",") % // low
          column[4].toString () % CGLiteral (",") % // close
          column[6].toString () % CGLiteral (",") % // volume
          column[5].toString () % CGLiteral (",") % // adj close
          column[7].toString (); // time
#endif
        out << outputline << CGLiteral ("\n");
      }
      else
      {
        retval = CG_ERR_INVALID_DATA;
        goto yahoo2csv_end;
      }

      inputline = in.readLine (0);
    }

    if (inputline.size() > 0)
    {
      inputline += CGLiteral (",00:00.00");
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
      QStringList column = inputline.split (CGLiteral(","),
                                            QString::KeepEmptyParts);
#else
      QVector<QStringRef> column = inputline.splitRef (CGLiteral(","),
                                   QString::KeepEmptyParts);
#endif
      if (column.size () >= 8)
      {
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
        const QString outputline =
          column[0] % CGLiteral (",") % // date
          column[1] % CGLiteral (",") % // open
          column[2] % CGLiteral (",") % // high
          column[3] % CGLiteral (",") % // low
          column[4] % CGLiteral (",") % // close
          column[6] % CGLiteral (",") % // volume
          column[5] % CGLiteral (",") % // adj close
          column[7]; // time
#else
        const QString outputline =
          column[0].toString () % CGLiteral (",") % // date
          column[1].toString () % CGLiteral (",") % // open
          column[2].toString () % CGLiteral (",") % // high
          column[3].toString () % CGLiteral (",") % // low
          column[4].toString () % CGLiteral (",") % // close
          column[6].toString () % CGLiteral (",") % // volume
          column[5].toString () % CGLiteral (",") % // adj close
          column[7].toString (); // time
#endif
        out << outputline << CGLiteral ("\n");
      }
      else
      {
        retval = CG_ERR_INVALID_DATA;
        goto yahoo2csv_end;
      }
    }
  }
  else
    retval = CG_ERR_OPEN_FILE;

yahoo2csv_end:
  rawcsv.close ();
  csv.close ();
  return retval;
}

// create csv file from raw IEX json file
static CG_ERR_RESULT
iex2csv (const QString & namein, const QString & nameout)
{
  QFile rawjson, csv;
  QString line, outputline;
  QStringList values;
  CG_ERR_RESULT retval = CG_ERR_OK;

  rawjson.setFileName (namein);
  csv.setFileName (nameout);
  if (rawjson.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
      rawjson.close();
      return CG_ERR_OPEN_FILE;
    }

    QTextStream in (&rawjson);
    QTextStream out (&csv);
    in.seek (0);
    line = in.readAll ();   // json is just one line
    if (line.size () != 0)
    {
      QStringList node, value, jsondata;
      QString date, open, high, low, close, volume, adjclose,time = CGLiteral("00:00.00");

      line.remove ("[");
      line.remove ("]");
      jsondata = line.split(CGLiteral ("},"));
      foreach (QString data, jsondata)
      {
        data += CGLiteral ("}");
        data.replace ("}}", "}");
        if (json_parse (data, &node, &value, nullptr))
        {
          const qint32 n = node.size ();
          date = open = high = low = close = volume = CGLiteral ("");

          for (qint32 counter = 0; counter < n; counter ++)
          {
            if (node.at (counter) == QLatin1String ("date"))
              date = value.at (counter);
            else if (node.at (counter) == QLatin1String ("open"))
              open = value.at (counter);
            else if (node.at (counter) == QLatin1String ("high"))
              high = value.at (counter);
            else if (node.at (counter) == QLatin1String ("low"))
              low = value.at (counter);
            else if (node.at (counter) == QLatin1String ("close"))
              close = value.at (counter);
            else if (node.at (counter) == QLatin1String ("volume"))
              volume = value.at (counter);
          }
          adjclose = close;
          outputline = date % CGLiteral (",") %
                       open % CGLiteral (",") %
                       high % CGLiteral (",") %
                       low % CGLiteral (",") %
                       close % CGLiteral (",") %
                       volume % CGLiteral (",") %
                       adjclose % CGLiteral (",") %
                       time;
          out << outputline << CGLiteral("\n");
        }
      }
    }
  }
  else
    return CG_ERR_OPEN_FILE;

  rawjson.close ();
  csv.close ();
  return retval;
}

// create csv file from raw STANDARD csv file
static CG_ERR_RESULT
standard2csv (const QString & namein, const QString & nameout, QString & symbolname)
{
  QFile rawcsv, csv;
  QString inputline;
  QStringList values;
  CG_ERR_RESULT retval = CG_ERR_OK;

  inputline.reserve (512);
  rawcsv.setFileName (namein);
  csv.setFileName (nameout);
  if (rawcsv.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
      rawcsv.close();
      return CG_ERR_OPEN_FILE;
    }
    QTextStream in (&rawcsv);
    QTextStream out (&csv);
    in.seek (0);
    if (!in.atEnd ())
      inputline = in.readLine (0);  // first line
    while (!in.atEnd ())
    {
      QStringList column, datetok;
      QString outputline, datestr;
      inputline += CGLiteral (",00:00.00");
      column = inputline.split (CGLiteral (","), QString::KeepEmptyParts);

      if (column.size () < 7)
        return  CG_ERR_INVALID_DATA;

      symbolname = column[0];
      datetok = column[1].split (CGLiteral ("-"), QString::KeepEmptyParts);
      if (datetok.size () != 3)
        return  CG_ERR_INVALID_DATA;

      datestr = QString::number(datetok[2].toInt ()) % CGLiteral ("-");
      if (datetok.at (1) == QLatin1String ("Jan")) datestr += CGLiteral ("01");
      else if (datetok.at (1) == QLatin1String ("Feb")) datestr += CGLiteral ("02");
      else if (datetok.at (1) == QLatin1String ("Mar")) datestr += CGLiteral ("03");
      else if (datetok.at (1) == QLatin1String ("Apr")) datestr += CGLiteral ("04");
      else if (datetok.at (1) == QLatin1String ("May")) datestr += CGLiteral ("05");
      else if (datetok.at (1) == QLatin1String ("Jun")) datestr += CGLiteral ("06");
      else if (datetok.at (1) == QLatin1String ("Jul")) datestr += CGLiteral ("07");
      else if (datetok.at (1) == QLatin1String ("Aug")) datestr += CGLiteral ("08");
      else if (datetok.at (1) == QLatin1String ("Sep")) datestr += CGLiteral ("09");
      else if (datetok.at (1) == QLatin1String ("Oct")) datestr += CGLiteral ("10");
      else if (datetok.at (1) == QLatin1String ("Nov")) datestr += CGLiteral ("11");
      else if (datetok.at (1) == QLatin1String ("Dec")) datestr += CGLiteral ("12");
      else
        return  CG_ERR_INVALID_DATA;
#if QT_VERSION >= QT_VERSION_CHECK (5, 5, 0)
      datestr += CGLiteral ("-") % QString().asprintf("%02d", datetok[0].toInt ());
#else
      datestr += CGLiteral ("-") % QString().sprintf("%02d", datetok[0].toInt ());
#endif
      outputline = datestr % CGLiteral (",");
      outputline += column[2] % CGLiteral (",");
      outputline += column[3] % CGLiteral (",");
      outputline += column[4] % CGLiteral (",");
      outputline += column[5] % CGLiteral (",");
      outputline += column[6] % CGLiteral (",");
      outputline += column[5] % CGLiteral (",");
      outputline += column[7];
      out << outputline << CGLiteral ("\n");
      inputline = in.readLine (0);
    }
    if (inputline.size() > 0)
      out << inputline << CGLiteral ("\n");
  }
  else
    return CG_ERR_OPEN_FILE;

  rawcsv.close ();
  csv.close ();
  return retval;
}

// create csv file from AMI BROKER or METASTOCK7 csv file
static CG_ERR_RESULT
ami2csv (const QString & namein, const QString & nameout, QString & symbolname)
{
  QFile rawcsv, csv;
  QString inputline;
  QStringList values;
  CG_ERR_RESULT retval = CG_ERR_OK;

  inputline.reserve (512);
  rawcsv.setFileName (namein);
  csv.setFileName (nameout);
  if (rawcsv.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
      rawcsv.close();
      return CG_ERR_OPEN_FILE;
    }
    QTextStream in (&rawcsv);
    QTextStream out (&csv);
    in.seek (0);
    do
    {
      QStringList column, datetok;
      QString outputline, datestr;

      datestr.reserve (32);
      outputline.reserve (512);

      inputline = in.readLine (0);
      if (inputline.size() > 0)
      {
        inputline += CGLiteral (",00:00.00");
        column = inputline.split (",", QString::KeepEmptyParts);
        symbolname = column[0];
        datetok << column[1].right (2);
        datetok << column[1].mid (4,2);
        datetok << column[1].left (4);
        datestr = datetok[2] % underscore % datetok.at (1) % underscore % datetok[0];
        outputline = datestr   % CGLiteral (",") %
                     column[2] % CGLiteral (",") %
                     column[3] % CGLiteral (",") %
                     column[4] % CGLiteral (",") %
                     column[5] % CGLiteral (",") %
                     column[6] % CGLiteral (",") %
                     column[5] % CGLiteral (",") %
                     column[7];
        out << outputline << "\n";
      }
    }
    while (!in.atEnd ());
  }
  else
    return CG_ERR_OPEN_FILE;

  rawcsv.close ();
  csv.close ();
  return retval;
}

// create csv file from AMI BROKER or METASTOCK8 csv file
static CG_ERR_RESULT
metastock8csv (const QString & namein, const QString & nameout, QString & symbolname)
{
  QFile rawcsv, csv;
  QString inputline;
  QStringList values;
  CG_ERR_RESULT retval = CG_ERR_OK;

  rawcsv.setFileName (namein);
  csv.setFileName (nameout);
  if (rawcsv.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
      rawcsv.close();
      return CG_ERR_OPEN_FILE;
    }
    QTextStream in (&rawcsv);
    QTextStream out (&csv);
    in.seek (0);
    if (!in.atEnd ())
      inputline = in.readLine (0);  // first line
    while (!in.atEnd ())
    {
      QStringList column, datetok;
      QString outputline, datestr;
      inputline += CGLiteral (",00:00.00");
      column = inputline.split (",", QString::KeepEmptyParts);
      symbolname = column[0];
      datetok << column[2].right (2);
      datetok << column[2].mid (4,2);
      datetok << column[2].left (4);
      datestr = datetok[2];
      datestr += CGLiteral ("-");
      datestr += datetok.at (1);
      datestr += CGLiteral ("-");
      datestr += datetok[0];
      outputline = datestr % CGLiteral (",");
      outputline += column[3] % CGLiteral (",");
      outputline += column[4] % CGLiteral (",");
      outputline += column[5] % CGLiteral (",");
      outputline += column[6] % CGLiteral (",");
      outputline += column[7] % CGLiteral (",");
      outputline += column[6] % CGLiteral (",");
      outputline += column[8];
      out << outputline << "\n";
      inputline = in.readLine (0);
    }
    if (inputline.size() > 0)
      out << inputline << "\n";
  }
  else
    return CG_ERR_OPEN_FILE;

  rawcsv.close ();
  csv.close ();
  return retval;
}

// create csv file from raw YAHOO finance csv file
static CG_ERR_RESULT
twelvedata2csv (const QString & namein, const QString & nameout)
{
  QFile rawcsv, csv;
  QString inputline, outputline;
  QStringList values, column;
  CG_ERR_RESULT retval = CG_ERR_OK;

  inputline.reserve (512);
  rawcsv.setFileName (namein);
  csv.setFileName (nameout);
  if (rawcsv.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
      rawcsv.close();
      return CG_ERR_OPEN_FILE;
    }
    QTextStream in (&rawcsv);
    QTextStream out (&csv);
    in.seek (0);
    inputline = in.readLine (0);    // header
    if (!in.atEnd ())
      inputline = in.readLine (0);  // first line
    while (!in.atEnd ())
    {
      inputline += CGLiteral (";00:00.00");
      column = inputline.split (CGLiteral(";"), QString::KeepEmptyParts);

      if (column.size () >= 5)
      {
        outputline =
          column[0] % CGLiteral (",") % // date
          column[1] % CGLiteral (",") % // open
          column[2] % CGLiteral (",") % // high
          column[3] % CGLiteral (",") % // low
          column[4] % CGLiteral (",") % // close
          column[5] % CGLiteral (",") % // volume
          column[4] % CGLiteral (",") % // adj close
          column[6]; // time
        out << outputline << CGLiteral ("\n");
      }
      else
      {
        retval = CG_ERR_INVALID_DATA;
        goto twelvedata2csv_end;
      }

      inputline = in.readLine (0);
    }

    if (inputline.size() > 0)
    {
      inputline += CGLiteral (";00:00.00");
      column = inputline.split (CGLiteral(";"), QString::KeepEmptyParts);
      \

      if (column.size () >= 6)
      {
        outputline =
          column[0] % CGLiteral (",") % // date
          column[1] % CGLiteral (",") % // open
          column[2] % CGLiteral (",") % // high
          column[3] % CGLiteral (",") % // low
          column[4] % CGLiteral (",") % // close
          column[5] % CGLiteral (",") % // volume
          column[4] % CGLiteral (",") % // adj close
          column[6]; // time
        out << outputline << CGLiteral ("\n");
      }
      else
      {
        retval = CG_ERR_INVALID_DATA;
        goto twelvedata2csv_end;
      }
    }
  }
  else
    retval = CG_ERR_OPEN_FILE;

twelvedata2csv_end:
  rawcsv.close ();
  csv.close ();
  return retval;
}

// create csv file from Microsoft Excel file
struct xlsdata
{
  QString Open;
  QString High;
  QString Low;
  QString Close;
  QString AdjClose;
  QString Volume;
  QString Date;
  QString Time;
};
Q_DECLARE_TYPEINFO (xlsdata, Q_MOVABLE_TYPE);
typedef QList <xlsdata> XLSData;

typedef enum
{
  XLSOPEN,
  XLSHIGH,
  XLSLOW,
  XLSCLOSE,
  XLSVOLUME,
  XLSDATE,
  XLSTIME,
  XLSADJCLOSE
} xlscolumn;

// convert excel serial date to day, month and year
static void
ExcelSerialDateToDMY(int nSerialDate, int &nDay,
                     int &nMonth, int &nYear)
{
  // Excel/Lotus 123 have a bug with 29-02-1900. 1900 is not a
  // leap year, but Excel/Lotus 123 think it is...
  if (nSerialDate == 60)
  {
    nDay    = 29;
    nMonth    = 2;
    nYear    = 1900;

    return;
  }
  else if (nSerialDate < 60)
  {
    // Because of the 29-02-1900 bug, any serial date
    // under 60 is one off... Compensate.
    nSerialDate++;
  }

  // Modified Julian to DMY calculation with an addition of 2415019
  int l = nSerialDate + 68569 + 2415019;
  int n = int(( 4 * l ) / 146097);
  l = l - int(( 146097 * n + 3 ) / 4);
  int i = int(( 4000 * ( l + 1 ) ) / 1461001);
  l = l - int(( 1461 * i ) / 4) + 31;
  int j = int(( 80 * l ) / 2447);
  nDay = l - int(( 2447 * j ) / 80);
  l = int(j / 11);
  nMonth = j + 2 - ( 12 * l );
  nYear = 100 * ( n - 49 ) + i + l;
}

// fill excel data from raw data
static void
fillxlsdata (QStringList &rawdata, XLSData &exceldata, int start, int pivot, int col)
{
  int maxcounter = rawdata.size ();
  for (int counter = start, dcounter = 1;
       counter < maxcounter;
       counter += pivot, dcounter ++)
  {
    switch (col)
    {
    case XLSOPEN:
      exceldata[dcounter].Open = rawdata[counter];
      break;
    case XLSCLOSE:
      exceldata[dcounter].Close = rawdata[counter];
      break;
    case XLSHIGH:
      exceldata[dcounter].High = rawdata[counter];
      break;
    case XLSLOW:
      exceldata[dcounter].Low = rawdata[counter];
      break;
    case XLSADJCLOSE:
      exceldata[dcounter].AdjClose = rawdata[counter];
      break;
    case XLSVOLUME:
      exceldata[dcounter].Volume = rawdata[counter];
      break;
    case XLSDATE:
      rawdata[counter] = rawdata[counter].remove ("'");
      if (rawdata[counter].contains ('-', Qt::CaseInsensitive))
        exceldata[dcounter].Date = rawdata[counter];
      else
      {
        QString datestr;
        int day, month, year, serial;
        serial = (int) rawdata[counter].toFloat ();
        ExcelSerialDateToDMY (serial, day, month, year);
#if QT_VERSION >= QT_VERSION_CHECK (5, 5, 0)
        exceldata[dcounter].Date = datestr.asprintf ("%04d-%02d-%02d", year, month, day);
#else
        exceldata[dcounter].Date = datestr.sprintf ("%04d-%02d-%02d", year, month, day);
#endif
      }
      break;
    case XLSTIME:
      exceldata[dcounter].Time = rawdata[counter];
      break;
    }
  }
}

static CG_ERR_RESULT
excel2csv (const QString & namein, const QString & nameout, QString & symbolname)
{
  Q_UNUSED (symbolname)

  QFile csv;
  QStringList rawdata;
  xls::xlsWorkBook *pWB;
  xls::xlsWorkSheet *pWS;
  xls::WORD maxcol = 0, maxrow = 0, rowcounter = 0;
  CG_ERR_RESULT result = CG_ERR_OK;

  if(!QFile::exists(namein))
    return CG_ERR_OPEN_FILE;

  // parse xls sheet
  pWB = xls::xls_open (namein.toLocal8Bit ().data (), "UTF-8");

  // process all sheets
  // for (quint32 i = 0; i < pWB->sheets.count; i++)
  // {
  // open and parse the sheet
  pWS = xls_getWorkSheet(pWB, 0);
  xls_parseWorkSheet(pWS);
  maxrow = pWS->rows.lastrow + 1;
  maxcol = pWS->rows.lastcol;

  // process all rows of the sheet
  for (xls::WORD cellRow = 0; cellRow < maxrow; cellRow++)
  {
    for (xls::WORD cellCol = 0; cellCol < maxcol; cellCol++)
    {
      xls::xlsCell* cell = xls::xls_cell(pWS, cellRow, cellCol);

      if (cell->str != nullptr)
      {
        char *xstr;
        xstr = (char *) malloc (strlen ((const char *) cell->str) + 32);
        if (xstr != nullptr)
        {
          strcpy (xstr, (const char *) cell->str);
          if (strtod ((const char *) xstr, NULL) == 0 && cell->d != 0)
            sprintf ((char *) xstr, "%.4f", cell->d);
          rawdata += QString ((QString ((const char *) xstr).toUtf8 ())).toUpper ();
          free (xstr);
        }
        else
          rawdata += CGLiteral ("");
      }
      else
        rawdata += CGLiteral ("");
    }
    rowcounter ++;
  }
  xls_close_WS(pWS);
  if (GlobalProgressBar != nullptr)
    if (GlobalProgressBar->value () < 101)
      GlobalProgressBar->setValue (GlobalProgressBar->value () + 5);
  // }
  xls_close(pWB);

  // create csv file
  csv.setFileName (nameout);
  if (!csv.open (QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    return CG_ERR_OPEN_FILE;

  QTextStream out (&csv);
  XLSData data;
  struct xlsdata dummy;
  dummy.Open = "OPEN";
  dummy.Close = "CLOSE";
  dummy.High = "HIGH";
  dummy.Low = "LOW";
  dummy.AdjClose = "ADJ CLOSE";
  dummy.Volume = "VOLUME";
  dummy.Date = "DATE";
  dummy.Time = "TIME";
  data += dummy;

  dummy.Open = dummy.Close = "0.0";
  dummy.High = dummy.Low = dummy.AdjClose = "0.0";
  dummy.Volume = "0";
  dummy.Date = "0000-00-00";
  dummy.Time = "00:00.00";
  maxrow = rowcounter;

  for (xls::WORD counter = 1; counter < maxrow; counter ++)
    data += dummy;

  for (xls::WORD counter = 0; counter < maxcol; counter ++)
  {
    if (rawdata[counter] == "OPEN")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSOPEN);
    else if (rawdata[counter] == "CLOSE")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSCLOSE);
    else if (rawdata[counter] == "HIGH")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSHIGH);
    else if (rawdata[counter] == "LOW")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSLOW);
    else if (rawdata[counter] == "ADJ CLOSE")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSADJCLOSE);
    else if (rawdata[counter] == "DATE")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSDATE);
    else if (rawdata[counter] == "TIME")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSTIME);
    else if (rawdata[counter] == "VOLUME")
      fillxlsdata (rawdata, data, counter + maxcol, maxcol, XLSVOLUME);
  }

  if (GlobalProgressBar != nullptr)
    if (GlobalProgressBar->value () < 101)
      GlobalProgressBar->setValue (GlobalProgressBar->value () + 5);

  QString outputline;
  outputline.reserve (1048576);
  for (xls::WORD counter = 1; counter < maxrow; counter ++)
  {
    outputline = data[counter].Date;
    outputline += CGLiteral (",") % data[counter].Open %
                  CGLiteral (",") % data[counter].High %
                  CGLiteral (",") % data[counter].Low %
                  CGLiteral (",") % data[counter].Close %
                  CGLiteral (",") % data[counter].Volume %
                  CGLiteral (",") % data[counter].AdjClose %
                  CGLiteral (",") % data[counter].Time;
    out << outputline << "\n";
  }
  if (GlobalProgressBar != nullptr)
    if (GlobalProgressBar->value () < 101)
      GlobalProgressBar->setValue (GlobalProgressBar->value () + 5);

  csv.close ();
  return result;
}

/// General purpose csv functions
// Return and SQL statement to drop a view if exists
static QString
dropview (QString viewname)
{
  return CGLiteral ("DROP VIEW IF EXISTS ") % viewname % CGLiteral (";");
}

// Returns an SQL statement that creates the timeframe views
static QString
tfview (QString tablename, QString tf, SymbolEntry *data, QString operation, bool adjust)
{
  QString SQLCommand,
          vname,
          viewclose, viewopen, viewhigh, viewlow, viewmindate,
          viewmaxdate, viewvolume;
  
  SQLCommand.truncate (0);
  vname = tablename % underscore % tf;

  if (operation == QLatin1String ("UPDATE"))
    goto tfview_end;

  viewclose = vname % underscore % CGLiteral ("_CLOSE");
  viewopen = vname % underscore % CGLiteral ("_OPEN");
  viewhigh = vname % underscore % CGLiteral ("_HIGH");
  viewlow = vname % underscore % CGLiteral ("_LOW");
  viewmindate = vname % underscore % CGLiteral ("_MINDATE");
  viewmaxdate = vname % underscore % CGLiteral ("_MAXDATE");
  viewvolume = vname % underscore % CGLiteral ("_VOLUME");

  // drop all views
  SQLCommand.reserve (1048576);
  SQLCommand.append
  (dropview (vname) %
   dropview (viewclose) %
   dropview (viewopen) %
   dropview (viewhigh) %
   dropview (viewlow) %
   dropview (viewmindate) %
   dropview (viewmaxdate) %
   dropview (viewvolume));
  SQLCommand.append ('\n');

  // maxdate
  SQLCommand.append (CGLiteral ("CREATE VIEW ") % viewmaxdate %
                     CGLiteral (" AS SELECT MAX (DATE) AS DATE, ") % tf % CGLiteral (" FROM ") %
                     tablename % CGLiteral (" GROUP BY ") % tf % CGLiteral (";"));
  SQLCommand.append ('\n');

  // mindate
  SQLCommand.append (CGLiteral ("CREATE VIEW ") % viewmindate %
                     CGLiteral (" AS SELECT MIN (DATE) AS DATE, ") % tf %
                     CGLiteral (" FROM ") % tablename % CGLiteral (" GROUP BY ") %
                     tf % CGLiteral (";"));
  SQLCommand.append ('\n');

  // tf low
  SQLCommand += CGLiteral ("CREATE VIEW ") % viewlow %
                CGLiteral (" AS SELECT MIN (LOW) AS LOW, ") % tf % CGLiteral (" FROM ") %
                tablename % CGLiteral (" GROUP BY ") % tf % CGLiteral (";");
  SQLCommand.append ('\n');

  // tf high
  SQLCommand += CGLiteral ("CREATE VIEW ") % viewhigh %
                CGLiteral (" AS SELECT MAX (HIGH) AS HIGH, ") % tf % CGLiteral (" FROM ") %
                tablename % CGLiteral (" GROUP BY ") % tf % CGLiteral (";");
  SQLCommand.append ('\n');

  // tf open
  SQLCommand += CGLiteral ("CREATE VIEW ") % viewopen %
                CGLiteral (" AS SELECT ") % tablename % CGLiteral (".OPEN, ") %
                tablename % dot % tf % CGLiteral (", ") %
                tablename % CGLiteral (".DATE, ") %
                tablename % CGLiteral (".TIME FROM ") %
                tablename % CGLiteral (", ") % viewmindate % CGLiteral (" WHERE ") %
                tablename % CGLiteral (".DATE = ") % viewmindate % CGLiteral (".DATE AND ") %
                tablename % dot % tf % CGLiteral (" = ") % viewmindate %
                dot % tf % CGLiteral (";");
  SQLCommand.append ('\n');

  // tf close
  SQLCommand += CGLiteral ("CREATE VIEW ") % viewclose %
                CGLiteral (" AS SELECT ") % tablename % CGLiteral (".CLOSE, ") %
                tablename % CGLiteral (".CLOSE AS ADJCLOSE, ") %
                tablename % dot % tf % CGLiteral (" FROM ") % tablename %
                CGLiteral (", ") % viewmaxdate % CGLiteral (" WHERE ") %
                tablename % CGLiteral (".DATE = ") % viewmaxdate % CGLiteral (".DATE AND ") %
                tablename % dot % tf % CGLiteral (" = ") % viewmaxdate %
                dot % tf % CGLiteral (";");
  SQLCommand.append ('\n');

  // tf volume
  SQLCommand += CGLiteral ("CREATE VIEW ") % viewvolume %
                CGLiteral (" AS SELECT SUM (") % tablename % CGLiteral (".VOLUME ) AS VOLUME, ") %
                tablename % dot % tf % CGLiteral (" FROM ") %
                tablename % CGLiteral (" GROUP BY ") % tf % CGLiteral (";");
  SQLCommand.append ('\n');

  // final tf view
  SQLCommand += CGLiteral ("CREATE VIEW ") % vname %
                CGLiteral (" AS SELECT ") %
                viewhigh % CGLiteral (".HIGH AS HIGH, ") %
                viewlow % CGLiteral (".LOW AS LOW, ") %
                viewopen % CGLiteral (".OPEN AS OPEN, ") %
                viewclose % CGLiteral (".CLOSE AS CLOSE, ") %
                viewvolume % CGLiteral (".VOLUME AS VOLUME, ") %
                viewopen % CGLiteral (".DATE AS DATE, ") %
                viewopen % CGLiteral (".TIME AS TIME, ") %
                viewclose % CGLiteral (".ADJCLOSE AS ADJCLOSE FROM ") %
                viewhigh % CGLiteral (", ") % viewlow % CGLiteral (", ") % viewopen % CGLiteral (", ") %
                viewclose % CGLiteral (", ") % viewvolume % CGLiteral (" WHERE ") %
                viewhigh % dot % tf % CGLiteral (" = ") % viewlow % dot % tf % CGLiteral (" AND ") %
                viewhigh % dot % tf % CGLiteral (" = ") % viewopen % dot % tf % CGLiteral (" AND ") %
                viewhigh % dot % tf % CGLiteral (" = ") % viewclose % dot % tf % CGLiteral (" AND ") %
                viewhigh % dot % tf % CGLiteral (" = ") % viewvolume % dot % tf % CGLiteral (";");
  SQLCommand.append ('\n');

  // remove symbol from symbols table
  SQLCommand += CGLiteral ("delete from SYMBOLS where KEY='") % vname % CGLiteral ("';");
  SQLCommand.append ('\n');

  // insert symbol into symbols table
  SQLCommand += CGLiteral("INSERT INTO SYMBOLS VALUES ('") %
                data->symbol % CGLiteral("','") %
                data->name % CGLiteral("','") %
                data->market % CGLiteral("','") %
                data->source % CGLiteral("','") %
                tf % CGLiteral("','") %
                CGLiteral(" ") % CGLiteral("','") %
                CGLiteral(" ") % CGLiteral("','") %
                vname % CGLiteral("','") %
                data->currency % CGLiteral("',0,'") %
                data->dnlstring % CGLiteral("',");
  if (adjust)
    SQLCommand += CGLiteral("'YES', ");
  else
    SQLCommand += CGLiteral("'NO', ");
  SQLCommand += CGLiteral ("'") % data->tablename % CGLiteral("', '") % data->format % CGLiteral("', 0, '');");
  SQLCommand.append ('\n');

tfview_end:
  // update DATEFROM, DATETO and TFRESOLUTION of symbol entry

  SQLCommand += CGLiteral("update SYMBOLS set DATEFROM = (select min(DATE) from ") %
                vname % CGLiteral(") where KEY='") % vname % CGLiteral("';") %
				'\n' %
				CGLiteral("update SYMBOLS set DATETO = (select max(DATE) from ") %
                vname % CGLiteral(") where KEY='") % vname % CGLiteral("';") %
				'\n' %
				CGLiteral("update SYMBOLS set TFRESOLUTION = (select MINUTE_RESOLUTION ") %
                CGLiteral("from TIMEFRAMES where SYMBOLS.TIMEFRAME = TIMEFRAMES.TIMEFRAME);");

  return SQLCommand;
}

// CSV Line 2 SQL
// Returns an SQL INSERT command
const QString
csvline2SQL (QString &csvline, QString &tablename)
{
  QDate datevar;
  QString lastclose = CGLiteral ("0.001"), daynum, weeknum;

  const QString inputline = csvline;
  QStringList column = inputline.split(comma, QString::KeepEmptyParts);

  if (column.size () < 8)
  {
    for (qint32 counter = column.size (); counter < 8; counter ++)
      column += CGLiteral ("");
  }

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
  QStringList yyyymmdd = column[0].split (CGLiteral("-"),
                                          QString::KeepEmptyParts);
#else
  QVector<QStringRef> yyyymmdd = column[0].splitRef (CGLiteral("-"),
                                                     QString::KeepEmptyParts);
#endif
  
  if (yyyymmdd.size () != 3)
    return CGLiteral ("");

  for (qint32 counter = 0; counter < 8; counter ++)
  {
    if (column[counter].size () > 17)
      column[counter].truncate (0);

    if (column[counter].isEmpty ())
      column[counter] = CGLiteral ("0");
  }

  // ATTENTION: Fix zero values. Possible FDIV error
  if (column[4].toFloat () <= 0.0001)
    column[4] = lastclose;

  if (column[1].toFloat () <= 0.0001)
    column[1] = column[4];

  if (column[2].toFloat () <= 0.0001)
    column[2] = column[4];

  if (column[3].toFloat () <= 0.0001)
    column[3] = column[4];

  if (column[5].toFloat () == 0.0)
    column[5] = CGLiteral ("0");

  if (column[6].toFloat () <= 0.0001)
    column[6] = column[4];
  // END OF FIX

  datevar.setDate (yyyymmdd[0].toInt (),
                   yyyymmdd[1].toInt (),
                   yyyymmdd[2].toInt ());
  daynum = QString::number(datevar.dayOfYear ());
  if (daynum.size () == 1)
    daynum = CGLiteral ("00") % daynum;
  if (daynum.size () == 2)
    daynum = CGLiteral ("0") % daynum;
  daynum = yyyymmdd[0] % daynum;

  if (datevar.weekNumber (nullptr) == 1 &&  yyyymmdd[1].toInt () == 12)
  {
    weeknum = CGLiteral ("01");
    weeknum = QString::number (yyyymmdd[0].toInt () + 1) % weeknum;
  }
  else
  {
    weeknum = QString::number(datevar.weekNumber (nullptr));
    if (weeknum.size () == 1)
      weeknum = CGLiteral ("0")% weeknum;
    weeknum = yyyymmdd[0] % weeknum;
  }

  const QString SQL = CGLiteral ("insert into ") % tablename %
        CGLiteral (" (OPEN, HIGH, LOW, CLOSE, VOLUME, ADJCLOSE, MONTH, YEAR, DAY, WEEK, DATE, TIME) values (") %
        column[1] % CGLiteral (",") %
        column[2] % CGLiteral (",") %
        column[3] % CGLiteral (",") %
        column[4] % CGLiteral (",") %
        column[5] % CGLiteral (",") %
        column[6] % CGLiteral (",") %
        yyyymmdd[0] % yyyymmdd[1] % CGLiteral (",") %
        yyyymmdd[0] % CGLiteral (",") %
        daynum % CGLiteral (",") %
        weeknum % CGLiteral (",") %
        CGLiteral ("'") % column[0] % CGLiteral ("',") %
        CGLiteral ("'") % column[7] % CGLiteral ("');");

  return SQL;
}

// check if view adjusted exists
static int
sqlcb_checkexistence (void *cnt, int argc, char **argv, char **column)
{
  Q_UNUSED (column)
  Q_UNUSED (argc)
  int *counter;

  counter = (int *) cnt;
  *counter = QString (argv[0]).toInt ();

  return 0;
}

// CSV 2 SQLITE:
// Returns CG_ERR_OK on success, an error otherwise
CG_ERR_RESULT
csv2sqlite (SymbolEntry *data, QString operation)
{
  QFile tmpcsv;
  QTemporaryFile *tempfile;
  QString tempfilename, SQLCommand = "", inputline,
                        indexname, adjustedviewname,
                        symbol;
  CG_ERR_RESULT result = CG_ERR_OK;
  int rc;

  SQLCommand.reserve (12582912); //reserve 12M
  tablename_normal (data->tablename);
  tablename_normal (data->tmptablename);

  adjustedviewname = data->tablename % CGLiteral("_ADJUSTED");
  data->name.remove ("'");

  tempfile = new QTemporaryFile ;
  if (tempfile->open ())
    tempfilename = tempfile->fileName ();
  else
  {
    result = CG_ERR_OPEN_FILE;
    setGlobalError(result, __FILE__, __LINE__);
  }
  delete tempfile;

  if (result == CG_ERR_OK)
  {
    if (data->format == QLatin1String ("YAHOO CSV"))
    {
      data->adjust = true;
      result = yahoo2csv (data->csvfile, tempfilename);
    }

    if (data->format == QLatin1String ("IEX JSON"))
    {
      data->adjust = false;
      result = iex2csv (data->csvfile, tempfilename);
    }

    if (data->format == QLatin1String ("TWELVEDATA CSV"))
    {
      data->adjust = false;
      result = twelvedata2csv (data->csvfile, tempfilename);
    }

    if (data->format == QLatin1String ("STANDARD CSV"))
    {
      data->adjust = false;
      result = standard2csv (data->csvfile, tempfilename, symbol);
    }

    if (data->format == QLatin1String ("AMI BROKER") ||
        data->format == QLatin1String ("METASTOCK ASCII 7"))
    {
      data->adjust = false;
      result = ami2csv (data->csvfile, tempfilename, symbol);
    }

    if (data->format == QLatin1String ("METASTOCK ASCII 8"))
    {
      data->adjust = false;
      result = metastock8csv (data->csvfile, tempfilename, symbol);
    }

    if (data->format == QLatin1String ("MICROSOFT EXCEL"))
    {
      // data->adjust = true;
      result = excel2csv (data->csvfile, tempfilename, symbol);
    }
  }

  if (result != CG_ERR_OK)
    goto csv2sqlite_end;

  // drop temporary table
  SQLCommand += CGLiteral ("drop table if exists ") % data->tmptablename % CGLiteral ("; ");

  // create temporary table
  SQLCommand += CGLiteral ("create temporary table ") % data->tmptablename %
                CGLiteral (" as select * from DATAMODEL;");

  // fill temporary table
  tmpcsv.setFileName (tempfilename);
  if (tmpcsv.open (QIODevice::ReadOnly|QIODevice::Text))
  {
    QTextStream in (&tmpcsv);
    do
    {
      inputline = in.readLine (0);

      if (GlobalProgressBar != nullptr)
        if (GlobalProgressBar->value () < 101)
          GlobalProgressBar->setValue (GlobalProgressBar->value () + 1);

      const QString SQL = csvline2SQL (inputline, data->tmptablename);

      if (!SQL.isEmpty ())
      {
        SQLCommand.append ('\n');
        SQLCommand.append (SQL);
      }

    }
    while (!in.atEnd ());
    tmpcsv.close ();
  }
  SQLCommand.append ('\n');

  // drop data table
  SQLCommand += CGLiteral ("drop table if exists ") % data->tablename % CGLiteral ("; ") %
                CGLiteral ("drop view if exists ") % data->tablename % CGLiteral ("; ") %
                '\n' %

  // create data table
				CGLiteral ("create table ") % data->tablename %
                CGLiteral (" (OPEN REAL NOT NULL, HIGH REAL  NOT NULL,") %
                CGLiteral (" LOW REAL NOT NULL, CLOSE REAL NOT NULL, VOLUME INTEGER,") %
                CGLiteral (" ADJCLOSE REAL NOT NULL, DATE TEXT NOT NULL, TIME TEXT,") %
                CGLiteral (" MONTH INTEGER, YEAR INTEGER, DAY INTEGER, WEEK INTEGER, ") %
                CGLiteral (" TICK INTEGER PRIMARY KEY AUTOINCREMENT);");
  SQLCommand.append ('\n');

  // copy temporary table to data table
  SQLCommand += CGLiteral ("insert into ") % data->tablename %
                CGLiteral (" (OPEN, HIGH, LOW, CLOSE, VOLUME, ADJCLOSE, MONTH, YEAR, DAY, WEEK, DATE, TIME)") %
                CGLiteral (" select OPEN, HIGH, LOW, CLOSE, VOLUME, ADJCLOSE, MONTH, YEAR, DAY, WEEK, DATE, TIME from ") %
                data->tmptablename % CGLiteral (" GROUP BY DATE, TIME ORDER BY DATE ASC;") %
                '\n' %

				CGLiteral ("DELETE FROM ") % data->tablename %
                CGLiteral (" WHERE OPEN = 0.001 AND CLOSE = 0.001 AND HIGH = 0.001 AND LOW = 0.001; ") %
				CGLiteral ("UPDATE ") % data->tablename %
                CGLiteral (" SET HIGH = OPEN WHERE OPEN >= CLOSE AND HIGH < OPEN;") %
				CGLiteral ("UPDATE ") % data->tablename %
                CGLiteral (" SET LOW = CLOSE WHERE OPEN >= CLOSE AND LOW > CLOSE;") %
				CGLiteral ("UPDATE ") % data->tablename %
                CGLiteral (" SET HIGH = CLOSE WHERE OPEN <= CLOSE AND HIGH < CLOSE;") %
				CGLiteral ("UPDATE ") % data->tablename %
                CGLiteral (" SET LOW = CLOSE WHERE OPEN <= CLOSE AND LOW > OPEN;") %

  // correct ADJCLOSE if invalid
				CGLiteral ("update ") % data->tablename %
                CGLiteral (" set ADJCLOSE = 0 where CLOSE = 0;") %
                CGLiteral ("update ") % data->tablename %
                CGLiteral (" set ADJCLOSE = CLOSE where ADJCLOSE < 0;") %
                '\n' %

  // remove symbol from symbols table
				CGLiteral ("delete from SYMBOLS where KEY='") % data->tablename % CGLiteral ("';") %
				'\n' %

  // insert symbol into symbols table
				CGLiteral ("insert into SYMBOLS values ('") %
                data->symbol % CGLiteral ("','") %
                data->name % CGLiteral ("','") %
                data->market % CGLiteral ("','") %
                data->source % CGLiteral ("','") %
                data->timeframe % CGLiteral ("','") %
                CGLiteral (" ") % CGLiteral ("','") %
                CGLiteral (" ") % CGLiteral ("','") %
                data->tablename % CGLiteral ("','") %
                data->currency % CGLiteral ("',0,'") %
                data->dnlstring % CGLiteral ("',") %
                CGLiteral ("'NO', ") %
                CGLiteral ("'") % data->tablename % CGLiteral ("', '") % data->format % CGLiteral ("', 0, '');");
  SQLCommand.append ('\n');
  SQLCommand += CGLiteral ("INSERT OR IGNORE INTO CURRENCIES (SYMBOL) VALUES ('") %
                data->currency % CGLiteral ("');") %

                // update DATEFROM, DATETO and TFRESOLUTION of symbol entry
                CGLiteral ("update SYMBOLS set DATEFROM = (select min(DATE) from ") %
                data->tablename % CGLiteral (") where KEY='") % data->tablename %
                CGLiteral ("';");
  SQLCommand.append ('\n');

  SQLCommand += CGLiteral ("update SYMBOLS set DATETO = (select max(DATE) from ") %
                data->tablename % CGLiteral (") where KEY='") % data->tablename %
                CGLiteral ("';");
  SQLCommand.append ('\n');

  SQLCommand += CGLiteral ("update SYMBOLS set TFRESOLUTION = (select MINUTE_RESOLUTION ") %
                CGLiteral ("from TIMEFRAMES where SYMBOLS.TIMEFRAME = TIMEFRAMES.TIMEFRAME);");

  // drop temporary table
  SQLCommand += CGLiteral ("drop table if exists ") % data->tmptablename % CGLiteral ("; ");
  SQLCommand.append ('\n');

  // create indexes
  indexname = data->tablename % CGLiteral ("_monthidx");
  SQLCommand += CGLiteral ("CREATE INDEX '") % indexname % CGLiteral ("' on ") %
                data->tablename % CGLiteral (" (MONTH ASC);");
  SQLCommand.append ('\n');

  indexname = data->tablename % CGLiteral ("_dateidx");
  SQLCommand += CGLiteral ("CREATE INDEX '") % indexname % CGLiteral ("' on ") %
                data->tablename % CGLiteral (" (DATE ASC);");
  SQLCommand.append ('\n');

  indexname = data->tablename % CGLiteral ("_weekidx");
  SQLCommand += CGLiteral ("CREATE INDEX '") % indexname % CGLiteral ("' on ") %
                data->tablename % CGLiteral (" (WEEK ASC);");
  SQLCommand.append ('\n');

  indexname = data->tablename % CGLiteral ("_yearidx");
  SQLCommand += CGLiteral ("CREATE INDEX '") % indexname % CGLiteral ("' on ") %
                data->tablename % CGLiteral (" (YEAR ASC);");
  SQLCommand.append ('\n');

  // create week, month, year views for unadjusted data
  if (operation == QLatin1String ("CREATE") /*&& data->source != "CSV"*/)
  {
    SQLCommand += tfview (data->tablename, CGLiteral ("WEEK"), data, operation, false) %
                  tfview (data->tablename, CGLiteral ("MONTH"), data, operation, false) %
                  tfview (data->tablename, CGLiteral ("YEAR"), data, operation, false);
  }

  // create adjusted view
  if (operation == QLatin1String ("CREATE") && data->adjust == true)
  {
    SQLCommand += dropview (adjustedviewname);
    SQLCommand.append ('\n');
    SQLCommand +=
      CGLiteral ("CREATE VIEW ") % adjustedviewname %
      CGLiteral (" AS SELECT ") % data->tablename %
      CGLiteral (".OPEN*(") % data->tablename %
      CGLiteral (".ADJCLOSE/") % data->tablename %
      CGLiteral (".CLOSE) AS OPEN,") % data->tablename %
      CGLiteral (".HIGH*(") % data->tablename %
      CGLiteral (".ADJCLOSE/") % data->tablename %
      CGLiteral (".CLOSE) AS HIGH,") % data->tablename %
      CGLiteral (".LOW*(") % data->tablename %
      CGLiteral (".ADJCLOSE/") % data->tablename %
      CGLiteral (".CLOSE) AS LOW,") % data->tablename %
      CGLiteral (".CLOSE*(") % data->tablename %
      CGLiteral (".ADJCLOSE/") % data->tablename %
      CGLiteral (".CLOSE) AS CLOSE,") % data->tablename %
      CGLiteral (".VOLUME as VOLUME,") % data->tablename %
      CGLiteral (".ADJCLOSE as ADJCLOSE,") % data->tablename %
      CGLiteral (".DATE as DATE,") % data->tablename %
      CGLiteral (".TIME as TIME,") % data->tablename %
      CGLiteral (".MONTH as MONTH,") % data->tablename %
      CGLiteral (".YEAR as YEAR,") % data->tablename %
      CGLiteral (".DAY as DAY,") % data->tablename %
      CGLiteral (".WEEK as WEEK,") % data->tablename %
      CGLiteral (".TICK as TICK from ") % data->tablename %
      CGLiteral (";");
    SQLCommand.append ('\n');

    SQLCommand +=
      CGLiteral ("UPDATE ") %  data->tablename %
      CGLiteral (" SET ADJCLOSE = CLOSE WHERE ") %
      CGLiteral (" (SELECT  MAX (high)/MIN(low) FROM ") % adjustedviewname %
      CGLiteral (") > 5000;");

    SQLCommand.append ('\n');

    // remove symbol from symbols table
    SQLCommand +=
      CGLiteral ("delete from SYMBOLS where KEY='") % adjustedviewname %
      CGLiteral ("';");
    SQLCommand.append ('\n');

    // insert adjusted symbol into symbols table
    SQLCommand += CGLiteral ("insert into SYMBOLS values ('") %
                  data->symbol % CGLiteral ("','") %
                  data->name % CGLiteral ("','") %
                  data->market % CGLiteral ("','") %
                  data->source % CGLiteral ("','") %
                  data->timeframe % CGLiteral ("','") %
                  CGLiteral (" ") % CGLiteral ("','") %
                  CGLiteral (" ") % CGLiteral ("','") %
                  adjustedviewname % CGLiteral ("','") %
                  data->currency % CGLiteral ("',0,'") %
                  data->dnlstring % CGLiteral ("',") %
                  CGLiteral ("'YES', ") %
                  CGLiteral ("'") % data->tablename %
                  CGLiteral ("', '") % data->format %
                  CGLiteral ("', 0, '');");
    SQLCommand.append ('\n');

    // update DATEFROM, DATETO and TFRESOLUTION of adjusted view entry
    SQLCommand +=
      CGLiteral ("update SYMBOLS set DATEFROM = (select min(DATE) from ") %
      adjustedviewname % CGLiteral (") where KEY='") % adjustedviewname %
      CGLiteral ("';");
    SQLCommand.append ('\n');

    SQLCommand +=
      CGLiteral ("update SYMBOLS set DATETO = (select max(DATE) from ") %
      adjustedviewname % CGLiteral (") where KEY='") % adjustedviewname %
      CGLiteral ("';");
    SQLCommand.append ('\n');

    SQLCommand +=
      CGLiteral ("update SYMBOLS set TFRESOLUTION = (select MINUTE_RESOLUTION ");
    SQLCommand +=
      CGLiteral ("from TIMEFRAMES where SYMBOLS.TIMEFRAME = TIMEFRAMES.TIMEFRAME);");
    SQLCommand.append ('\n');

    // create week, month, year views for adjusted data
    SQLCommand += tfview (adjustedviewname, CGLiteral ("WEEK"), data, operation, data->adjust) %
                  tfview (adjustedviewname, CGLiteral ("MONTH"), data, operation, data->adjust) %
                  tfview (adjustedviewname, CGLiteral ("YEAR"), data, operation, data->adjust);
    SQLCommand.append ('\n');
  }

  // insert or update statistics (basedata table)
  SQLCommand +=
    CGLiteral ("delete from basedata where base = '") % data->tablename %
    CGLiteral ("';");
  SQLCommand.append ('\n');
  SQLCommand +=
    CGLiteral ("insert into basedata (base, bv, mc, ebitda, pe, peg, dy, epscurrent, epsnext, es, ps, pbv) ") %
    CGLiteral ("values ('") % data->tablename % CGLiteral ("','") %
    data->BookValue % CGLiteral ("','") % data->MarketCap %
    CGLiteral ("','") % data->EBITDA % CGLiteral ("','") % data->PE %
    CGLiteral ("','") % data->PEG % CGLiteral ("','") %
    data->Yield % CGLiteral ("','") % data->EPScy %  CGLiteral ("','") %
    data->EPSny % CGLiteral ("','") % data->ESh %  CGLiteral ("','") %
    data->PS %  CGLiteral ("','") % data->PBv % CGLiteral ("');");
  SQLCommand.append ('\n');

  // correct forex last update timestamp
  if (data->source == QLatin1String ("FOREX"))
    SQLCommand += CGLiteral ("UPDATE SYMBOLS  SET LASTUPDATE = (SELECT NETFONDSFOREXUPDATE FROM VERSION) WHERE SOURCE = 'FOREX';");
  SQLCommand.append ('\n');

  // just update symbol table's date from and date to columns
  if (operation == QLatin1String ("UPDATE"))
  {
    const QString SQL =
      CGLiteral ("SELECT count(*) AS cnt FROM sqlite_master WHERE type = 'view' AND tbl_name = '") %
      adjustedviewname % CGLiteral ("';");
    int cnt = selectcount (SQL);

    // update DATEFROM, DATETO and TFRESOLUTION of adjusted view entry
    if (data->adjust && cnt > 0)
    {
      SQLCommand +=
        CGLiteral ("UPDATE ") %  data->tablename %
        CGLiteral (" SET ADJCLOSE = CLOSE WHERE ") %
        CGLiteral (" (SELECT  MAX (high)/MIN(low) FROM ") %
        adjustedviewname % CGLiteral (") > 5000;") %
        CGLiteral ("update SYMBOLS set DATEFROM = (select min(DATE) from ") %
        adjustedviewname % CGLiteral (") where KEY='") % adjustedviewname %
        CGLiteral ("';") %
        CGLiteral ("update SYMBOLS set DATETO = (select max(DATE) from ") %
        adjustedviewname % CGLiteral (") where KEY='") % adjustedviewname %
        CGLiteral ("';") %
        CGLiteral ("update SYMBOLS set TFRESOLUTION = (select MINUTE_RESOLUTION ") %
        CGLiteral ("from TIMEFRAMES where SYMBOLS.TIMEFRAME = TIMEFRAMES.TIMEFRAME);") %
        CGLiteral ("update SYMBOLS set CURRENCY = '") % data->currency %
        CGLiteral ("' where KEY='") % adjustedviewname %
        CGLiteral ("';");
    }

    SQLCommand +=
      tfview (data->tablename, CGLiteral ("WEEK"), data, operation, data->adjust) %
      tfview (data->tablename, CGLiteral ("MONTH"), data, operation, data->adjust) %
      tfview (data->tablename, CGLiteral ("YEAR"), data, operation, data->adjust);

    if (data->adjust && cnt > 0)
    {
      SQLCommand +=
        tfview (adjustedviewname, CGLiteral ("WEEK"), data, operation, data->adjust) %
        tfview (adjustedviewname, CGLiteral ("MONTH"), data, operation, data->adjust) %
        tfview (adjustedviewname, CGLiteral ("YEAR"), data, operation, data->adjust) %

        // update timestamp
        CGLiteral ("UPDATE SYMBOLS SET LASTUPDATE = strftime('%s', 'now') ") %
        CGLiteral ("where KEY='") % adjustedviewname %
        CGLiteral ("' AND LASTUPDATE <> strftime('%s', 'now');");
    }

    // update download string
    SQLCommand +=
      CGLiteral ("UPDATE SYMBOLS SET DNLSTRING = '") % data->dnlstring %
      CGLiteral ("' ") % CGLiteral ("WHERE BASE = '") % data->tablename %
      CGLiteral ("';") %

      // update currency
      CGLiteral ("UPDATE SYMBOLS SET CURRENCY = '") % data->currency %
      CGLiteral ("' WHERE BASE = '") % data->tablename % CGLiteral ("';");
    // correct forex last update timestamp
    if (data->source == "FOREX")
      SQLCommand +=
        CGLiteral ("UPDATE SYMBOLS  SET LASTUPDATE = (SELECT NETFONDSFOREXUPDATE FROM VERSION) WHERE SOURCE = 'FOREX'");
  }

  // execute sql
  rc = updatedb (SQLCommand);
  if (rc != SQLITE_OK)
  {
    result = CG_ERR_TRANSACTION;
    setGlobalError(result, __FILE__, __LINE__);
    goto csv2sqlite_end;
  }

csv2sqlite_end:
  // remove temporary file
  QFile::remove(tempfilename);
  return result;
}

