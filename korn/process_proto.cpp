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


#include "process_proto.h"

#include <kdebug.h>

#include <tqlayout.h>
#include <tqptrvector.h>
#include <tqstringlist.h>

#include "account_input.h"

void Process_Protocol::configFillGroupBoxes( TQStringList* groupBoxes ) const
{
	groupBoxes->append( "Process" );
}

void Process_Protocol::configFields( TQPtrVector< TQWidget >* vector, const TQObject*, TQPtrList< AccountInput > *result ) const
{
	result->append( new URLInput( (TQWidget*)vector->at( 0 ), i18n( "Program:" ), "", "mailbox" ) );
}

void Process_Protocol::readEntries( TQMap< TQString, TQString >*, TQMap< TQString, TQString >* ) const
{
}

void Process_Protocol::writeEntries( TQMap< TQString, TQString >* map ) const
{
	clearFields( map, (KIO_Protocol::Fields)( server | port | username | password | save_password | metadata ) );
}
