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

#ifndef DCOPDROP_H
#define DCOPDROP_H

#include "maildrop.h"

#include <dcopobject.h>

class DCOPDropInterface;
//class KDropCfgDialog;
class KornMailId;
class KornMailSubject;

class KConfigBase;

template<class A, class B> class QMap;
class QString;

/**
 * This class implements a KMailDrop for DCOP-objects.
 * This class handles all new messages which are comming in through DCOP.
 */
class DCOPDrop : public KMailDrop
{ Q_OBJECT
public:
	/**
	 * Constructor: no parameters
	 */
	DCOPDrop();
	/**
	 * Destructor
	 */
	virtual ~DCOPDrop();
	
	/**
	 * A DCOPDrop cannot produce error messages, so it always returns true.
	 *
	 * @return true is the box is valid
	 */
	virtual bool valid() { return true; }
	/**
	 * This forces the drop to recheck. It is inpossible to recheck dcop,
	 * so this function does nothing.
	 */
	virtual void recheck();
	/**
	 * This function starts the monitor, however, this drop is always running,
	 * and doesn't need to be started.
	 * The return value is always true.
	 *
	 * @return true
	 */
	virtual bool startMonitor();
	/**
	 * This function starts the monitor, however, this drop is always running,
	 * and doesn't need to be started.
	 * The return value is always true.
	 *
	 * @return true
	 */
	virtual bool stopMonitor();
	/**
	 * Return true is the monitor has been started before.
	 * @return true is it is 'running'.
	 */
	virtual bool running() { return _isRunning; }
	
	//virtual void addConfigPage( KDropCfgDialog* ) ;
	/**
	 * This function gives a new instance of a DCOPDrop.
	 *
	 * @return A new instance of a DCOPDrop.
	 */
	virtual KMailDrop* clone() const { return new DCOPDrop; }

	/**
	 * This function reeds the config which are shipped which the group.
	 *
	 * @param config The configuration group which contains the info for this account.
	 * @return The same value as KMailDrop::readConfigGroup( config ) returns.
	 */
	virtual bool readConfigGroup( const KConfigGroup& config );
	/**
	 * This function also reeds the configurion, but from a mapping.
	 *
	 * @param map The mapping containing the configuration.
	 * @param protocol The protocol which comes with the mapping.
	 *
	 * @return true is all information is retrieved succesfully.
	 */
	virtual bool readConfigGroup( const QMap< QString, QString > & map, const Protocol * protocol );
	/**
	 * This function writes the information to a config group.
	 *
	 * @param config The configuration to write true
	 * @return true if no error occured.
	 */
	virtual bool writeConfigGroup( KConfigBase& config ) const;
	/**
	 * This returns the type of the box, in this case allways "dcop".
	 *
	 * @return "dcop"
	 */
	virtual QString type() const;
	
	/**
	 * This function should return true if it uses a synchrone comminucation.
	 * It doens't, so it returns false.
	 *
	 * @return false
	 */
	virtual bool synchrone() const { return false; }
	
	/**
	 * Return true if it is possible to read the subjects of new email.
	 *
	 * @return true
	 */
	virtual bool canReadSubjects() { return true; }
	/**
	 * This function does reads the subjects.
	 * @param stop A variable which isn't used: only used for synchrone actions
	 * @return A QValueVector which KornMailSubject* instance for every new mail.
	 */
	virtual QValueVector< KornMailSubject >* doReadSubjects( bool *stop );
	
	/**
	 * This function should return true if it is possible to delete emails.
	 * This is possible, so it always return true.
	 *
	 * @return true
	 */
	virtual bool canDeleteMails() { return true; }
	/**
	 * This function deletes the email.
	 *
	 * @param ids The id's of the email which must be deleted.
	 * @param stop Not used: only for synchrone opererations.
	 * @return true if deleting was succesfull
	 */
	virtual bool deleteMails( QPtrList<const KornMailId> * ids, bool * stop );
	
	/**
	 * This function returns true if it is possible to read emails.
	 * The emails are only given at once as a whole, so reading is impossible.
	 *
	 * @return false
	 */
	virtual bool canReadMail() { return false; }
	
	
private:
	bool _isRunning;
	QMap< int, KornMailSubject* > *_msgList;
	QString *_name;
	int _counter;
	DCOPDropInterface *_interface;
	
	void eraseList( void );
	
private slots:
	void doReadSubjectsASync( void );
	
public: //accessed by DCOPDropInterface
	int addMessage( const QString& subject, const QString& message );
	bool removeMessage( int id );
	
	//accessed by DCOPDropCfg
	QString DCOPName() const;
	void setDCOPName( const QString& );
};

#endif
