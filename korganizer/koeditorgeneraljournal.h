/*
    This file is part of KOrganizer.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KOEDITORGENERALJOURNAL_H
#define KOEDITORGENERALJOURNAL_H

#include "koeditorgeneral.h"

#include <tqobject.h>
#include <tqdatetime.h>

class KDateEdit;
class KTimeEdit;
class KTextEdit;
class TQLineEdit;
class TQLabel;
class TQBoxLayout;
class TQCheckBox;
class TQWidget;

namespace KCal {
class Incidence;
class Journal;
}
using namespace KCal;

class KOEditorGeneralJournal : public KOEditorGeneral
{
  Q_OBJECT
  public:
    KOEditorGeneralJournal ( TQWidget *parent=0, const char* name=0 );
    virtual ~KOEditorGeneralJournal();

    void initDate( TQWidget *, TQBoxLayout * );
    void initDescription( TQWidget *, TQBoxLayout * );
    void initTitle( TQWidget *parent, TQBoxLayout *topLayout );

    /** Set widgets to default values */
    void setDefaults( const TQDate &date );
    void setDate( const TQDate &date );
    void setTime( const TQTime &time );
    /** Read journal object and setup widgets accordingly */
    void readJournal( Journal *, const TQDate &, bool tmpl = false );
    /** Write journal settings to event object */
    void writeJournal( Journal * );

    /** Check if the input is valid. */
    bool validateInput();

    void setDescription( const TQString &text );
    void setSummary( const TQString &text );
    void finishSetup();

  protected:
    TQLineEdit  *mSummaryEdit;
    TQLabel     *mSummaryLabel;
    KTextEdit  *mDescriptionEdit;
    TQLabel     *mDateLabel;
    KDateEdit  *mDateEdit;
    TQCheckBox  *mTimeCheckBox;
    KTimeEdit  *mTimeEdit;
};

#endif
