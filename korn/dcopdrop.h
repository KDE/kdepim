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

#ifndef DBUSDROP_H
#define DBUSDROP_H

/**
 * @file
 *
 * This file contains the DBUSDrop class.
 */

#include "maildrop.h"

class KornMailSubject;

class KConfigBase;

template<class A, class B> class QMap;
template<class T> class QList;
class QString;
class QVariant;

/**
 * This class implements a KMailDrop for DBUS-objects.
 * This class handles all new messages which are coming in through DBUS.
 */
class DBUSDrop : public KMailDrop
{ Q_OBJECT
public:
	/**
	 * Constructor: no parameters
	 */
	DBUSDrop();
	/**
	 * Destructor
	 */
	virtual ~DBUSDrop();

	/**
	 * A DBUSDrop cannot produce error messages, so it always returns true.
	 *
	 * @return true is the box is valid
	 */
	virtual bool valid() { return true; }
	/**
	 * This forces the drop to recheck. It is inpossible to recheck DBUS,
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
	 *
	 * @return true is it is 'running'.
	 */
	virtual bool running() { return _isRunning; }

	/**
	 * This function gives a new instance of a DBUSDrop.
	 *
	 * @return A new instance of a DBUSDrop.
	 */
	virtual KMailDrop* clone() const { return new DBUSDrop; }

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
	 * @return true is all information is retrieved successfully.
	 */
	virtual bool readConfigGroup( const QMap< QString, QString > & map, const Protocol * protocol );
	/**
	 * This function writes the information to a config group.
	 *
	 * @param config The configuration to write true
	 * @return true if no error occurred.
	 */
	virtual bool writeConfigGroup( KConfigBase& config ) const;
	/**
	 * This returns the type of the box, in this case always "dbus".
	 *
	 * @return "dbus"
	 */
	virtual QString type() const;

	/**
	 * This function should return true if it uses a synchrone comminucation.
	 * It doesn't, so it returns false.
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
	virtual QVector< KornMailSubject >* doReadSubjects( bool *stop );

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
	virtual bool deleteMails( QList<QVariant> * ids, bool * stop );

	/**
	 * This function returns true if it is possible to read emails.
	 * The emails are only given at once as a whole, so reading is impossible.
	 *
	 * @return false
	 */
	virtual bool canReadMail() const { return false; }


private:
	bool _isRunning;
	QMap< int, KornMailSubject* > *_msgList;
	QString *_name;
	int _counter;

	void eraseList( void );

private slots:
	void doReadSubjectsASync( void );

public slots: //accessed by DBUSDropInterface
	/**
	 * This function append a message to the box.
	 * This function is called from DBUSDropInterface.
	 *
	 * @param subject the subject of the message
	 * @param message the content of the message
	 * @return the message id
	 */
	int addMessage( const QString& subject, const QString& message );
	/**
	 * This function removes a messages from the box.
	 * This function is called from DBUSDropInterface.
	 *
	 * @param id the message id to be removed
	 * @return true if succesfull, false otherwise
	 */
	bool removeMessage( int id );

	//accessed by DBUSDropCfg
public:
	/**
	 * This function returns the dbus name.
	 * This function is accessed by DBUSDropCfg.
	 *
	 * @return the dbus name
	 */
	QString DBUSName() const;
	/**
	 * This function sets the dbus name.
	 * This function is accessed by DBUSDropCfg.
	 *
	 * @param name the new dbus name
	 */
	void setDBUSName( const QString& name );
};

#endif //DBUSDROP_H
