/*
  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/
#ifndef CALENDARSUPPORT_PRINTING_CALPRINTER_H
#define CALENDARSUPPORT_PRINTING_CALPRINTER_H

#include "../calendarsupport_export.h"
#include "printplugin.h"

#include <KComboBox>
#include <KDialog>

class QButtonGroup;
class QStackedWidget;

namespace CalendarSupport {

/**
  CalPrinter is a class for printing Calendars.  It can print in several
  different formats (day, week, month).  It also provides a way for setting
  up the printer and remembering these preferences.
*/
class CALENDARSUPPORT_EXPORT CalPrinter : public QObject, public CalPrinterBase
{
  Q_OBJECT
  public:
    enum ePrintOrientation {
      eOrientPlugin=0,
      eOrientPrinter,
      eOrientPortrait,
      eOrientLandscape
    };

  public:
    /**
      \param par parent widget for dialogs
      \param cal calendar to be printed
      \param uniqItem if true, indicates the calendar print dialog will only
      provide the option to print an single incidence; else, all possible types
      of print types will be shown
    */
    CalPrinter( QWidget *par, const Akonadi::ETMCalendar::Ptr &calendar,
                bool uniqItem = false );

    virtual ~CalPrinter();

    void init( const Akonadi::ETMCalendar::Ptr &calendar );

    /**
      Set date range to be printed.

      \param start Start date
      \param end   End date
    */
    void setDateRange( const QDate &start, const QDate &end );

  public Q_SLOTS:
    void updateConfig();

  private Q_SLOTS:
    void doPrint( PrintPlugin *selectedStyle,
                  ePrintOrientation dlgorientation, bool preview = false );

  public:
    void print( int type, const QDate &fd, const QDate &td,
                KCalCore::Incidence::List selectedIncidences = KCalCore::Incidence::List(),
                bool preview = false );
    Akonadi::ETMCalendar::Ptr calendar() const;
    KConfig *config() const;

  protected:
    PrintPlugin::List mPrintPlugins;

  private:
    Akonadi::ETMCalendar::Ptr mCalendar;
    QWidget *mParent;
    KConfig *mConfig;
    bool mUniqItem;
};

class CalPrintDialog : public KDialog
{
  Q_OBJECT
  public:
    explicit CalPrintDialog( int initialPrintType, PrintPlugin::List plugins,
                             QWidget *parent = 0, bool mUniqItem = false );

    virtual ~CalPrintDialog();

    PrintPlugin *selectedPlugin();
    void setOrientation( CalPrinter::ePrintOrientation orientation );
    CalPrinter::ePrintOrientation orientation()
    {
      return mOrientation;
    }

  public Q_SLOTS:
    void setPrintType( int );
    void setPreview( bool );

  protected Q_SLOTS:
    void slotOk();

  private:
    QButtonGroup *mTypeGroup;
    QStackedWidget *mConfigArea;
    QMap<int, PrintPlugin*> mPluginIDs;
    QString mPreviewText;
    KComboBox *mOrientationSelection;

    CalPrinter::ePrintOrientation mOrientation;
};

}

#endif
