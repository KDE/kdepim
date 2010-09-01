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
#ifndef JOURNALENTRY_H
#define JOURNALENTRY_H
//
// Widget showing one Journal entry

#include <libkcal/resourcecalendar.h>
#include <tqvbox.h>

class TQLabel;
class KActiveLabel;
class TQCheckBox;
class TQGridLayout;
class KLineEdit;
class KTextEdit;
class KTimeEdit;
class TQButton;
namespace KOrg {
class IncidenceChangerBase;
}
using namespace KOrg;
namespace KCal {
  class Calendar;
  class Journal;
}
using namespace KCal;

class JournalEntry : public TQWidget {
    Q_OBJECT
  public:
    typedef ListBase<JournalEntry> List;

    JournalEntry( Journal* j, TQWidget *parent );
    virtual ~JournalEntry();

    void setJournal(Journal *);
    Journal *journal() const { return mJournal; }

    TQDate date() const { return mDate; }

    void clear();
    void readJournal( Journal *j );

    bool isReadOnly() const { return mReadOnly; }
    void setReadOnly( bool readonly );

  protected slots:
    void setDirty();
    void deleteItem();
    void editItem();
    void printItem();
    void timeCheckBoxToggled(bool on);
  public slots:
    void setIncidenceChanger( IncidenceChangerBase *changer ) { mChanger = changer; }
    void setDate(const TQDate &);
    void flushEntry();

  signals:
    void deleteIncidence( Incidence * );
    void editIncidence( Incidence *, const TQDate& );

  protected:
    void clearFields();
    bool eventFilter( TQObject *o, TQEvent *e );

    void writeJournal();

  private:
    void writeJournalPrivate( Journal *j );

    Journal *mJournal;
    TQDate mDate;
    bool mReadOnly;

    TQLabel *mTitleLabel;
    KLineEdit *mTitleEdit;
    KTextEdit *mEditor;
    TQCheckBox *mTimeCheck;
    KTimeEdit *mTimeEdit;
    TQButton *mDeleteButton;
    TQButton *mEditButton;
    TQButton *mPrintButton;

    TQGridLayout *mLayout;

    bool mDirty;
    bool mWriteInProgress;
    IncidenceChangerBase *mChanger;
};


class JournalDateEntry : public TQVBox {
    Q_OBJECT
  public:
    typedef ListBase<JournalDateEntry> List;

    JournalDateEntry( Calendar *, TQWidget *parent );
    virtual ~JournalDateEntry();

    void addJournal( Journal * );
    Journal::List journals() const;

    void setDate( const TQDate & );
    TQDate date() const { return mDate; }

    void clear();


  signals:
    void setIncidenceChangerSignal( IncidenceChangerBase *changer );
    void setDateSignal( const TQDate & );
    void flushEntries();
    void editIncidence( Incidence *, const TQDate& );
    void deleteIncidence( Incidence * );
    void newJournal( ResourceCalendar *, const TQString &, const TQDate & );

  public slots:
    void emitNewJournal();
    void setIncidenceChanger( IncidenceChangerBase *changer );
    void journalEdited( Journal* );
    void journalDeleted( Journal* );

  private:
    Calendar *mCalendar;
    TQDate mDate;
    TQMap<Journal*,JournalEntry*> mEntries;

    KActiveLabel *mTitle;
    TQWidget *mAddBar;
    IncidenceChangerBase *mChanger;
};


#endif
