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

class DCOPDrop : public KMailDrop
{ Q_OBJECT
public:
	DCOPDrop();
	virtual ~DCOPDrop();
	
	virtual bool valid() { return true; }
	virtual void recheck();
	virtual bool startMonitor();
	virtual bool stopMonitor();
	virtual bool running() { return _isRunning; }
	
	//virtual void addConfigPage( KDropCfgDialog* ) ;
	virtual KMailDrop* clone() const { return new DCOPDrop; }

	virtual bool readConfigGroup( const KConfigBase& );
	virtual bool writeConfigGroup( KConfigBase& ) const;
	virtual QString type() const;
	
	virtual bool synchrone() const { return false; }
	
	virtual bool canReadSubjects() { return true; }
	virtual QValueVector< KornMailSubject >* doReadSubjects( bool *stop );
	
	virtual bool canDeleteMails() { return true; }
	virtual bool deleteMails( QPtrList<const KornMailId> * ids, bool * stop );
	
	virtual bool canReadMail() const { return false; }
	
	
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
