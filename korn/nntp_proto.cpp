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

#include <tqwidget.h>
#include <tqobject.h>
#include <tqstringlist.h>
#include <tqptrvector.h>
#include <tqptrlist.h>

void Nntp_Protocol::configFillGroupBoxes( TQStringList* groupBoxes ) const
{
	groupBoxes->append( "server" );
	groupBoxes->append( "user" );
}

void Nntp_Protocol::configFields( TQPtrVector< TQWidget >* vector, const TQObject*, TQPtrList< AccountInput > * result ) const
{
	result->append( new TextInput( (TQWidget*)vector->at( 0 ), i18n( "Server" ), TextInput::text, "", "server" ) );
	result->append( new TextInput( (TQWidget*)vector->at( 0 ), i18n( "Port" ), 0, 65535, "119", "port" ) );
	
	result->append( new TextInput( (TQWidget*)vector->at( 1 ), i18n( "Username" ), TextInput::text, "", "username" ) );
	result->append( new TextInput( (TQWidget*)vector->at( 1 ), i18n( "Password" ), TextInput::password, "", "password" ) );
	result->append( new CheckboxInput( (TQWidget*)vector->at( 1 ), i18n( "Save password" ), "true", "savepassword" ) );
	TQObject::connect( (TQObject*)result->last()->rightWidget(), TQT_SIGNAL( toggled( bool ) ),
			  (TQObject*)result->prev()->rightWidget(), TQT_SLOT( setEnabled( bool ) ) );
	result->last()->setValue( "false" );
}

void Nntp_Protocol::readEntries( TQMap< TQString, TQString >*, TQMap< TQString, TQString > * ) const
{
}

void Nntp_Protocol::writeEntries( TQMap< TQString, TQString >* map ) const
{
	clearFields( map, (KIO_Protocol::Fields)( KIO_Protocol::mailbox | KIO_Protocol::metadata ) );
}

