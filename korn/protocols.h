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

#ifndef MK_PROTOCOLS_H
#define MK_PROTOCOLS_H

/**
 * @file
 *
 * This file contains the class Protocols.
 * Protocols have some static function which can be used to obtain a Protocol* with the a given name.
 */

class Protocol;

class QString;
class QStringList;
template< class T, class U > class QHash;

/**
 * This class contains one Protocol pointer for each protocol.
 * A Protocol* can be obtained with the getProto( const QString& ) function.
 * If that doesn't work, it is possible to get a fallbackprotocol.
 */
class Protocols
{
public:
	/**
	 * Empty constructor
	 */
	Protocols() {}
	/**
	 * Empty destructor
	 */
	~Protocols() {}
	
	/**
	 * This function returns a pointer to a protocol with name @p name.
	 * 
	 * @param name the name of the protocol to be found
	 * @return a pointer to a Protocol with configName() name.
	 */
	static const Protocol* getProto( const QString& name );
	/**
	 * Returns a Protocol* which can be used if a Protocol* with a particular name isn't found.
	 * In practice, it always returns a "mbox"-protocol.
	 *
	 * @return Protocol* a arbitrairy protocol.
	 */
	static const Protocol* firstProtocol();
	
	/**
	 * This function returns a list with all registered protocols
	 *
	 * @return a list with all registered protocols
	 */
	static QStringList getProtocols();
private:
	static void fillProtocols();

	static void addProtocol( Protocol* );

	static QHash< QString, Protocol* > *protocols;
};

#endif //MK_PROTOCOLS_H
