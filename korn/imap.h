/*
 * imap.h -- Declaration of class KImap4Drop.
 */
#ifndef KEG_IMAPDROP_H
#define KEG_IMAPDROP_H

#include <qstring.h>
#include "polldrop.h"

class KBiffImap;
class QWidget;
class KDropDialog;

/**
 * Polling monitor for IMAP4 maildrops.
 * @author Kurt Granroth (granroth@kde.org)
 * $version $Id$
 */
class KImap4Drop : public KPollableDrop
{
private:
	QString _server;
	int	_port;

	QString _user;
	QString _password;
	QString _mailbox;
	bool	_savePassword;

	bool _valid;

	KBiffImap *_imap;

public:
	static const char *HostConfigKey;
	static const char *PortConfigKey;
	static const char *UserConfigKey;
	static const char *MailboxConfigKey;
	static const char *PassConfigKey;
	static const char *SavePassConfigKey;
	static const int  DefaultPort;

public:
	/**
	* KImap4Drop Constructor
	*/
	KImap4Drop();

	/** 
	  * Set the IMAP4 server that will be checked for new mail.
	 */
	void setImapServer( const QString & server, int port = DefaultPort );

	/** Set the account information for the IMAP server. */
	void setUser( const QString & user, const QString & password,
    const QString & mailbox, bool savepwd = false );

	QString server() const { return _server; }
	int port() const { return _port; }

	QString user() const { return _user; }
	QString password() const { return _password; }
	QString mailbox() const { return _mailbox; }

	virtual void recheck();

	virtual bool valid();

	/**
	* KImap4Drop Destructor
	*/
	virtual ~KImap4Drop();


	virtual KMailDrop* clone () const ;
	virtual bool readConfigGroup ( const KConfigBase& cfg );
	virtual bool writeConfigGroup ( KConfigBase& cfg ) const;
	virtual QString type() const { return QString::fromUtf8("imap4"); }

	virtual void addConfigPage( KDropCfgDialog * );

private:
	KImap4Drop& operator = ( const KImap4Drop& );
	static void encrypt( QString& str );
	static void decrypt( QString& str );
};
#endif // KEG_IMAPDROP_H
