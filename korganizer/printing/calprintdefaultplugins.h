/*
    This file is part of KOrganizer.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef CALPRINTDEFAULTPLUGINS_H
#define CALPRINTDEFAULTPLUGINS_H


#include <klocale.h>
#include "calprintpluginbase.h"

#ifndef KORG_NOPRINTER
namespace KCal {
class Calendar;
}

using namespace KCal;
using namespace KOrg;

class CalPrintIncidence : public CalPrintPluginBase
{
  public:
    CalPrintIncidence();
    virtual ~CalPrintIncidence();
    virtual TQString description()
    {
      return i18n( "Print &incidence" );
    }
    virtual TQString info()
    {
      return i18n( "Prints an incidence on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Incidence;
    }

  // Enable the Print Incidence option only if there are selected incidences.
    virtual bool enabled()
      {
        if ( mSelectedIncidences.count() > 0 ) {
          return true;
        } else {
          return false;
        }
      }
    virtual TQWidget *createConfigWidget( TQWidget * );
    virtual KPrinter::Orientation defaultOrientation()
    {
      return KPrinter::Portrait;
    }

  public:
    void print( TQPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
  protected:
    int printCaptionAndText( TQPainter &p, const TQRect &box, const TQString &caption, 
                             const TQString &text, TQFont captionFont, TQFont textFont );
  

  protected:
    bool mShowOptions;
    bool mShowSubitemsNotes;
    bool mShowAttendees;
    bool mShowAttachments;
};


class CalPrintDay : public CalPrintPluginBase
{
  public:
    CalPrintDay();
    virtual ~CalPrintDay();
    virtual TQString description()
    {
      return i18n( "Print da&y" );
    }
    virtual TQString info()
    {
      return i18n( "Prints all events of a single day on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Day;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual TQWidget *createConfigWidget( TQWidget* );

  public:
    void print(TQPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const TQDate& from, const TQDate& to );

  protected:
    TQTime mStartTime, mEndTime;
    bool mIncludeTodos;
    bool mIncludeAllEvents;
};

class CalPrintWeek : public CalPrintPluginBase
{
  public:
    CalPrintWeek();
    virtual ~CalPrintWeek();
    virtual TQString description()
    {
      return i18n( "Print &week" );
    }
    virtual TQString info()
    {
      return i18n( "Prints all events of one week on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Week;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual TQWidget *createConfigWidget( TQWidget * );

    /**
      Returns the default orientation for the eWeekPrintType.
    */
    virtual KPrinter::Orientation defaultOrientation();

  public:
    void print(TQPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const TQDate& from, const TQDate& to );

  protected:
    enum eWeekPrintType { Filofax=0, Timetable, SplitWeek } mWeekPrintType;
    TQTime mStartTime, mEndTime;
    bool mIncludeTodos;
};

class CalPrintMonth : public CalPrintPluginBase
{
  public:
    CalPrintMonth();
    virtual ~CalPrintMonth();
    virtual TQString description()
    {
      return i18n( "Print mont&h" );
    }
    virtual TQString info()
    {
      return i18n( "Prints all events of one month on one page" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Month;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual TQWidget *createConfigWidget( TQWidget * );
    virtual KPrinter::Orientation defaultOrientation()
    {
      return KPrinter::Landscape;
    }


  public:
    void print(TQPainter &p, int width, int height);
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();
    virtual void setDateRange( const TQDate& from, const TQDate& to );

  protected:
    bool mWeekNumbers;
    bool mRecurDaily;
    bool mRecurWeekly;
    bool mIncludeTodos;
};

class CalPrintTodos : public CalPrintPluginBase
{
  public:
    CalPrintTodos();
    virtual ~CalPrintTodos();
    virtual TQString description()
    {
      return i18n( "Print to-&dos" );
    }
    virtual TQString info()
    {
      return i18n( "Prints all to-dos in a (tree-like) list" );
    }
    virtual int sortID()
    {
      return CalPrinterBase::Todolist;
    }
    virtual bool enabled()
    {
      return true;
    }
    virtual TQWidget *createConfigWidget( TQWidget * );

  public:
    void print( TQPainter &p, int width, int height );
    virtual void readSettingsWidget();
    virtual void setSettingsWidget();
    virtual void loadConfig();
    virtual void saveConfig();

  protected:
    TQString mPageTitle;

    enum eTodoPrintType {
      TodosAll = 0,
      TodosUnfinished,
      TodosDueRange
    } mTodoPrintType;

    enum eTodoSortField {
      TodoFieldSummary = 0,
      TodoFieldStartDate,
      TodoFieldDueDate,
      TodoFieldPriority,
      TodoFieldPercentComplete,
      TodoFieldUnset
    } mTodoSortField;

    enum eTodoSortDirection {
      TodoDirectionAscending = 0,
      TodoDirectionDescending,
      TodoDirectionUnset
    } mTodoSortDirection;

    bool mIncludeDescription;
    bool mIncludePriority;
    bool mIncludeDueDate;
    bool mIncludePercentComplete;
    bool mConnectSubTodos;
    bool mStrikeOutCompleted;
    bool mSortField;
    bool mSortDirection;
};


#endif

#endif
