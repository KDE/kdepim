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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MK_KIO_PROTO_H
#define MK_KIO_PROTO_H

/*
 * KIO can handle multiple protocols. But some protocols have their own
 * manipulations of KURL or MetaData , and some protocols could do more then other
 * protocols. So, this class is the generic class of a class specified
 * by a protocol.
 */

class QString;
#include <kio/global.h>
#include <klocale.h>
#include <qstringlist.h>

class KIO_Protocol
{
public:
	/*
	 * Constuctor; empty
	 */
	KIO_Protocol() {}

	/*
	 * Destructor; empty too
	 */
	virtual ~KIO_Protocol() {}

	/*
	 * Public enumeration
	 */
	enum DeleteTypeEnum { get, del };
	/*
	 * This enumeration is used when returning the capebilitys of a protocol
	 */
	enum Fields {	no_fields = 0, server = 1, port = 2, username = 4, password = 8,
			mailbox = 16, save_password = 32, auth = 64 };

	/*
	 * Function to get another instance of the protocol.
	 * This function have to be overloaded in every inheritance.
	 */
	virtual KIO_Protocol * clone() const { return new KIO_Protocol; }

	/*
	 * @return: the name of the kio_slave
	 */
	virtual QString protocol() const { return "file"; }

	/*
	 * @return: the name of the protocol used by the configuration
	 */
	virtual QString configName() const { return "not specified"; }

	virtual bool connectionBased() const { return false; }

	/*
	 * The next four functions return the [capebilities] of a protocol.
	 * fullMessage means that by downloaden, the whole message is downloaded.
	 * if it is false, only the headers should be downloaded.
	 */
	virtual bool  canReadSubjects() const { return false; }
	virtual bool  canDeleteMail() const { return false; }
	virtual bool  canReadMail() const { return false; }
	virtual bool  fullMessage() const { return false; }

	/*
	 * The following lines are the options in the configuration;
	 * true means that an option is enabled;
	 * false means that the option is disabled.
	 */
	virtual int fields() const { return server | port | username | password | mailbox; }
	virtual int urlFields() const { return no_fields; }
	virtual unsigned short defaultPort() const { return 0; }

	/*
	 * This sets the string of such fields in Configuration
	 */
	virtual QString serverName() const { return i18n( "Server:" ); }
	virtual QString portName() const { return i18n( "Port:" ); }
	virtual QString usernameName() const { return i18n( "Username:" ); }
	virtual QString mailboxName() const { return i18n( "Mailbox:" ); }
	virtual QString passwordName() const { return i18n( "Password:" ); }
	virtual QString savePasswordName() const { return i18n( "Save password" ); }
	virtual QString authName() const { return i18n( "Authentication:" ); }

	/*
	 * The next function returns the method of deleting: some protoocols
	 * like to delete files with KIO::get; other with KIO::del
	 */
	virtual DeleteTypeEnum deleteFunction() const { return del; }

	/*
	 * The next options are the input for the Authentication Combo, seperated by '|'.
	 * The name should be the same as the auth-metadata.
	 */
	virtual QStringList authList() const { return QStringList::split( '|', "Plain", false ); }

	/*
	 * The next functions are manipulations of an KURL.
	 * At some points in the code, a KURL is used. But sometimes,
	 * these have to had a little retouch. That is possible with these function.
	 *
	 * For example, by imap, by default, the whole message is downloaded and marked as reed.
	 * By changing an option to the KURL path, this can be prevented.
	 *
	 * The most functions are recognized by name.
	 * commitDelete return true if a protocol has to confirm a deletion.
	 * It will be called after marking the messages for deletion.
	 * deleteCommitKURL is the KURL manipulator; the KURL is as in the settings.
	 * That KURL isn't retouch by deleteMailKURL.
	 */
	virtual void recheckConnectKURL( KURL &, KIO::MetaData & ) { }
	virtual void recheckKURL     ( KURL &, KIO::MetaData & ) { }
	virtual void readSubjectConnectKURL ( KURL & kurl, KIO::MetaData & ) { kurl.setPath( "" ); }
	virtual void readSubjectKURL ( KURL &, KIO::MetaData & ) { } //For editing a kurl (adding extra options)
	virtual void deleteMailConnectKURL( KURL & kurl, KIO::MetaData & ) { kurl.setPath( "" ); }
	virtual void deleteMailKURL  ( KURL &, KIO::MetaData & ) { }
	virtual bool commitDelete() { return false; }
	virtual void deleteCommitKURL( KURL &, KIO::MetaData & ) { }
	virtual void readMailKURL    ( KURL &, KIO::MetaData & ) { }
};

#endif //MK_KIO_PROTO_H
