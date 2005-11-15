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

#ifndef MK_PROCESS_PROTOCOL
#define MK_PROCESS_PROTOCOL

/**
 * @file
 *
 * This file defines an implementation of a Protocol for processes.
 * More information about the process protocol in process.h.
 *
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */

#include "protocol.h"

#include <qstring.h>

template< class T > class QList;

/**
 * A class that defines the properties of a process type.
 */
class Process_Protocol : public Protocol
{
public:
	/**
	 * Empty constructor
	 */
	Process_Protocol() { }
	/**
	 * Empty destructor
	 */
	virtual ~Process_Protocol() { }

	/**
	 * This is an implementation of the pure virtual method Protocol::getProtocol().
	 * In this class, it returns itselfs.
	 *
	 * @param config a configuration (it is ignored in this class)
	 * @return a pointer to a protocol; in this class always itself.
	 */
	virtual const Protocol* getProtocol( KConfigGroup* config ) const;
	/**
	 * This is an implementation of the pure virtual method Protocol::createMaildrop()
	 * In the class, it returns a new pointer to a ProcessDrop.
	 *
	 * @param config a configuration (which is ignored in this class)
	 * @return a new instance of a KMailDrop; in this class a new instance of a ProcessDrop
	 */
	virtual KMailDrop* createMaildrop( KConfigGroup* config ) const;
	/**
	 * This is an implementation of the Protocol::createConfig() method.
	 * In this class, it reads the entry 'program' out the file and put it into the configuartion mapping.
	 *
	 * @param config the configuration to read
	 * @param password the password of this account (ignored in this class)
	 * @return a new pointer to a configuartion mapping containing the entry and value of 'program'
	 */
	virtual QMap< QString, QString > * createConfig( KConfigGroup* config, const QString& password ) const;
	/**
	 * An implementation of the Protocol::configName() method.
	 * In this class, it always return "process".
	 *
	 * @return a string which represent the type of protocol; in this class always "process"
	 */
	virtual QString configName() const { return "process"; }
				        
	/**
	 * An implementation of the Protocol::configFillGroupBoxes() method.
	 * In this class, only groupbox is append.
	 *
	 * @param list the list to append the names of groups to; in this class, only one string is added
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * An implementation of the Protocol::configFields() method.
	 * In this class, one AccountInput for the program name is added to the first (and only) group box.
	 *
	 * @param vector A list with group boxes. In this class, this vector should always contain one item.
	 * @param obj the QObject of the dialog; ignored in this class
	 * @param inputs the list to append the input box to
	 */
	virtual void configFields( QVector< QWidget* >* vector, const QObject* obj, QList< AccountInput* >* inputs ) const;

	/**
	 * An implementation of the Protocol::readEntries() method.
	 *
	 * In this class, @p map is left untouched.
	 *
	 * @param map the configuration that can be changed (in this class, it is left untouched)
	 */
	virtual void readEntries( QMap< QString, QString >* map ) const;
	/**
	 * An implementation of the Protocol::writeEntries() method.
	 *
	 * In this class @p config_map is left untouched.
	 *
	 * @param config_map the configuration mapping; in this class it is left untouched
	 */
	virtual void writeEntries( QMap< QString, QString >* config_map ) const;
};

#endif //MK_PROCESS_PROTOCOL

