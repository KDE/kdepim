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
#include "popupaddressee.h"
#include "popupnumber.h"
#include <kicon.h>
#include <kdebug.h>
#include <klocale.h>

popupAddressee::popupAddressee(const QString &deviceName, const KABC::Addressee &_addressee, QWidget *parent, bool ro)
 : KMenu(parent), addressee(_addressee)
{
    setObjectName(deviceName);
    if(addressee.isEmpty()) return;
    addTitle( KIcon( "personal" ), addressee.formattedName() );
    if(!ro) {
        addAction( KIcon("edit"), i18n("Edit"), this, SLOT(slotEdit()));
        addAction( KIcon("editdelete"), i18n("Delete"), this, SLOT(slotDelete()));
    }
    KABC::PhoneNumber::List::Iterator it;
    KABC::PhoneNumber::List p_list=addressee.phoneNumbers();
    QMenu *curmenu;
    for( it=p_list.begin(); it!=p_list.end() ;++it )
    {
        curmenu=new popupNumber( objectName(), (*it).number(), this );
        curmenu->setTitle((*it).number() );
        addMenu(curmenu );
    }
}


popupAddressee::~popupAddressee()
{
}


#include "popupaddressee.moc"


void popupAddressee::slotEdit()
{
    emit editClicked( addressee );
}

void popupAddressee::slotDelete()
{
    emit delContact();
}

