/*
 * Copyright (C) 2005, Mart Kelder (mart.kde@hccnet.nl)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "nntp_proto.h"

#include "account_input.h"

#include <qwidget.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qptrvector.h>
#include <qptrlist.h>

void Nntp_Protocol::configFillGroupBoxes( QStringList* groupBoxes ) const
{
	groupBoxes->append( "server" );
	groupBoxes->append( "user" );
}

void Nntp_Protocol::configFields( QPtrVector< QWidget >* vector, const QObject*, QPtrList< AccountInput > * result ) const
{
	result->append( new TextInput( (QWidget*)vector->at( 0 ), i18n( "Server" ), TextInput::text, "", "server" ) );
	result->append( new TextInput( (QWidget*)vector->at( 0 ), i18n( "Port" ), 0, 65535, "119", "port" ) );
	
	result->append( new TextInput( (QWidget*)vector->at( 1 ), i18n( "Username" ), TextInput::text, "", "username" ) );
	result->append( new TextInput( (QWidget*)vector->at( 1 ), i18n( "Password" ), TextInput::password, "", "password" ) );
	result->append( new CheckboxInput( (QWidget*)vector->at( 1 ), i18n( "Save password" ), "true", "savepassword" ) );
	QObject::connect( (QObject*)result->last()->rightWidget(), SIGNAL( toggled( bool ) ),
			  (QObject*)result->prev()->rightWidget(), SLOT( setEnabled( bool ) ) );
	result->last()->setValue( "false" );
}

void Nntp_Protocol::readEntries( QMap< QString, QString >*, QMap< QString, QString > * ) const
{
}

void Nntp_Protocol::writeEntries( QMap< QString, QString >* map ) const
{
	clearFields( map, (KIO_Protocol::Fields)( KIO_Protocol::mailbox | KIO_Protocol::metadata ) );
}

