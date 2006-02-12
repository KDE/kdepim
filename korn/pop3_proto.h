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

#ifndef MK_POP3_PROTO_H
#define MK_POP3_PROTO_H

/**
 * @file
 *
 * This file contains the class Pop3_Protocol
 */

#include "kio_proto.h"

#include <kurl.h>

template< class T > class QList;

/**
 * This class contains the settings for the Pop3 protocol.
 * Because pop3 emails are fetched with KIO, it is derived from KIO_Protocol.
 */
class Pop3_Protocol : public KIO_Protocol
{
public:
	/**
	 * Constructor
	 */
	Pop3_Protocol()  {}
	/**
	 * Destructor
	 */
	virtual ~Pop3_Protocol() {}

	/**
	 * This returns a protocol pointer needed to read the configuration.
	 * In this class, it always returns itself (this).
	 *
	 * @param config the configuration group
	 * @return in this class, it returns a pointer to itselfs
	 */
	virtual const Protocol* getProtocol( KConfigGroup* ) const { return this; }

	//TODO: Check if clone() is still used
	/**
	 * Return a copy of this class.
	 * 
	 * @return a new instance of a Pop3 protocol
	 */
	virtual KIO_Protocol * clone() const { return new Pop3_Protocol; }

	/**
	 * This function returns true if the protocol is connection based.
	 * Pop3 is connection based, so this class always returns true.
	 *
	 * @return in this class, it always returns true
	 */
	virtual bool connectionBased() const { return true; }
	
	/**
	 * This function returns the kioslave which is used.
	 *
	 * @param ssl true if ssl should be used; false otherwise
	 * @return the name of the kioslave (in this class, pop3 or pop3s)
	 */
	virtual QString protocol( bool ssl ) const { return ssl ? "pop3s" : "pop3"; }
	/**
	 * This function returns the name for the configuration file.
	 * If the type is "pop3", this Protocol should be used.
	 *
	 * @return in this class, always "pop3"
	 */
	virtual QString configName() const { return "pop3"; }
	/**
	 * This function can be used to determine if it is possible to read subjects.
	 * For pop3, it is possible to read subjects, so this function returns true.
	 *
	 * @return in this class, always true
	 */
	virtual bool canReadSubjects() const { return true; }
	/**
	 * This function can be used to determine if it is possible to delete mails.
	 * For pop3, it is possible to delete mails, so this function returns true.
	 *
	 * @return in this class, always true
	 */
	virtual bool canDeleteMail() const { return true; }
	/**
	 * This function can be used to determine if it is possible to read emails.
	 * For pop3, it is possible to read emails, so this function returns true.
	 *
	 * @return in this class, always true
	 */
	virtual bool canReadMail() const { return true; }

	/**
	 * This function returns the default port.
	 * For pop3, it is 995 if ssl is used, and 110 is ssl isn't used.
	 *
	 * @param ssl true if ssl is used; false otherwise
	 * @return the default port for the protocol
	 */
	virtual unsigned short defaultPort( bool ssl ) const { return ssl?995:110; }

	/**
	 * This function returns the delete type.
	 * Delete is done with a "get" command.
	 * 
	 * @return in this class, always get.
	 */
	virtual DeleteTypeEnum deleteFunction() const { return get; }

	//TODO: check if authList is still used
	/**
	 * @return a stringlist
	 */
	virtual QStringList authList() const { return QStringList::split( '|', "Plain|APOP", false ); }
	
	/**
	 * This function manipuletes a kurl and metadata before the job is executed.
	 * In this class, the path of the kurl is changed.
	 * This function is called before subjects are read.
	 *
	 * @param kurl a kurl to update for this job
	 * @param metadata the metadata to update for this job
	 */
	virtual void readSubjectKUrl( KUrl & kurl, KIO::MetaData & ) { kurl.setPath( kurl.path().replace( "/download/", "/headers/" ) ); }
	/**
	 * This function manipuletes a kurl and metadata before the job is executed.
	 * In this class, the path of the kurl is changed.
	 * This function is called before files are deleted.
	 *
	 * @param kurl a kurl to update for this job
	 * @param metadata the metadata to update for this job
	 */
	virtual void deleteMailKUrl ( KUrl & kurl, KIO::MetaData & ) { kurl.setPath( kurl.path().replace( "/download/", "/remove/" ) ); }
	/**
	 * This function returns true if a delete command should be comitted.
	 * This is the case for pop3, so this function returns true for this class.
	 *
	 * @return in this class, always true
	 */
	virtual bool commitDelete () { return true; }
	/**
	 * This function manipuletes a kurl and metadata before the job is executed.
	 * In this class, the path of the kurl is changed.
	 * This function is called before a delete command is committed.
	 *
	 * @param kurl a kurl to update for this job
	 * @param metadata the metadata to update for this job
	 */
	virtual void deleteCommitKUrl(KUrl & kurl, KIO::MetaData & ) { kurl.setPath( "commit" ); }

	/**
	 * This function adds names of groupbases to the list.
	 * In this class, it adds two names to that list.
	 *
	 * @param list a list that contains the groupboxes after returning
	 */
	virtual void configFillGroupBoxes( QStringList* ) const;
	/**
	 * This function adds the input fields to the group boxes.
	 *
	 * @param vector a vector with group boxes
	 * @param object a QObject to connect signals to
	 * @param result after returning, it contains a list with input fields.
	 */
        virtual void configFields( QVector< QWidget* >* vector, const QObject*, QList< AccountInput* >* ) const;
	/**
	 * This function is used to alter the configuration before it is used.
	 * This class interpret the encryption.
	 *
	 * @param config the configuration mapping
	 * @param metadata the metadata already found in the configuration
	 */
        virtual void readEntries( QMap< QString, QString >* config, QMap< QString, QString >* metadata ) const;
	/**
	 * This function can be use to check which info is wrote.
	 * This class transforms the encryption option.
	 *
	 * @param config the configuration to be checked and changed
	 */
        virtual void writeEntries( QMap< QString, QString >* config ) const;
};

#endif //MK_POP3_PROTO_H
