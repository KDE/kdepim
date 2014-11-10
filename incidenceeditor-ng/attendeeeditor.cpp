/*
  Copyright (C) 2010 Casey Link <unnamedrambler@gmail.com>
  Copyright (C) 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "attendeeeditor.h"

#include <QDebug>

using namespace IncidenceEditorNG;

AttendeeEditor::AttendeeEditor(QWidget *parent)
    : MultiplyingLineEditor(new AttendeeLineFactory(parent), parent)
{
    connect(this, SIGNAL(lineAdded(KPIM::MultiplyingLine*)),
            SLOT(slotLineAdded(KPIM::MultiplyingLine*)));

    addData();
}

void AttendeeEditor::slotLineAdded(KPIM::MultiplyingLine *line)
{
    AttendeeLine *att = qobject_cast<AttendeeLine *>(line);
    if (!att) {
        return;
    }

    connect(att, SIGNAL(changed()), SLOT(slotCalculateTotal()));
    connect(att, SIGNAL(changed(KCalCore::Attendee::Ptr,KCalCore::Attendee::Ptr)),
            SIGNAL(changed(KCalCore::Attendee::Ptr,KCalCore::Attendee::Ptr)));
    connect(att, SIGNAL(editingFinished(KPIM::MultiplyingLine*)),
            SIGNAL(editingFinished(KPIM::MultiplyingLine*)));
}

void AttendeeEditor::slotCalculateTotal()
{
    int empty = 0;
    int count = 0;

    foreach (KPIM::MultiplyingLine *line, lines()) {
        AttendeeLine *att = qobject_cast<AttendeeLine *>(line);
        if (att) {
            if (att->isEmpty()) {
                ++empty;
            } else {
                ++count;
            }
        }
    }
    emit countChanged(count);
    // We always want at least one empty line
    if (empty == 0) {
        addData();
    }
}

AttendeeData::List AttendeeEditor::attendees() const
{
    QList<KPIM::MultiplyingLineData::Ptr> dataList = allData();
    AttendeeData::List attList;
    //qDebug() << "num attendees:" << dataList.size();
    foreach (KPIM::MultiplyingLineData::Ptr datum, dataList) {
        AttendeeData::Ptr att = qSharedPointerDynamicCast<AttendeeData>(datum);
        if (!att) {
            continue;
        }
        attList << att;
    }
    return attList;
}

void AttendeeEditor::addAttendee(const KCalCore::Attendee::Ptr &attendee)
{
    addData(AttendeeData::Ptr(new AttendeeData(attendee)));
}

void AttendeeEditor::removeAttendee(const AttendeeData::Ptr &attendee)
{
    removeData(attendee);
}

void AttendeeEditor::setActions(AttendeeLine::AttendeeActions actions)
{
    foreach (KPIM::MultiplyingLine *line, lines()) {
        AttendeeLine *att = qobject_cast<AttendeeLine *>(line);
        att->setActions(actions);
    }
}
