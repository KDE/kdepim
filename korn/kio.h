/*
 * Copyright (C)       Kurt Granroth
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

/*
 * kio.h -- Declaration of class KKIODrop.
 */
#ifndef KEG_KIODROP_H
#define KEG_KIODROP_H

#include "polldrop.h"
#include <QList>

class QWidget;
class KDropDialog;
class KornMailSubject;
class KProcess;
class KIO_Count;
class KIO_Protocol;
class KIO_Subjects;
class KIO_Read;
class KIO_Delete;
class KConfigGroup;
class KURL;
template<class> class QList;
template<class> class QVector;
namespace KIO { class Job; class MetaData; class Slave; class TransferJob; }

/**
 * Polling monitor for IMAP4 maildrops.
 * @author Kurt Granroth (granroth@kde.org)
 * Changes to poll with KIO
 * @author Mart Kelder (mart.kde@hccnet.nl)
 * $version $Id$
 */
class KKioDrop : public KPollableDrop
{
 Q_OBJECT
private:
	KURL *_kurl;
	KIO::MetaData *_metadata;

	QString _password;
	
	bool _valid;
		
	const KIO_Protocol * _protocol;
	
	KIO_Count *_count;
	KIO_Subjects *_subjects;
	KIO_Read *_read;
	KIO_Delete *_delete;

	int _readSubjectsTotalSteps;
	int _deleteMailsTotalSteps;
	//For process
	KProcess *_process;
	
	//List of mailurls fetched by the last time emails were counted
	struct FileInfo {
		QString name;
		long size;
	};
	QList<FileInfo> *_mailurls;
	
	/*
	 * The help-classes of this functions are friend functions, because this way, they can
	 * access the _kurl and _metadata-values without a function; and this way, no signal or
	 * public functions are neccesairy to emit a signal from KKioDrop.
	 */
	friend class KIO_Count;
	friend class KIO_Subjects;
	friend class KIO_Read;
	friend class KIO_Delete;
	friend class KIO_Single_Subjects;
public:
	/** The name of the config key for the protocol */
	static const char *ProtoConfigKey;
	/** The name of the config key for the hostname */
	static const char *HostConfigKey;
	/** The name of the config key for the port */
	static const char *PortConfigKey;
	/** The name of the config key for the username */
	static const char *UserConfigKey;
	/** The name of the config key for the mailbox */
	static const char *MailboxConfigKey;
	/** The name of the config key for the password */
	static const char *PassConfigKey;
	/** The name of the config key for the savepassword (true/false) setting */
	static const char *SavePassConfigKey;
	/** The name of the config key for the metadata */
	static const char *MetadataConfigKey;

public:
	/**
	 * KKioDrop Constructor
	 */
	KKioDrop();
	/**
	 * Constructor
	 *
	 * @param config a configuration group for reading settings out
	 */
	KKioDrop( KConfigGroup* );

	/** 
	 * Set the server that will be checked for new mail.
	 *
	 * @param proto the protocol
	 * @param server the serer or host
	 * @param port the port number
	 */
	void setKioServer( const QString & proto, const QString & server, int port = -1 );
	/** 
	 * Set the server that will be checked for new mail.
	 *
	 * @param proto the protocol for this account
	 * @param server the serer or host for this account
	 * @param port the port number for this account
	 * @param metadata the metadata for this account
	 * @param ssl true if ssl should used; false otherwise
	 * @param setProtocol true if _protocol should be initialized
	 */
	void setKioServer( const QString & proto, const QString & server, int port,
	                   const KIO::MetaData metadata, bool ssl, bool setProtocol = true ); //Last argument inits _protocol

	/**
	 * Set the account information for the PROTO server.
	 *
	 * @param user the username of this account
	 * @param password the password for this account
	 * @param mailbox the mailbox setting for this account
	 * @param auth the autentication for this account
	 */
	void setUser( const QString & user, const QString & password, const QString & mailbox, const QString & auth );

	/**
	 * This function returns the name of the protocol of this account
	 *
	 * @return the name of the protocol for this account
	 */
	QString protocol() const;
	/**
	 * This function returns the hostname.
	 *
	 * @return the hostname of the account
	 */
	QString server() const;
	/**
	 * This function returns the port
	 *
	 * @return the port of this account
	 */
	int port() const;

	/**
	 * This function returns the username.
	 *
	 * @return the username of this account
	 */
	QString user() const;
	/**
	 * This function returns the password.
	 *
	 * @return the password of this account
	 */
	QString password() const;
	/**
	 * This function returns the mailbox.
	 *
	 * @return the mailbox of this account
	 */
	QString mailbox() const;
	/**
	 * This function returns the authentication string.
	 *
	 * @return the authentication method of this account
	 */
	QString auth() const;

	/**
	 * This function rechecks the mailbox: it count the number of emails.
	 */
	virtual void recheck();
	/**
	 * This function always rechecks the mailbox, but it cancelled a pending count.
	 */
	virtual void forceRecheck();

	/**
	 * This function returns false if an errors occured.
	 *
	 * @return false if an errors occured; true otherwise
	 */
	virtual bool valid();

	/**
	* KKioDrop Destructor
	*/
	virtual ~KKioDrop();

	/**
	 * This function checks if it is possible to read subjects.
	 *
	 * @return true if it is possible to read subjects; false otherwise
	 */
	virtual bool canReadSubjects(void);
	/**
	 * This function reads the subjects of a mail box.
	 *
	 * @param stop a pointer to a boolean to cancel the function
	 * @return a list of subjects, of 0 is asynchrone
	 */
	virtual QVector<KornMailSubject> * doReadSubjects(bool * stop); 
	
	/**
	 * This function checks if it is possible to delete emails.
	 * 
	 * @return true if it is possible to delete emails; false otherwise
	 */
	virtual bool canDeleteMails();
	/**
	 * This function deletes a list of emails
	 * 
	 * @param ids a list of emails id's to delete
	 * @param stop a pointer to a boolean to cancel the operation
	 * @return true if succesfull; false otherwise
	 */
	virtual bool deleteMails(QList<const KornMailId*> * ids, bool * stop);

	/**
	 * This function checks if it is possible to read emails.
	 *
	 * @return true if it is possible to read emails; false otherwise
	 */
	virtual bool canReadMail ();
	/**
	 * This function reads a certain email.
	 * The email can be specified with a KornMailId (more specific: a KornStringId)
	 *
	 * @param id the id of the email which must be read
	 * @param stop a pointer to a boolean to cancel the reading
	 * @return the message which is read
	 */
	virtual QString readMail(const KornMailId * id, bool * stop);
	//TODO: remove *stop from parameter, and the return parameter
	
	/**
	 * This function returns a clone of the maildrop.
	 *
	 * @return a clone of this maildrop
	 */
	virtual KMailDrop* clone () const ;
	/**
	 * This function reads the configuration from a configuration mapping.
	 * The mapping is constructed from the configuration file.
	 *
	 * @param map the configuration mapping containing the settings of the account
	 * @param protocol a pointer to a Protocol class containing information about the protocol
	 * @return true if succesfull; false otherwise
	 */
	virtual bool readConfigGroup ( const QMap< QString, QString >& map, const Protocol * protocol );
	/**
	 * This function writes the configuration to a file
	 *
	 * @param cfg the configuration mapping: the settings are written to this group
	 * @return true if succesfull; false otherwise
	 */
	//TODO: delete this function (because writing is done during configuration). Note: resetcounter should be written
	virtual bool writeConfigGroup ( KConfigBase& cfg ) const;
	
	/**
	 * This function returns the typename of this maildrop.
	 * For this class, it always return "kio".
	 *
	 * @return the type of the Maildrop, for this class always "kio"
	 */
	virtual QString type() const { return QString::fromUtf8("kio"); }
	
	//TODO: synchrone or asynchrone; not both. Delete synchone, asynchrone of both
	/**
	 * This function returns if this protocol is synchrone (blocks when checking)
	 * kio never blocks, so this function returns false
	 *
	 * @return true if the maildrop is synchrone; false otherwise
	 */
	virtual bool synchrone() const { return false; } //class is not synchrone

	/**
	 * This function returns if this protocol is synchrone (blocks when checking)
	 * kio never blocks, so this function returns true
	 *
	 * @return true if the maildrop is asynchrone; false otherwise
	 */
	virtual bool asynchrone() const { return true; }

private:
	KKioDrop& operator = ( const KKioDrop& );
	//static void encrypt( QString& str );
	//static void decrypt( QString& str );

	/*
	 * The next functions are called from the help-classes (which are friend of the class).
	 * The only thing this functions do is emitting signals.
	 */
	void emitRechecked() { emit rechecked(); }
	void emitChanged( int value ) { emit changed( value, this ); }
	
	void emitReadSubjectsReady( bool success ) { emit readSubjectsReady( success ); }
	void emitReadSubjectsRead( KornMailSubject * subject ) { emit readSubject( subject ); }
	void emitReadSubjectsTotalSteps( int value ) { _readSubjectsTotalSteps = value; emit readSubjectsTotalSteps( value ); }
	void emitReadSubjectsProgress( int value ) { emit readSubjectsProgress( _readSubjectsTotalSteps - value ); }
	
	void emitReadMailReady( QString* msg ) { emit readMailReady( msg ); }
	
	void emitDeleteMailsReady( bool value ) { emit deleteMailsReady( value ); }
	void emitDeleteMailsTotalSteps( int value ) { _deleteMailsTotalSteps = value; emit deleteMailsTotalSteps( value ); }
	void emitDeleteMailsProgress( int value ) { emit deleteMailsProgress( _deleteMailsTotalSteps - value ); }
	
	void emitShowPassivePopup( QList< KornMailSubject > *subject, int total )
			{ emit showPassivePopup( subject, total, passiveDate(), this->realName() ); }
	void emitShowPassivePopup( const QString& error )
	                { if( passivePopup() ) { emit showPassivePopup( error, this->realName() ); } }
	void emitValidChanged() { emit validChanged( valid() ); }

private slots:
	void slotConnectionError( int, const QString& );
	void slotConnectionWarning( const QString& );
	void slotConnectionInfoMessage( const QString& );
	
public slots:
	/**
	 * If this slot is called, a pending readSubjects is cancelled
	 */
	virtual void readSubjectsCanceled();
	/**
	 * If this slot is called, a pending readMail is cancelled
	 */
	virtual void readMailCanceled();
	/**
	 * If this slot is called, a pending deleteMail is cancelled
	 */
	virtual void deleteMailsCanceled();
};
#endif // KEG_KIODROP_H
