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
	virtual bool hasServer() const { return true; }
	virtual bool hasPort() const { return true; }
	virtual bool hasUsername() const { return true; }
	virtual bool hasMailbox() const { return true; }
	virtual bool hasPassword() const { return true; }
	virtual bool hasAuth() const { return false; }
	virtual unsigned short defaultPort() const { return 0; }

	/*
	 * This sets the string of such fields in Configuration
	 */
	virtual QString serverName() const { return i18n( "Server:" ); }
	virtual QString portName() const { return i18n( "Port:" ); }
	virtual QString usernameName() const { return i18n( "Username:" ); }
	virtual QString mailboxName() const { return i18n( "Mailbox:" ); }
	virtual QString passwordName() const { return i18n( "Password:" ); }
	virtual QString savePasswordName() const { return i18n( "Save Password:" ); }
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
