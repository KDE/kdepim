/*    
	imaddresswidget.cpp
	
	IM address editor widget for KAddressbook
	
	Copyright (c) 2004 Will Stephenson   <lists@stevello.free-online.co.uk>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>

#include <kdebug.h>
#include <kiconloader.h>

#include "imaddresswidget.h"

IMAddressWidget::IMAddressWidget( QWidget *parent ) : IMAddressBase( parent )
{
	populateProtocols();
}

IMAddressWidget::IMAddressWidget( QWidget *parent, const IMProtocol& protocol, const QString& address, const IMContext& context/*, bool inVCard*/ ) : IMAddressBase( parent )
{
	populateProtocols();
	cmbProtocol->setCurrentItem( (int)protocol );
	cmbContext->setCurrentItem( (int)context );
	edtAddress->setText( address );
	//chkVCard->setChecked( inVCard );
}

IMProtocol IMAddressWidget::protocol()
{
	IMProtocol protocol = Unknown;
	if ( cmbProtocol->currentItem() )
	{
		int protocolIndex = cmbProtocol->currentItem();
		
		// get the protocol corresponding to the selected item
		switch ( protocolIndex )
		{
		case 0:
			protocol = AIM;
			break;
		case 1:
			protocol = GaduGadu;
			break;
		case 2:
			protocol = Jabber;
			break;
		case 3:
			protocol = ICQ;
			break;
		case 4:
			protocol = IRC;
			break;
		case 5:
			protocol = MSN;
			break;
		case 6:
			protocol = SMS;
			break;
		case 7:
			protocol = Yahoo;
			break;
		default:
			protocol = Unknown;
			kdDebug( 0 ) << k_funcinfo << "Unknown protocol selected! Check populateProtocol()" << endl;
		}
	}
	return protocol;
}

IMContext IMAddressWidget::context()
{
	IMContext context = Any;
	if ( cmbContext->currentItem() )
	{
		
		int contextIndex = cmbContext->currentItem();
		kdDebug( 0 ) << "current context is " << contextIndex << endl;
		switch ( contextIndex )
		{
		case 0:
			context = Any;
			break;
		case 1:
			context = Home;
			break;
		case 2:
			context = Work;
			break;
		}
	}
	return context;
}

QString IMAddressWidget::address()
{
	return edtAddress->text();
}

/*
bool IMAddressWidget::inVCard()
{
	return chkVCard->isChecked();
}
*/

void IMAddressWidget::populateProtocols()
{
	// insert the protocols in order
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("aim_protocol") ), QString::fromLatin1("AIM") );
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("gadu_protocol") ), QString::fromLatin1("Gadu-Gadu") );
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("jabber_protocol") ), QString::fromLatin1("Jabber") );
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("icq_protocol") ), QString::fromLatin1("ICQ") );
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("irc_protocol") ), QString::fromLatin1("IRC") );
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("msn_protocol") ), QString::fromLatin1("MSN") );
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("sms_protocol") ), QString::fromLatin1("SMS") );
	cmbProtocol->insertItem( SmallIcon(QString::fromLatin1("yahoo_protocol") ), QString::fromLatin1("Yahoo") );
}

