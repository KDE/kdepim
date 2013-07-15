/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteractionbeep.h"

#include <KDE/KLocale>
#include <KDE/KNotification>

using namespace MailCommon;

FilterActionBeep::FilterActionBeep( QObject *parent )
    : FilterActionWithNone( QLatin1String("beep"), i18n( "Beep" ), parent )
{
}

FilterAction::ReturnCode FilterActionBeep::process( ItemContext &/*context*/ ) const
{
    KNotification::beep();
    return GoOn;
}


SearchRule::RequiredPart FilterActionBeep::requiredPart() const
{
    return SearchRule::Envelope;
}


FilterAction* FilterActionBeep::newAction()
{
    return new FilterActionBeep;
}

#include "filteractionbeep.moc"
