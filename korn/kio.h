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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * kio.h -- Declaration of class KKIODrop.
 */
#ifndef KEG_KIODROP_H
#define KEG_KIODROP_H

#include "polldrop.h"

class QWidget;
class KDropDialog;
class KornMailSubject;
class KProcess;
class KIO_Count;
class KIO_Protocol;
class KIO_Subjects;
class KIO_Read;
class KIO_Delete;
class KURL;
template<class> class QPtrList;
template<class> class QValueList;
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
		
	KIO_Protocol * _protocol;
	QPtrList<KIO_Protocol> * _protocols;
	
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
	QValueList<FileInfo> *_mailurls;
	
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
	static const char *ProtoConfigKey;
	static const char *HostConfigKey;
	static const char *PortConfigKey;
	static const char *UserConfigKey;
	static const char *MailboxConfigKey;
	static const char *PassConfigKey;
	static const char *SavePassConfigKey;
	static const char *AuthConfigKey;

public:
	/**
	* KKioDrop Constructor
	*/
	KKioDrop();

	/** 
	  * Set the server that will be checked for new mail.
	 */
	void setKioServer( const QString & proto, const QString & server, int port = -1 );
	void setKioServer( const QString & proto, const QString & server, int port,
	                   const KIO::MetaData metadata, bool setProtocol = true ); //Last argument inits _protocol

	/** Set the account information for the PROTO server. */
	void setUser( const QString & user, const QString & password, const QString & mailbox, const QString & auth );

	// The next functions return settings
	QString protocol() const;
	QString server() const;
	int port() const;

	QString user() const;
	QString password() const;
	QString mailbox() const;
	QString auth() const;

	virtual void recheck();
	void forceRecheck();

	virtual bool valid();

	/**
	* KKioDrop Destructor
	*/
	virtual ~KKioDrop();

	virtual bool canReadSubjects(void);
	virtual QValueVector<KornMailSubject> * doReadSubjects(bool * stop); 
	
	virtual bool canDeleteMails();
	virtual bool deleteMails(QPtrList<const KornMailId> * ids, bool * stop);

	virtual bool canReadMail ();
	virtual QString readMail(const KornMailId * id, bool * stop);
	
	virtual KMailDrop* clone () const ;
	virtual bool readConfigGroup ( const KConfigBase& cfg );
	virtual bool writeConfigGroup ( KConfigBase& cfg ) const;
	virtual QString type() const { return QString::fromUtf8("kio"); }
	
	virtual bool synchrone() const { return false; } //class is not synchrone

	//virtual void addConfigPage( KDropCfgDialog * );

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
	
	void emitShowPassivePopup( QPtrList< KornMailSubject > *subject, int total )
			{ emit showPassivePopup( subject, total, passiveDate(), this->realName() ); }
	void emitValidChanged() { emit validChanged( valid() ); }

private slots:
	void slotConnectionError( int, const QString& );
	void slotConnectionWarning( const QString& );
	void slotConnectionInfoMessage( const QString& );
	
protected:
	//The next functions are needed for Process;
	virtual bool startProcess();
	virtual bool stopProcess();

public slots:
	virtual void readSubjectsCanceled();
	virtual void readMailCanceled();
	virtual void deleteMailsCanceled();
	
private slots:
	//For Process too
	void processExit(KProcess*);
	void receivedStdout( KProcess *, char *, int);
};
#endif // KEG_KIODROP_H
