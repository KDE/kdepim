/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "abstractcalendar.h"
#include "importwizard.h"
#include "importcalendarpage.h"

#include <KConfigGroup>

AbstractCalendar::AbstractCalendar(ImportWizard *parent)
    : mImportWizard(parent)
{
}

AbstractCalendar::~AbstractCalendar()
{
}

void AbstractCalendar::addImportInfo(const QString &log)
{
    mImportWizard->importCalendarPage()->addImportInfo(log);
}

void AbstractCalendar::addImportError(const QString &log)
{
    mImportWizard->importCalendarPage()->addImportError(log);
}

//eventviewsrc for calendar color for example
void AbstractCalendar::addEvenViewConfig(const QString &groupName, const QString &key, const QString &value)
{
    KSharedConfigPtr eventViewConfig = KSharedConfig::openConfig(QLatin1String("eventviewsrc"));
    KConfigGroup group = eventViewConfig->group(groupName);
    group.writeEntry(key, value);
    group.sync();
}
