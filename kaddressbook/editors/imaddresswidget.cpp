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
#include <klocale.h>
#include <kplugininfo.h>

#include "imaddresswidget.h"

IMAddressWidget::IMAddressWidget( QWidget *parent, QValueList<KPluginInfo *> protocols ) : IMAddressBase( parent )
{
	mProtocols = protocols;
	populateProtocols();
}

IMAddressWidget::IMAddressWidget( QWidget *parent, QValueList<KPluginInfo *> protocols,
                                  KPluginInfo *protocol, const QString& address,
                                  const IMContext& context )
  : IMAddressBase( parent )
{
  Q_UNUSED( context );

	mProtocols = protocols;
	populateProtocols();
	cmbProtocol->setCurrentItem( mProtocols.findIndex( protocol ) );
	/*cmbContext->setCurrentItem( (int)context );*/
	edtAddress->setText( address );
}

KPluginInfo * IMAddressWidget::protocol()
{
	int protocolIndex = cmbProtocol->currentItem();
	return mProtocols[ protocolIndex ];
}

IMContext IMAddressWidget::context()
{
	IMContext context = Any;
/*	if ( cmbContext->currentItem() )
	{
		
		int contextIndex = cmbContext->currentItem();
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
	*/
	return context;
}

QString IMAddressWidget::address()
{
	return edtAddress->text();
}

void IMAddressWidget::populateProtocols()
{
	// insert the protocols in order
	QValueList<KPluginInfo *>::ConstIterator it;
	for ( it = mProtocols.begin(); it != mProtocols.end(); ++it )
		cmbProtocol->insertItem( SmallIcon( (*it)->icon() ), (*it)->name() );
}

