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

#include "filteractionpipethrough.h"

#include <KLocale>

using namespace MailCommon;

FilterAction* FilterActionPipeThrough::newAction()
{
    return new FilterActionPipeThrough;
}

FilterActionPipeThrough::FilterActionPipeThrough( QObject *parent )
    : FilterActionWithCommand( QLatin1String("filter app"), i18nc("pipe through with command", "Pipe Through" ), parent )
{
}

FilterAction::ReturnCode FilterActionPipeThrough::process(ItemContext &context , bool) const
{
    return FilterActionWithCommand::genericProcess( context, true ); // use output
}

SearchRule::RequiredPart FilterActionPipeThrough::requiredPart() const
{
    return SearchRule::CompleteMessage;
}


