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

#ifndef MK_QMAIL_PROTO_H
#define MK_QMAIL_PROTO_H

/**
 * @file
 *
 * This file implements the class QMail_Protocol
 */

#include "kio_proto.h"

template< class T > class QList;

/**
 * This class is used to fetch emails from qmail boxes or maildir directories.
 * It is a subclass of KIO_Protocol, because the file kio-slave is used to fetch the messages.
 */
class QMail_Protocol : public KIO_Protocol
{
public:
	/**
	 * Empty constructor
	 */
	QMail_Protocol() {}
	/**
	 * Empty destructor
	 */
	virtual ~QMail_Protocol() {}

	/**
	 * This function return itselfs
	 * This is because it no configuration of this type it is needed
	 * to use another protocol.
	 *
	 * @param config the configuration
	 * @return itselfs, because never another protocol have to be used.
	 */
	virtual const Protocol* getProtocol( KConfigGroup* ) const { return this; }

	/**
	 * This function duplicates itselfs.
	 *
	 * @return a copy of itself
	 */
	virtual KIO_Protocol * clone() const { return new QMail_Protocol; }

	/**
	 * This function always returns false, because the protocol
	 * isn't based on a connection.
	 *
	 * @return in this class always false
	 */
	virtual bool connectionBased() const { return false; }

	/**
	 * This function returns the name of the kioslave which have to be used.
	 * In this class, it is the "file" kioslave.
	 *
	 * @return the function always returns "file" for this class.
	 */
	virtual QString protocol() const { return "file"; }
	/**
	 * This function returns the name of the protocol used in the configuration file.
	 *
	 * @return this function always returns "qmail" in this class
	 */
	virtual QString configName() const { return "qmail"; }

	/**
	 * This function is true, because it is possible to read subjects using this protocol.
	 *
	 * @return always true for this class
	 */
	virtual bool canReadSubjects() const { return true; }
	/**
	 * This function returns true, because it is possible to delete emails.
	 *
	 * @return always true for this class
	 */
	virtual bool canDeleteMail() const { return true; }
	/**
	 * This function returns true, because it is possible to read full messages.
	 *
	 * @return always true for this class
	 */
	virtual bool canReadMail() const { return true; }
	/**
	 * This function returns true, because a fetched subjects always is the whole message.
	 *
	 * @return always true for this class
	 */
	virtual bool fullMessage() const { return true; }

	/**
	 * This function alter the url such that it only looks in the directory new.
	 *
	 * @param kurl the url to be changed
	 */
	virtual void recheckKURL( KUrl &kurl, KIO::MetaData& ) const
		{ if( kurl.path().right( 1 ) == "/" )
		  	kurl.setPath( kurl.path() +  "new" );
		  else
		  	kurl.setPath( kurl.path() + "/new" );
		}
	 
	/**
	 * This function returns the name which is used a title in the configuration.
	 *
	 * @return for this class, always "path"
	 */
	virtual QString mailboxName() const { return i18n( "Path:" ); }

	/**
	 * This function append a groupboxe "Maildir" to the list.
	 *
	 * @param list A pointer to a stringlist. After returning, this list contains a new item.
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * This function adds a KUrl widget to the first vector.
	 * The value is used to get the path value for the mail box.
	 *
	 * @param vector a vector with the first groupbox
	 * @param obj a object to connect signals to (not used in this class)
	 * @param result a list where a new input can be added to
	 */
        virtual void configFields( QVector< QWidget* >* vector, const QObject* obj, QList< AccountInput* >* result ) const;
	/**
	 * This function doesn't do anything in this class.
	 *
	 * @param config the settings which can be manipulated (it is not touched in this class)
	 * @param metadata a mapping for metadata; it is merged at the other readEntries function (it is not touched in this class)
	 */
        virtual void readEntries( QMap< QString, QString >* config, QMap< QString, QString >* metadata ) const;
	/**
	 * This function clears some entries from the configuration map. The entries that are removed are not used any more.
	 *
	 * @param map the configuration mapping to be manipulated
	 */
        virtual void writeEntries( QMap< QString, QString > *map ) const;
};

#endif //MK_QMAIL_PROTO_H
