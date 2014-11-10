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

#ifndef INCIDENCEEDITOR_ALARMDIALOG_H
#define INCIDENCEEDITOR_ALARMDIALOG_H

#include "incidenceeditors_ng_export.h"

#include <KCalCore/Incidence>

#include <QDialog>

namespace Ui
{
class AlarmDialog;
}

namespace IncidenceEditorNG
{

class  INCIDENCEEDITORS_NG_EXPORT AlarmDialog : public QDialog
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
    /**
      Constructs a new alarm dialog.
      @p incidenceType will influence i18n strings, that will be different for to-dos.
     */
    explicit AlarmDialog(KCalCore::Incidence::IncidenceType incidenceType, QWidget *parent = 0);
    ~AlarmDialog();
    void load(const KCalCore::Alarm::Ptr &alarm);
    void save(const KCalCore::Alarm::Ptr &alarm) const;
    void setAllowBeginReminders(bool allow);
    void setAllowEndReminders(bool allow);
    void setOffset(int offset);
    void setUnit(Unit unit);
    void setWhen(When when);

private:
    void fillCombo();

private:
    Ui::AlarmDialog *mUi;
    KCalCore::Incidence::IncidenceType mIncidenceType;
    bool mAllowBeginReminders;
    bool mAllowEndReminders;
};

}

#endif // INCIDENCEEDITOR_ALARMDIALOG_H
