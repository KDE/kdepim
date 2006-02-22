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

#ifndef MK_KIO_PROTO_H
#define MK_KIO_PROTO_H

/**
 * @file
 * 
 * KIO can handle multiple protocols. But some protocols have their own
 * manipulations of KUrl or MetaData , and some protocols could do more then other
 * protocols. So, this file defines a generic class for specifying the specific
 * properties of a kio protocol.
 *
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */

class QString;
#include <kurl.h>
#include <kio/global.h>
#include <klocale.h>
#include <qstringlist.h>
#include "protocol.h"

#include "kio.h"

/**
 * This class is the base protocol for all Protocol implementations using kio.
 * It defines some kio-specific settings which are used while fetching emails.
 */
class KIO_Protocol : public Protocol
{
public:
	/**
	 * Constuctor; empty
	 */
	KIO_Protocol() {}

	/**
	 * Destructor; empty too
	 */
	virtual ~KIO_Protocol() {}

	/**
	 * Public enumeration of the delete type.
	 * A file can be deleted using two methods: KIO::del and KIO::get.
	 * The enumeration tells which method should be used.
	 *
	 * @param get KIO::get should be used if a file needs to be deleted
	 * @param del KIO::del should be used if a file needs to be deleted
	 */
	enum DeleteTypeEnum { get, del };

	/**
	 * This is an implementation of Protocol::createMaildrop().
	 * Because all kio-protocols uses this maildrop, this is defined in this class.
	 */
	virtual KMailDrop* createMaildrop( KConfigGroup* config ) const { return new KKioDrop( config ); }

	/**
	 * This is an implementation of Protocol::createConfig().
	 * It reads the configuration out the configuration file and appends the value to a configuration mapping.
	 *
	 * @param group the configuration group containing the settings
	 * @param password the password of the box (can come from a Wallet)
	 * @return a mapping containing the configuration of a kio-account.
	 */
	virtual QMap< QString, QString >* createConfig( KConfigGroup *group, const QString& password ) const;

	/**
	 * This function defines a default for the protocolname.
	 * The full description of this function is descripted in Protocol::protocol().
	 *
	 * If this function isn't reimplemented in a subclass, the "file"-kioslave is used.
	 *
	 * @param ssl if true, the ssl-version of the protocol (if any) should be returned
	 * @return the kioslave which should be used
	 */
	virtual QString protocol( bool ) const { return "file"; }

	/**
	 * Some protocols use conncetions (such as pop3, imap); other don't (such as file).
	 * This function should return true if the protocol is connection based.
	 *
	 * By default, it returns false.
	 *
	 * @return true if the protocol is connection based; false otherwise
	 */
	virtual bool connectionBased() const { return false; }

	/**
	 * This function should return true if this protocol is able to read the subjects messages.
	 *
	 * Because there will never exist an instance of this class, a can override this method.
	 * The default is false.
	 * 
	 * Normal, a maildrop is responsable for give this answer, but in the case of kio,
	 * several kio-protocols have differents results for this function.
	 *
	 * @return true if this protocol is able to read message; false otherwise
	 */
	virtual bool  canReadSubjects() const { return false; }
	/**
	 * This function should return true is this protocol is able to delete messages.
	 *
	 * Because there will never exist an instance of this class, a can override this method.
	 * The default is false.
	 * 
	 * Normal, a maildrop is responsable for give this answer, but in the case of kio,
	 * several kio-protocols have different resultss for this function.
	 *
	 * @return true if this protocol is able to delete messages; false otherwise
	 */
	virtual bool  canDeleteMail() const { return false; }
	/**
	 * This function should return true if this protocol is able to read the full mail.
	 *
	 * Because there will never exist an instance of this class, a can override this method.
	 * The default is false.
	 *
	 * Normal, a maildrop is responsable for give this answer, but in the case of kio,
	 * several kio-protocols have different resultss for this function.
	 *
	 * @return true if this protocol is able to read messages; false otherwise
	 */
	virtual bool  canReadMail() const { return false; }
	/**
	 * This function should return true if this protocols always reads the full message.
	 * Some protocols don't have a difference between reading a subject and reading a message.
	 * A subclass is able to pass this to the kio maildrop.
	 *
	 * Because there will never exist an instance of this class, a can override this method.
	 * The default is false.
	 *
	 * @return true if this protocol always reads the full message; false otherwise
	 */
	virtual bool  fullMessage() const { return false; }

	/**
	 * This function returns the default port for this kio-slave.
	 * 0 is the default value.
	 *
	 * @param ssl True if the ssl-port should be returned; false otherwise.
	 *            If the protocol doesn't support ssl, the argument is ignored.
	 * @return The default port of this kio protocol.
	 */
	virtual unsigned short defaultPort( bool ) const { return 0; }

	/**
	 * There are two possibilities to delete something: with a get command and with a del command.
	 * This function should return del if KIO::del should be used, and get if KIO::get should be used.
	 *
	 * @return del if KIO::del should be used to delete a message; get if KIO::get should be used to delete a message
	 */
	virtual DeleteTypeEnum deleteFunction() const { return del; }

	/**
	 * The next options are the input for the Authentication Combo, seperated by '|'.
	 * The name should be the same as the auth-metadata.
	 * The default value is a list with only "Plain" in it.
	 *
	 * @return a list of authentication strings.
	 */
	virtual QStringList authList() const { return QStringList::split( '|', "Plain", false ); }

	/**
	 * This function can manipulate a KUrl which is used after it for connecting.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * This function is called before connecting.
	 *
	 * @param url the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void recheckConnectKUrl( KUrl &, KIO::MetaData & ) const { }
	/**
	 * This function can manipulate a KUrl which is used for checking for new emails.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * This function is called before the email box is checked for new messages.
	 *
	 * @param url the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void recheckKUrl     ( KUrl &, KIO::MetaData & ) const { };
	/**
	 * This function can manipulate a KUrl which is used after it for connecting.
	 * After connecting, the connection is used to check for new emails.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * @param kurl the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void readSubjectConnectKUrl ( KUrl & kurl, KIO::MetaData & ) const { kurl.setPath( "" ); }
	/**
	 * This function can manipulate a KUrl which is used before reading a subject.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * This function is called before a particular subject is read.
	 *
	 * @param url the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void readSubjectKUrl ( KUrl &, KIO::MetaData & ) const { } //For editing a kurl (adding extra options)
	/**
	 * This function can manipulate a KUrl which is used after it for connecting.
	 * After the connecting, the connection is used to delete emails.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * @param kurl the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void deleteMailConnectKUrl( KUrl & kurl, KIO::MetaData & ) const { kurl.setPath( "" ); }
	/**
	 * This function can manipulate a KUrl which is used to delete an email.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * This function is called before an email is deleted.
	 *
	 * @param url the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void deleteMailKUrl  ( KUrl &, KIO::MetaData & ) const { }
	/**
	 * This function should returns true if it is neccesairry to commit a delete operation.
	 *
	 * Some protocols needs a committing command after a delete command is execute.
	 * This function can be used to determine if this protocol needs such a function.
	 *
	 * @return true if a commit command is neccesiary, false otherwise.
	 */
	virtual bool commitDelete() const { return false; }
	/**
	 * This function can manipulate a KUrl which is used for committing after a deleting operation.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * If a commit operation is neccesairry, this command can generate the kurl and metadata
	 * for commit the delete operation.
	 *
	 * @param url the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void deleteCommitKUrl( KUrl &, KIO::MetaData & ) const { }
	/**
	 * This function can manipulate a KUrl which is used before reading an email.
	 * 
	 * This is one of the functions to manipulation a KUrl.
	 * At some points in the code, a KUrl is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KUrl path, this can be prevented.
	 *
	 * This function is called before reading an email.
	 *
	 * @param url the kurl to manipulate
	 * @param metadata the metadata to manipulate
	 */
	virtual void readMailKUrl    ( KUrl &, KIO::MetaData & ) const { }


	/**
	 * This function returns a KIO_Protocol. It is used to prevent a dynamic cast.
	 *
	 * @return a casted protocol.
	 */
	virtual const KIO_Protocol* getKIOProtocol() const { return this; }

	/**
	 * This function is used to manipulate the settings before resing it.
	 *
	 * @param config the settings which can be manipulated.
	 */
	virtual void readEntries( QMap< QString, QString > *config ) const;
	/**
	 * This function is used to manipulate the settings before resing it.
	 *
	 * @param config the settings which can be manipulated.
	 * @param metadata a mapping for metadata; it is merged at the other readEntries function
	 * if that function isn't implemented.
	 */
	virtual void readEntries( QMap< QString, QString >* config, QMap< QString, QString >* metadata ) const = 0;
protected:
	/**
	 * This enumeration is used when returning the input fields of a protocol.
	 */
	enum Fields {	no_fields = 0, server = 1, port = 2, username = 4, password = 8,
			mailbox = 16, save_password = 32, metadata = 64 };

	/**
	 * This function is used to remove some info from the settings.
	 * It is used because most of the implementations of this class will use the code inside this function.
	 *
	 * @param map the configuration mapping.
	 * @param fields the fields to clear (thus, the fields that are not used)
	 */
	void clearFields( QMap< QString, QString > *map, const Fields fields ) const;
};

#endif //MK_KIO_PROTO_H
