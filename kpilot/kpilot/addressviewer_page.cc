/* todoviewer_page.cc			KPilot
**
** Copyright (C) 2007 Bertjan Broeksema <b.broeksema@kdemail.net>
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "addressviewer_page.h"

#include "options.h"
#include "pilotDatabase.h"
#include "pilotAddress.h"
#include "kpilotConfig.h"

AddressViewerPage::AddressViewerPage( QWidget *parent, const QString &dbPath )
	: ViewerPageBase( parent, dbPath, CSL1( "AddressDB" ), i18n( "Address Viewer" ) )
{

}

PilotAppInfoBase* AddressViewerPage::loadAppInfo()
{
	if( database()->isOpen() )
	{
		return new PilotAddressInfo( database() );
	}
	
	return 0L;
}

QListWidgetItem* AddressViewerPage::getListWidgetItem( PilotRecord *rec )
{
	if( !( rec->isDeleted() ) &&
		( !( rec->isSecret() ) || KPilotSettings::showSecrets() ) )
	{
		PilotAddress *address = new PilotAddress( rec );
		if( address )
		{
			int addressDisplayMode = KPilotSettings::addressDisplayMode();
		
			QListWidgetItem *newItem = new QListWidgetItem;
			newItem->setText( createTitle( address, addressDisplayMode ) );
			newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
			
			return newItem;
		}
	}
	
	return 0L;
}

QString AddressViewerPage::getRecordInfo( PilotRecord *rec )
{
	if( !( rec->isDeleted() ) &&
		( !( rec->isSecret() ) || KPilotSettings::showSecrets() ) )
	{
		PilotAddress *address = new PilotAddress( rec );
		if( address )
		{
			PilotAddressInfo *info = new PilotAddressInfo( database() );
		
			QString text(CSL1("<qt>"));
			text += address->getTextRepresentation( info, Qt::RichText );
			text += CSL1("</qt>\n");
			
			return text;
		}
	}
	
	return QString();
}

QString AddressViewerPage::createTitle(PilotAddress * address, int displayMode)
{
	QString title;

	switch (displayMode)
	{
	case 1:
		if (!address->getField(entryCompany).isEmpty())
		{
			title.append(address->getField(entryCompany));
		}
		if (!address->getField(entryLastname).isEmpty())
		{
			if (!title.isEmpty())
			{
				title.append( CSL1(", "));
			}

			title.append(address->getField(entryLastname));
		}
		break;
	case 0:
	default:
		if (!address->getField(entryLastname).isEmpty())
		{
			title.append(address->getField(entryLastname));
		}

		if (!address->getField(entryFirstname).isEmpty())
		{
			if (!title.isEmpty())
			{
				title.append( CSL1(", "));
			}
			title.append(address->getField(entryFirstname));
		}
		break;
	}

	return title;
}
