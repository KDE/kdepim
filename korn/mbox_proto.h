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

#ifndef MK_MBOX_PROTO_H
#define MK_MBOX_PROTO_H

/**
 * @file
 *
 * This file contains the class MBox_Protocol
 */
#include "kio_proto.h"

template< class T > class QList;

/**
 * Protocol for mbox.
 * It uses the KIO to access the MBox-files, thus it is derived from KIO_Protocol
 */
class MBox_Protocol : public KIO_Protocol
{
public:
	/**
	 * Empty constructor
	 */
	MBox_Protocol() {}
	/**
	 * Empty destructor
	 */
	virtual ~MBox_Protocol() {}

	/**
	 * This function returns a new Protocol* after reading the configuration.
	 * Because the protocol never changes for this protocol, it returns itselfs.
	 *
	 * @param config the configuration
	 * @return for this class, it always returns this
	 */
	virtual const Protocol* getProtocol( KConfigGroup* ) const { return this; }

	/**
	 * This function returns the name of the kioslave.
	 * Because mbox over ssl doesn't exists, the ssl-parameter is ignored.
	 * In this class, it always returns "mbox".
	 *
	 * @param ssl True if it should use ssl; false otherwise. This parameter is ignored in this class.
	 * @return in this class, this function always returns "mbox"
	 */
	virtual QString protocol( bool ) const { return "mbox"; }
	/**
	 * The name to store this type of protocol in the configuration.
	 * For this class, that name always is "mbox".
	 *
	 * @return in this class, this function always returns "mbox"
	 */
	virtual QString configName() const { return "mbox"; }

	/**
	 * True if it is possible to read the subjects.
	 * Because it is always possible to read subjects, it always returns true.
	 *
	 * @return in this class, this function always returns true
	 */
	virtual bool canReadSubjects() const { return true; }
	/**
	 * True if it is possible to delete subjects.
	 * Because it is not yet possible to delete email with the mbox kioslave, it always returns false.
	 *
	 * @return in this class, this function always returns false
	 */
	virtual bool canDeleteMail() const { return false; }
	/**
	 * True if it is possible to read the emails.
	 * Because it is always possible to read emails, it always returns true.
	 *
	 * @return in this class, this function always returns true
	 */
	virtual bool canReadMail() const { return true; }
	/**
	 * True if a subjects also is the whole message.
	 * For this protocol, the whole message is read at ones.
	 *
	 * @return in this class, this function always returns true
	 */
	virtual bool fullMessage() const { return true; }

	//TODO: Is this function still used?
	/**
	 * @return "File:"
	 */
	virtual QString mailboxName() const { return i18n( "File:" ); }
	
	/**
	 * This function changes the kurl and metadata before the job is started.
	 * In this class, some metadata is added.
	 * 
	 * @param url the url (not touched in this class)
	 * @param md metadata (changed after running this function)
	 */
	virtual void recheckKUrl( KUrl &, KIO::MetaData & md ) const
		{ md.insert( "onlynew", "" ); md.insert( "savetime", "" ); }
	/**
	 * This function can alter the url and metadata before the job is started.
	 * This function does nothing for this class.
	 *
	 * @param url the url (not touched in this class)
	 * @param metadata the metadata (not touched in this class)
	 */
	virtual void readSubjectKUrl( KUrl &, KIO::MetaData & ) const { }
	/**
	 * This function can alter the url and metadata before the job is started.
	 * This function does nothing for this class.
	 *
	 * @param url the url (not touched in this class)
	 * @param metadata the metadata (not touched in this class)
	 */
	virtual void deleteMailKUrl( KUrl &, KIO::MetaData & ) const { }
	/**
	 * This function can alter the url and metadata before the job is started.
	 * This function does nothing for this class.
	 *
	 * @param url the url (not touched in this class)
	 * @param metadata the metadata (not touched in this class)
	 */
	virtual void readMailKUrl( KUrl &, KIO::MetaData & ) const   { }

	/**
	 * This function is used to retrieve the names of the groupboxes for the configuration.
	 * This functions append one name to the list.
	 *
	 * @param list the list with the names of groupboxes
	 */
	virtual void configFillGroupBoxes( QStringList* list ) const;
	/**
	 * This function append the configuration fields to the parameter @p result
	 *
	 * @param vector a vector containing the group boxes
	 * @param obj an QObject to connect signals to
	 * @param result a resulting list with created AccountInput's.
	 */
	virtual void configFields( QVector< QWidget* >* vector, const QObject* obj, QList< AccountInput* >* result ) const;
	/**
	 * This function is used to change the configuration before putting it on the screen.
	 * In this class, nothing is changed.
	 *
	 * @param config the configuration entries
	 * @param metadata the metadata entries from the configuration
	 */
	virtual void readEntries( QMap< QString, QString >* config, QMap< QString, QString >* metadata ) const;
	/**
	 * This function is used to change the settings that are wrote to the configuration.
	 * In this class, some entries are cleared.
	 *
	 * @param config the configuration mapping
	 */
	virtual void writeEntries( QMap< QString, QString >* config ) const;
};

#endif //MK_MBOX_PROTO_H
