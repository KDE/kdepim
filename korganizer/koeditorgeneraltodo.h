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
#ifndef _KOEDITORGENERALTODO_H
#define _KOEDITORGENERALTODO_H

#include "koeditorgeneral.h"
#include "koglobals.h"

#include <tqdatetime.h>

class KRestrictedLine;

class KDateEdit;
class KTimeEdit;

namespace KCal {
class Todo;
}
using namespace KCal;

class KOEditorGeneralTodo : public KOEditorGeneral
{
    Q_OBJECT
  public:
    KOEditorGeneralTodo (TQObject* parent=0,const char* name=0);
    virtual ~KOEditorGeneralTodo();

    void initTime(TQWidget *, TQBoxLayout *);
    void initStatus(TQWidget *, TQBoxLayout *);
    void initCompletion(TQWidget *, TQBoxLayout *);
    void initPriority(TQWidget *, TQBoxLayout *);

    void finishSetup();

    /** Set widgets to default values */
    void setDefaults( const TQDateTime &due, bool allDay );
    /** Read todo object and setup widgets accordingly */
    void readTodo( Todo *todo, Calendar *calendar, const TQDate &date );
    /** Write todo settings to event object */
    void writeTodo(Todo *);

    /** Check if the input is valid. */
    bool validateInput();

    void updateRecurrenceSummary( Todo *todo );

    /** The todo has been modified externally */
    void modified( Todo *todo, KOGlobals::HowChanged modification );

  signals:
    void dueDateEditToggle( bool );
    void dateTimeStrChanged( const TQString & );
    void signalDateTimeChanged( const TQDateTime &, const TQDateTime & );
    void editRecurrence();

  protected slots:
    void completedChanged( int );
    void completedChanged();
    void dateChanged();
    void startDateModified();

    void enableDueEdit( bool enable );
    void enableStartEdit( bool enable );
    void enableTimeEdits( bool enable );

  protected:
    void setCompletedDate();

 private:
    bool                    mAlreadyComplete;
    bool                    mStartDateModified;

    KDateEdit               *mStartDateEdit;
    KTimeEdit               *mStartTimeEdit;
    TQCheckBox               *mTimeButton;
    TQCheckBox               *mDueCheck;
    KDateEdit               *mDueDateEdit;
    KTimeEdit               *mDueTimeEdit;
    TQCheckBox               *mCompletedToggle;
    TQComboBox               *mCompletedCombo;
    TQLabel                  *mCompletedLabel;
    TQLabel                  *mPriorityLabel;
    TQComboBox               *mPriorityCombo;

    KDateEdit               *mCompletionDateEdit;
    KTimeEdit               *mCompletionTimeEdit;

    TQCheckBox               *mStartCheck;

    TQDateTime               mCompletedDateTime;
};


#endif
