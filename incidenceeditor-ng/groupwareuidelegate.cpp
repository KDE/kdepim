/*
  Copyright (c) 2010 Kevin Ottens <ervin@kde.org>

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
*/

#include "groupwareuidelegate.h"
#include "incidencedialog.h"
#include "incidencedialogfactory.h"

#include <calendarsupport/utils.h>

#include <Akonadi/Item>

using namespace IncidenceEditorNG;

void GroupwareUiDelegate::requestIncidenceEditor(const Akonadi::Item &item)
{

// TODO_KDE5:
// The GroupwareUiDelegate interface should be a QObject. Right now we have no way of emitting a
// finished signal, so we have to use dialog->exec();

#ifndef KDEPIM_MOBILE_UI
    const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence(item);
    if (!incidence) {
        kWarning() << "Incidence is null, won't open the editor";
        return;
    }

    IncidenceDialog *dialog = IncidenceDialogFactory::create(/*needs initial saving=*/ false,
                                                             incidence->type(), 0);
    dialog->setAttribute( Qt::WA_DeleteOnClose, false );
    dialog->setIsCounterProposal(true);
    dialog->load(item, QDate::currentDate());
    dialog->exec();
    dialog->deleteLater();
    Akonadi::Item newItem = dialog->item();
    if (newItem.hasPayload<KCalCore::Incidence::Ptr>()) {
        KCalCore::IncidenceBase::Ptr newIncidence = newItem.payload<KCalCore::Incidence::Ptr>();
        *incidence.staticCast<KCalCore::IncidenceBase>() = *newIncidence;
    }
#else
    Q_UNUSED(item);
#endif
}

void GroupwareUiDelegate::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    // We don't need a calendar.
    Q_UNUSED(calendar);
}

void GroupwareUiDelegate::createCalendar()
{
    // We don't need a calendar
}
