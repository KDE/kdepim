/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#include "popupsms.h"
#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>

popupSMS::popupSMS(SMS *sms, QWidget *parent)
    : KMenu(parent)
{
    p_sms=sms;
    addTitle( KIcon( "mail_generic" ), i18n("SMS") );
    addAction(KIcon("editdelete"), i18n("Delete"), this, SLOT(slotRemove()));
}


popupSMS::~popupSMS()
{
}


#include "popupsms.moc"

void popupSMS::slotRemove()
{
    emit remove( p_sms );
}
