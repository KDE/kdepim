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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef DCOPDROPIF_H
#define DCOPDROPIF_H

#include <dcopobject.h>

class DCOPDrop;

/**
 * This DCOP-interface for the DCOPdrop.
 */
class DCOPDropInterface : virtual public DCOPObject
{
K_DCOP
public:
	/**
	 * Constructor
	 * @param drop The parent DCOPDrop
	 * @param name The name of the dcop-object
	 */
	DCOPDropInterface( DCOPDrop* drop, const char* name );
	/**
	 * Destructor
	 */
	~DCOPDropInterface();
	
	/**
	 * This function is used to change the dcop-name
	 *
	 * @param name The new name of this dcop interface.
	 */
	void changeName( const QString& name );
k_dcop:
	/**
	 * This function adds a message to list of new messages.
	 *
	 * @param subject The subject of the message
	 * @param message The body of the message
	 * @return The id which this message got
	 */
	int addMessage( const QString& subject, const QString& message );
	/**
	 * This function removes a message from the list of new messages.
	 *
	 * @param id The id of the message to be deleted. The id can be obtained in
	 * 	the "addMessage" dcop call.
	 */
	bool removeMessage( int id );
	
private:
	DCOPDrop *_drop;
};

#endif
