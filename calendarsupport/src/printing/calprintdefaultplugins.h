/*
  Copyright (c) 1998 Preston Brown <pbrown@kde.org>
  Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (C) 2008 Ron Goodheart <rong.dev@gmail.com>
  Copyright (c) 2012-2013 Allen Winter <winter@kde.org>

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
#ifndef CALENDARSUPPORT_CALPRINTDEFAULTPLUGINS_H
#define CALENDARSUPPORT_CALPRINTDEFAULTPLUGINS_H

#include "calendarsupport_export.h"
#include "calprintpluginbase.h"

#include "ui_calprintincidenceconfig_base.h"
#include "ui_calprintdayconfig_base.h"
#include "ui_calprintweekconfig_base.h"
#include "ui_calprintmonthconfig_base.h"
#include "ui_calprinttodoconfig_base.h"

#include <KLocalizedString>

namespace CalendarSupport
{

class CALENDARSUPPORT_EXPORT CalPrintIncidence : public CalPrintPluginBase
{
public:
    CalPrintIncidence();
    virtual ~CalPrintIncidence();
    QString groupName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Print incidence");
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("Print &incidence");
    }
    QString info() const Q_DECL_OVERRIDE
    {
        return i18n("Prints an incidence on one page");
    }
    int sortID() const Q_DECL_OVERRIDE
    {
        return CalPrinterBase::Incidence;
    }

    // Enable the Print Incidence option only if there are selected incidences.
    bool enabled() const Q_DECL_OVERRIDE
    {
        return !mSelectedIncidences.isEmpty();
    }
    QWidget *createConfigWidget(QWidget *) Q_DECL_OVERRIDE;
    QPrinter::Orientation defaultOrientation() const Q_DECL_OVERRIDE
    {
        return QPrinter::Portrait;
    }

public:
    void print(QPainter &p, int width, int height) Q_DECL_OVERRIDE;
    void readSettingsWidget() Q_DECL_OVERRIDE;
    void setSettingsWidget() Q_DECL_OVERRIDE;
    void loadConfig() Q_DECL_OVERRIDE;
    void saveConfig() Q_DECL_OVERRIDE;

protected:
    int printCaptionAndText(QPainter &p, const QRect &box, const QString &caption,
                            const QString &text, const QFont &captionFont, const QFont &textFont);

    bool mShowOptions;
    bool mShowSubitemsNotes;
    bool mShowAttendees;
    bool mShowAttachments;
    bool mShowNoteLines;
};

class CalPrintDay : public CalPrintPluginBase
{
public:
    CalPrintDay();
    virtual ~CalPrintDay();
    QString groupName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Print day");
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("Print da&y");
    }
    QString info() const Q_DECL_OVERRIDE
    {
        return i18n("Prints all events of a single day on one page");
    }
    int sortID() const Q_DECL_OVERRIDE
    {
        return CalPrinterBase::Day;
    }
    bool enabled() const Q_DECL_OVERRIDE
    {
        return true;
    }
    QWidget *createConfigWidget(QWidget *) Q_DECL_OVERRIDE;

public:
    void print(QPainter &p, int width, int height) Q_DECL_OVERRIDE;
    void readSettingsWidget() Q_DECL_OVERRIDE;
    void setSettingsWidget() Q_DECL_OVERRIDE;
    void loadConfig() Q_DECL_OVERRIDE;
    void saveConfig() Q_DECL_OVERRIDE;
    void setDateRange(const QDate &from, const QDate &to) Q_DECL_OVERRIDE;

protected:
    enum eDayPrintType {
        Filofax = 0,
        Timetable,
        SingleTimetable
    } mDayPrintType;
    QTime mStartTime, mEndTime;
    bool mIncludeDescription;
    bool mSingleLineLimit;
    bool mIncludeTodos;
    bool mIncludeAllEvents;
    bool mExcludeTime;
    bool mExcludeConfidential;
    bool mExcludePrivate;
};

class CalPrintWeek : public CalPrintPluginBase
{
public:
    CalPrintWeek();
    virtual ~CalPrintWeek();

    QString groupName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Print week");
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("Print &week");
    }
    QString info() const Q_DECL_OVERRIDE
    {
        return i18n("Prints all events of one week on one page");
    }
    int sortID() const Q_DECL_OVERRIDE
    {
        return CalPrinterBase::Week;
    }
    bool enabled() const Q_DECL_OVERRIDE
    {
        return true;
    }
    QWidget *createConfigWidget(QWidget *) Q_DECL_OVERRIDE;

    /**
      Returns the default orientation for the eWeekPrintType.
    */
    QPrinter::Orientation defaultOrientation() const Q_DECL_OVERRIDE;

public:
    void print(QPainter &p, int width, int height) Q_DECL_OVERRIDE;
    void readSettingsWidget() Q_DECL_OVERRIDE;
    void setSettingsWidget() Q_DECL_OVERRIDE;
    void loadConfig() Q_DECL_OVERRIDE;
    void saveConfig() Q_DECL_OVERRIDE;
    void setDateRange(const QDate &from, const QDate &to) Q_DECL_OVERRIDE;

protected:
    enum eWeekPrintType {
        Filofax = 0,
        Timetable,
        SplitWeek
    } mWeekPrintType;
    QTime mStartTime, mEndTime;
    bool mSingleLineLimit;
    bool mIncludeTodos;
    bool mIncludeDescription;
    bool mExcludeTime;
    bool mExcludeConfidential;
    bool mExcludePrivate;
};

class CalPrintMonth : public CalPrintPluginBase
{
public:
    CalPrintMonth();
    virtual ~CalPrintMonth();
    QString groupName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Print month");
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("Print mont&h");
    }
    QString info() const Q_DECL_OVERRIDE
    {
        return i18n("Prints all events of one month on one page");
    }
    int sortID() const Q_DECL_OVERRIDE
    {
        return CalPrinterBase::Month;
    }
    bool enabled() const Q_DECL_OVERRIDE
    {
        return true;
    }
    QWidget *createConfigWidget(QWidget *) Q_DECL_OVERRIDE;
    QPrinter::Orientation defaultOrientation() const Q_DECL_OVERRIDE
    {
        return QPrinter::Landscape;
    }

public:
    void print(QPainter &p, int width, int height) Q_DECL_OVERRIDE;
    void readSettingsWidget() Q_DECL_OVERRIDE;
    void setSettingsWidget() Q_DECL_OVERRIDE;
    void loadConfig() Q_DECL_OVERRIDE;
    void saveConfig() Q_DECL_OVERRIDE;
    void setDateRange(const QDate &from, const QDate &to) Q_DECL_OVERRIDE;

protected:
    bool mWeekNumbers;
    bool mRecurDaily;
    bool mRecurWeekly;
    bool mIncludeTodos;
    bool mSingleLineLimit;
    bool mIncludeDescription;
    bool mExcludeConfidential;
    bool mExcludePrivate;
};

class CalPrintTodos : public CalPrintPluginBase
{
public:
    CalPrintTodos();
    virtual ~CalPrintTodos();

    QString groupName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Print to-dos");
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("Print to-&dos");
    }
    QString info() const Q_DECL_OVERRIDE
    {
        return i18n("Prints all to-dos in a (tree-like) list");
    }
    int sortID() const Q_DECL_OVERRIDE
    {
        return CalPrinterBase::Todolist;
    }
    bool enabled() const Q_DECL_OVERRIDE
    {
        return true;
    }
    QWidget *createConfigWidget(QWidget *) Q_DECL_OVERRIDE;

public:
    void print(QPainter &p, int width, int height) Q_DECL_OVERRIDE;
    void readSettingsWidget() Q_DECL_OVERRIDE;
    void setSettingsWidget() Q_DECL_OVERRIDE;
    void loadConfig() Q_DECL_OVERRIDE;
    void saveConfig() Q_DECL_OVERRIDE;

protected:
    QString mPageTitle;

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
    bool mExcludeConfidential;
    bool mExcludePrivate;
};

class CalPrintIncidenceConfig : public QWidget, public Ui::CalPrintIncidenceConfig_Base
{
public:
    explicit CalPrintIncidenceConfig(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

class CalPrintDayConfig : public QWidget, public Ui::CalPrintDayConfig_Base
{
public:
    explicit CalPrintDayConfig(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

class CalPrintWeekConfig : public QWidget, public Ui::CalPrintWeekConfig_Base
{
public:
    explicit CalPrintWeekConfig(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

class CalPrintMonthConfig : public QWidget, public Ui::CalPrintMonthConfig_Base
{
public:
    explicit CalPrintMonthConfig(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

class CalPrintTodoConfig : public QWidget, public Ui::CalPrintTodoConfig_Base
{
public:
    explicit CalPrintTodoConfig(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

}

#endif
