/*
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "dcopdropif.h"

#include "dcopdrop.h"

DCOPDropInterface::DCOPDropInterface( DCOPDrop* drop, const char* name )
	: DCOPObject( name ),
	_drop( drop )
{
}

DCOPDropInterface::~DCOPDropInterface()
{
}

void DCOPDropInterface::changeName( const QString& name )
{
	this->setObjId( name.utf8() );
}

int DCOPDropInterface::addMessage( const QString& subject, const QString& message )
{
	return _drop->addMessage( subject, message );
}

bool DCOPDropInterface::removeMessage( int id )
{
	return _drop->removeMessage( id );
}
