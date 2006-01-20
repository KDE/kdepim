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

#ifndef KIO_CONNECTION
#define KIO_CONNECTION

/**
 * @file
 *
 * This file defines the class KIO_Connection
 */

class KIO_Protocol;

class KURL;
namespace KIO { class Slave; class MetaData; }

template< class T, class S > class QMap;

/**
 * This class is used to keep track of several connection.
 * The target is to prevent that multiple connections are opened
 * to the save host.
 */
class KIO_Connection
{
public:
	/**
	 * This function gets the slave given a kurl.
	 * It uses another slave if that is already available.
	 * A slave getted with this function should always be deleted with removeSlave of this class.
	 *
	 * @param kurl the kurl of the connection
	 * @param metadata the metadata for the kurl
	 * @param protocol the used protocol
	 * @return a slave which can be used for jobs, 0 if it fails to open one
	 */
	static KIO::Slave* getSlave( const KURL& kurl, const KIO::MetaData& metadata, const KIO_Protocol* protocol );

	/**
	 * This function must be called if a job is finished.
	 * It removes the slave is no other connections are pending
	 */
	static void removeSlave( KIO::Slave* slave );
private:
	/**
	 * This function inits the data of this class.
	 */
	static void initData();
	
	static QMap< KIO::Slave*, int > *m_slaves; //Function from a slave to how often the slave is used.
};

#endif //KIO_CONNECTION
