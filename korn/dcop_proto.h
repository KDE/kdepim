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


#ifndef DCOP_PROTO_H
#define DCOP_PROTO_H

#include "protocol.h"

class AccountInput;
class KConfigGroup;
class KIO_Protocol;
class KMailDrop;
class Protocol;

class QGroupBox;
class QObject;
class QStringList;
class QWidget;

template< class T> class QPtrList;
template< class T> class QPtrVector;
template< class T, class S> class QMap;

#include <qstring.h>

/**
 * This class implements a DCOP-protocol.
 * DCOP can be used to add messages to a box, or delete created dcop-messages.
 * This can be usefull in scripts.
 */
class DCOP_Protocol : public Protocol
{
public:
	/**
	 * Constructor
	 */
	DCOP_Protocol() {}
	/**
	 * Destructor
	 */
	virtual ~DCOP_Protocol() {}

	/**
	 * This function returns a Protocol pointer given a configuration.
	 * This function always returns itselfs, as the configuration never uses another protocol.
	 */
	virtual const Protocol* getProtocol( KConfigGroup* ) const { return this; }
	/**
	 * This function creates the maildrop used to count dcop-messages.
	 * @param config The configuration
	 */
	virtual KMailDrop* createMaildrop( KConfigGroup* config ) const;
	/**
	 * The function converts the information of the configuration file into a mapping.
	 *
	 * @param config The configuration instance to be mapped
	 * @return The keys and values of the configuration in a mapping
	 */
	virtual QMap< QString, QString > * createConfig( KConfigGroup* config, const QString& passwd ) const;
	/**
	 * This return the name of this protocol. It is always "dcop".
	 * @return The name of this protocol: "dcop"
	 */
	virtual QString configName() const { return "dcop"; }

	/**
	 * This function sets into the list the groupboxes.
	 *
	 * @param list A (empty) list, which is filled with the names of group-boxes.
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * This function filles the configuration field of this protocol.
	 * It is used to construct the configuration dialog.
	 *
	 * @param vector A vector with groupboxes.
	 * @param obj The pointer to the configDialog to connect signals to.
	 * @param result A list with AccountInput which is used to reconstruct the configuration.
	 */
	virtual void configFields( QPtrVector< QWidget >* vector, const QObject* obj, QPtrList< AccountInput >* result ) const;
	/**
	 * This function can edit some configuaration option before reading them.
	 */
	virtual void readEntries( QMap< QString, QString >* ) const;
	/**
	 * This function can edit some configuaration option before writing them.
	 */
	virtual void writeEntries( QMap< QString, QString >* ) const;

	//Functions that return a derived class.
	//This way, no explicit cast is needed
	/**
	 * This function returns a cast to a KIO_Protocol. Because this isn't a KIO_Protocol,
	 * it returns 0.
	 *
	 * @return 0
	 */
	virtual const KIO_Protocol* getKIOProtocol() const { return 0; }
};

#endif
