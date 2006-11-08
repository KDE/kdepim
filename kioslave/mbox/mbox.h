/*
 * This is a simple kioslave to handle mbox-files.
 * Copyright (C) 2004 Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef MBOX_H
#define MBOX_H

#include <kio/slavebase.h>
//Added by qt3to4:
#include <QByteArray>

class QByteArray;
class KUrl;

/**
 * This class is the main class and implements all function
 * which can be called through the user.
 */
class MBoxProtocol : public KIO::SlaveBase
{
public:
	/**
	 * Constructor, for the parameters,  See KIO::SlaveBase
	 */
	MBoxProtocol( const QByteArray&, const QByteArray& );
	/**
	 * Empty destructor
	 */
	virtual ~MBoxProtocol();

	/**
	 * This functions is used when an user or a program wants to
	 * get a file from a mbox-file
	 * @param url The url which points to the virtual file to get
	 */
	virtual void get( const KUrl& url );

	/**
	 * This functions gives a listing back.
	 * @param url The url to the mbox-file.
	 */
	virtual void listDir( const KUrl& url );

	/**
	 * This functions gives general properties about a mbox-file,
	 * or mbox-email back.
	 */
	virtual void stat( const KUrl& url );

	/**
	 * This functions determinate the mimetype of a given mbox-file or mbox-email.
	 * @param url The url to get the mimetype from
	 */
	virtual void mimetype( const KUrl& url );

	/**
	 * Through this functions, other class which have an instance to this
	 * class (classes which are part of kio_mbox) can emit an error with
	 * this function
	 * @param errno The error number to be thrown
	 * @param arg The argument of the error message of the error number.
	 */
	void emitError( int _errno, const QString& arg );
private:
	bool m_errorState;
};

#endif

