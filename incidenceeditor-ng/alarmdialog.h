/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef ALARMDIALOG_H
#define ALARMDIALOG_H

#include <kcalcore/alarm.h>

#include <KDialog>

namespace Ui {
class AlarmDialog;
}

namespace IncidenceEditorsNG {

class AlarmDialog : public KDialog
{
  public:
    enum Unit {
      Minutes,
      Hours,
      Days
    };

    enum When {
      BeforeStart = 0,
      AfterStart,
      BeforeEnd,
      AfterEnd
    };

  public:
    AlarmDialog();

    void load( const KCalCore::Alarm::Ptr &alarm );
    void save( const KCalCore::Alarm::Ptr &alarm ) const;
    void setAllowEndReminders( bool allowEndReminders );
    void setIsTodoReminder( bool isTodo );
    void setOffset( int offset );
    void setUnit( Unit unit );
    void setWhen( When when );

  private:
    Ui::AlarmDialog *mUi;
    bool mAllowEndReminders;
};

}

#endif // ALARMDIALOG_H
