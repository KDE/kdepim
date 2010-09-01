/*
    This file is part of KOrganizer.
    Copyright (c) 2000-2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef _KOEDITORRECURRENCE_H
#define _KOEDITORRECURRENCE_H

#include <tqdatetime.h>
#include <tqwidget.h>
#include <tqbitarray.h>

#include <kdialogbase.h>

#include <libkcal/incidencebase.h>

class TQWidgetStack;
class TQSpinBox;
class TQRadioButton;
class TQGroupBox;
class TQCheckBox;

class KDateEdit;
namespace KCal {
class Incidence;
}
using namespace KCal;

class RecurBase : public QWidget
{
  public:
    RecurBase( TQWidget *parent = 0, const char *name = 0 );

    void setFrequency( int );
    int frequency();
    // FIXME: If we want to adjust the recurrence when the start/due date change,
    // we need to reimplement this method in the derived classes!
    void setDateTimes( const TQDateTime &/*start*/, const TQDateTime &/*end*/ ) {}

    TQWidget *frequencyEdit();

  protected:
    static TQComboBox *createWeekCountCombo( TQWidget *parent=0, const char *name=0 );
    static TQComboBox *createWeekdayCombo( TQWidget *parent=0, const char *name=0 );
    static TQComboBox *createMonthNameCombo( TQWidget *parent=0, const char *name=0 );
    TQBoxLayout *createFrequencySpinBar( TQWidget *parent, TQLayout *layout,
    TQString everyText, TQString unitText );

  private:
    TQSpinBox *mFrequencyEdit;
};

class RecurDaily : public RecurBase
{
  public:
    RecurDaily( TQWidget *parent = 0, const char *name = 0 );
};

class RecurWeekly : public RecurBase
{
  public:
    RecurWeekly( TQWidget *parent = 0, const char *name = 0 );

    void setDays( const TQBitArray & );
    TQBitArray days();

  private:
    TQCheckBox *mDayBoxes[7];
};

class RecurMonthly : public RecurBase
{
  public:
    RecurMonthly( TQWidget *parent = 0, const char *name = 0 );

    void setByDay( int day );
    void setByPos( int count, int weekday );

    bool byDay();
    bool byPos();

    int day();

    int count();
    int weekday();

  private:
    TQRadioButton *mByDayRadio;
    TQComboBox *mByDayCombo;

    TQRadioButton *mByPosRadio;
    TQComboBox *mByPosCountCombo;
    TQComboBox *mByPosWeekdayCombo;
};

class RecurYearly : public RecurBase
{
  public:
    enum YearlyType { byDay, byPos, byMonth };

    RecurYearly( TQWidget *parent = 0, const char *name = 0 );

    void setByDay( int day );
    void setByPos( int count, int weekday, int month );
    void setByMonth( int day, int month );

    YearlyType getType();

    int day();
    int posCount();
    int posWeekday();
    int posMonth();
    int monthDay();
    int month();

  private:
    TQRadioButton *mByMonthRadio;
    TQRadioButton *mByPosRadio;
    TQRadioButton *mByDayRadio;

    TQSpinBox *mByMonthSpin;
    TQComboBox *mByMonthCombo;

    TQComboBox *mByPosDayCombo;
    TQComboBox *mByPosWeekdayCombo;
    TQComboBox *mByPosMonthCombo;

    TQSpinBox *mByDaySpin;
};

class RecurrenceChooser : public QWidget
{
    Q_OBJECT
  public:
    RecurrenceChooser( TQWidget *parent = 0, const char *name = 0 );

    enum { Daily, Weekly, Monthly, Yearly };

    void setType( int );
    int type();

  signals:
    void chosen( int );

  protected slots:
    void emitChoice();

  private:
    TQComboBox *mTypeCombo;

    TQRadioButton *mDailyButton;
    TQRadioButton *mWeeklyButton;
    TQRadioButton *mMonthlyButton;
    TQRadioButton *mYearlyButton;
};

class ExceptionsBase
{
  public:
    virtual void setDates( const DateList & ) = 0;
    virtual DateList dates() = 0;
};

class ExceptionsWidget : public TQWidget, public ExceptionsBase
{
    Q_OBJECT
  public:
    ExceptionsWidget( TQWidget *parent = 0, const char *name = 0 );

    void setDates( const DateList & );
    DateList dates();

  protected slots:
    void addException();
    void changeException();
    void deleteException();

  private:
    KDateEdit *mExceptionDateEdit;
    TQListBox *mExceptionList;
    DateList mExceptionDates;
};

class ExceptionsDialog : public KDialogBase, public ExceptionsBase
{
  public:
    ExceptionsDialog( TQWidget *parent, const char *name = 0 );

    void setDates( const DateList & );
    DateList dates();

  private:
    ExceptionsWidget *mExceptions;
};

class RecurrenceRangeBase
{
  public:
    virtual void setDefaults( const TQDateTime &from ) = 0;

    virtual void setDuration( int ) = 0;
    virtual int duration() = 0;

    virtual void setEndDate( const TQDate & ) = 0;
    virtual TQDate endDate() = 0;

    virtual void setDateTimes( const TQDateTime &start,
                               const TQDateTime &end = TQDateTime() ) = 0;
};

class RecurrenceRangeWidget : public TQWidget, public RecurrenceRangeBase
{
    Q_OBJECT
  public:
    RecurrenceRangeWidget( TQWidget *parent = 0, const char *name = 0 );

    void setDefaults( const TQDateTime &from );

    void setDuration( int );
    int duration();

    void setEndDate( const TQDate & );
    TQDate endDate();

    void setDateTimes( const TQDateTime &start,
                       const TQDateTime &end = TQDateTime() );

  protected slots:
    void showCurrentRange();

  private:
    TQGroupBox *mRangeGroupBox;
    TQLabel *mStartDateLabel;
    TQRadioButton *mNoEndDateButton;
    TQRadioButton *mEndDurationButton;
    TQSpinBox *mEndDurationEdit;
    TQRadioButton *mEndDateButton;
    KDateEdit *mEndDateEdit;
};

class RecurrenceRangeDialog : public KDialogBase, public RecurrenceRangeBase
{
  public:
    RecurrenceRangeDialog( TQWidget *parent = 0, const char *name = 0 );

    void setDefaults( const TQDateTime &from );

    void setDuration( int );
    int duration();

    void setEndDate( const TQDate & );
    TQDate endDate();

    void setDateTimes( const TQDateTime &start,
                       const TQDateTime &end = TQDateTime() );

  private:
    RecurrenceRangeWidget *mRecurrenceRangeWidget;
};

class KOEditorRecurrence : public QWidget
{
    Q_OBJECT
  public:
    KOEditorRecurrence ( TQWidget *parent = 0, const char *name = 0 );
    virtual ~KOEditorRecurrence();

    enum { Daily, Weekly, Monthly, Yearly };

    /** Set widgets to default values */
    void setDefaults( const TQDateTime &from, const TQDateTime &to, bool allday );
    /** Read event object and setup widgets accordingly */
    void readIncidence( Incidence * );
    /** Write event settings to event object */
    void writeIncidence( Incidence * );

    /** Check if the input is valid. */
    bool validateInput();

    bool doesRecur();

    void saveValues();
    void restoreValues();

  public slots:
    void setRecurrenceEnabled( bool );
    void setDateTimes( const TQDateTime &start, const TQDateTime &end );
    void setDateTimeStr( const TQString & );

  signals:
    void dateTimesChanged( const TQDateTime &start, const TQDateTime &end );

  protected slots:
    void showCurrentRule( int );
    void showExceptionsDialog();
    void showRecurrenceRangeDialog();

  private:
    Recurrence mSaveRec;
    TQCheckBox *mEnabledCheck;

    TQGroupBox *mTimeGroupBox;
    TQLabel *mDateTimeLabel;

    TQGroupBox *mRuleBox;
    TQWidgetStack *mRuleStack;
    RecurrenceChooser *mRecurrenceChooser;

    RecurDaily *mDaily;
    RecurWeekly *mWeekly;
    RecurMonthly *mMonthly;
    RecurYearly *mYearly;

    RecurrenceRangeBase *mRecurrenceRange;
    RecurrenceRangeWidget *mRecurrenceRangeWidget;
    RecurrenceRangeDialog *mRecurrenceRangeDialog;
    TQPushButton *mRecurrenceRangeButton;

    ExceptionsBase *mExceptions;
    ExceptionsDialog *mExceptionsDialog;
    ExceptionsWidget *mExceptionsWidget;
    TQPushButton *mExceptionsButton;

    TQDateTime mEventStartDt;
};

class KOEditorRecurrenceDialog : public KDialogBase
{
  Q_OBJECT
  public:
    KOEditorRecurrenceDialog( TQWidget *parent );
    KOEditorRecurrence* editor() const { return mRecurrence; }

  protected slots:
    void slotOk();
    void slotCancel();

  private:
    KOEditorRecurrence *mRecurrence;
    bool mRecurEnabled;
};

#endif
