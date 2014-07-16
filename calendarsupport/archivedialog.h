/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef CALENDARSUPPORT_ARCHIVEDIALOG_H
#define CALENDARSUPPORT_ARCHIVEDIALOG_H

#include "calendarsupport_export.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <KDialog>

class KComboBox;
class KDateComboBox;
class KIntNumInput;
class KUrlRequester;

class QCheckBox;
class QRadioButton;

namespace Akonadi {
  class IncidenceChanger;
  class ETMCalendar;
}

namespace CalendarSupport {

class CALENDARSUPPORT_EXPORT ArchiveDialog : public KDialog
{
  Q_OBJECT
  public:
    ArchiveDialog( const Akonadi::ETMCalendar::Ptr &calendar,
                   Akonadi::IncidenceChanger *changer,
                   QWidget *parent = 0 );
    ~ArchiveDialog();

  signals:
    // connected by KODialogManager to CalendarView
    void eventsDeleted();
    void autoArchivingSettingsModified();

  protected slots:
    void slotEventsDeleted();
    void slotUser1();
    void slotEnableUser1();
    void slotActionChanged();

  private slots:
    void showWhatsThis();

  private:
    KUrlRequester *mArchiveFile;
    KDateComboBox *mDateEdit;
    QCheckBox *mDeleteCb;
    QRadioButton *mArchiveOnceRB;
    QRadioButton *mAutoArchiveRB;
    KIntNumInput *mExpiryTimeNumInput;
    KComboBox *mExpiryUnitsComboBox;
    QCheckBox *mEvents;
    QCheckBox *mTodos;
    Akonadi::IncidenceChanger *mChanger;
    Akonadi::ETMCalendar::Ptr mCalendar;
};

}

#endif
