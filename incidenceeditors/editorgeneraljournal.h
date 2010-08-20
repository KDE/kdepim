/*
  This file is part of KOrganizer.

  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2007 Mike McQuaid <mike@mikemcquaid.com>

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
#ifndef EDITORGENERALJOURNAL_H
#define EDITORGENERALJOURNAL_H

#include <kcalcore/journal.h>

#include "editorgeneral.h"


namespace KPIM {
  class KDateEdit;
  class KTimeEdit;
}

class EditorGeneralJournal : public EditorGeneral
{
  Q_OBJECT
  public:
    explicit EditorGeneralJournal ( QObject *parent=0 );
    virtual ~EditorGeneralJournal();

    void initDate( QWidget *, QBoxLayout * );
    void initCategories( QWidget *, QBoxLayout * );
    void initTitle( QWidget *parent, QBoxLayout *topLayout );

    /** Set date widget to default values */
    void setDate( const QDate &date );
    /** Set time widget to default values */
    void setTime( const QTime &time );
    /** Read journal object and setup widgets accordingly */
    void readJournal( const KCalCore::Journal::Ptr &, const QDate &date, bool tmpl = false );
    /** Write journal settings to event object */
    void fillJournal( KCalCore::Journal::Ptr & );

    /** Check if the input is valid. */
    bool validateInput();

    void setSummary( const QString &text );
    void finishSetup();

  protected:
    virtual bool setAlarmOffset( const KCalCore::Alarm::Ptr &alarm, int value ) const;

  private:
    QWidget *mParent;
    FocusLineEdit *mTitleEdit;
    QLabel *mTitleLabel;
    QLabel *mDateLabel;
    KPIM::KDateEdit *mDateEdit;
    QCheckBox *mTimeCheckBox;
    KPIM::KTimeEdit *mTimeEdit;

};

#endif
