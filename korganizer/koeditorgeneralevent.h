/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef _KOEDITORGENERALEVENT_H
#define _KOEDITORGENERALEVENT_H

#include "koeditorgeneral.h"
#include <tqdatetime.h>

class TQLabel;
class KDateEdit;
class KTimeEdit;
class TQCheckBox;
class TQComboBox;
class TQBoxLayout;

namespace KCal {
class Event;
}
using namespace KCal;

class KOEditorGeneralEvent : public KOEditorGeneral
{
    Q_OBJECT
  public:
    KOEditorGeneralEvent (TQObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneralEvent();

    void initTime(TQWidget *,TQBoxLayout *);
    void initClass(TQWidget *,TQBoxLayout *);
    void initInvitationBar( TQWidget* parent, TQBoxLayout *layout );

    void finishSetup();

    /** Set widgets to default values */
    void setDefaults( const TQDateTime &from, const TQDateTime &to, bool allDay );
    /**
      Read event object and setup widgets accordingly. If templ is true, the
      event is read as template, i.e. the time and date information isn't set.
    */
    void readEvent( Event *event, Calendar *calendar, const TQDate &date, bool tmpl = false );
    /** Write event settings to event object */
    void writeEvent( Event * );

    /** Check if the input is valid. */
    bool validateInput();

    void updateRecurrenceSummary( Event *event );

    TQFrame* invitationBar() const { return mInvitationBar; }

  public slots:
    void setDateTimes( const TQDateTime &start, const TQDateTime &end );
    void setDuration();

  protected slots:
    void timeStuffDisable( bool disable );
    void associateTime( bool time );

    void startTimeChanged( TQTime );
    void startDateChanged( const TQDate& );
    void endTimeChanged( TQTime );
    void endDateChanged( const TQDate& );

    void emitDateTimeStr();

  signals:
    void allDayChanged(bool);
    void dateTimeStrChanged( const TQString & );
    void dateTimesChanged( const TQDateTime &start, const TQDateTime &end );
    void editRecurrence();
    void acceptInvitation();
    void declineInvitation();

  private:
    TQLabel                  *mStartDateLabel;
    TQLabel                  *mEndDateLabel;
    KDateEdit               *mStartDateEdit;
    KDateEdit               *mEndDateEdit;
    KTimeEdit               *mStartTimeEdit;
    KTimeEdit               *mEndTimeEdit;
    TQLabel                  *mDurationLabel;
    TQCheckBox               *mAlldayEventCheckbox;
    TQComboBox               *mFreeTimeCombo;
    TQFrame                  *mInvitationBar;

    // current start and end date and time
    TQDateTime mCurrStartDateTime;
    TQDateTime mCurrEndDateTime;
};

#endif
