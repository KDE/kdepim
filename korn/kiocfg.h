/*
 * Copyright (C)       Kurt Granroth
 * Copyright (C) 2004, Mart Kelder (mart.kde@hccnet.nl)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
* kiocfg.h -- Declaration of class KKIOCfg.
*/
#ifndef KEG_KIOCFG_H
#define KEG_KIOCFG_H

#include<qobject.h>

#include "moncfg.h"

class QLineEdit;
class KKioDrop;
class QLabel;
class QCheckBox;
class QComboBox;
template<class> class QPtrList;
class KIO_Protocol;

/**
* Configuration manager for @ref KImap4Drop monitors.
* @author Kurt Granroth (granroth@kde.org)
* @version $Id$
* Copyed and edited for kio by Mart Kelder
*/
class KKioCfg : public KMonitorCfg
{
Q_OBJECT
public:
	/**
	* KImapCfg Constructor
	*/
	KKioCfg( KKioDrop *drop );

	/**
	* KImapCfg Destructor
	*/
	virtual ~KKioCfg();
	
	virtual QString name() const;
	virtual QWidget *makeWidget( QWidget *parent );
	virtual void updateConfig();

	void addProtocol( KIO_Protocol * proto );	

private:
	KKioCfg& operator=( KKioCfg& );
	KKioCfg( const KKioCfg& );
	bool setComboItem( const QString & item );

	QPtrList<KIO_Protocol> *_protocols;

	QLabel *_serverLabel;
	QLabel *_portLabel;
	QLabel *_mailboxLabel;
	QLabel *_userLabel;
	QLabel *_pwdLabel;
	QLabel *_authLabel;
	
	QComboBox *_protoCombo;
	QLineEdit *_serverEdit;
	QLineEdit *_portEdit;
	QLineEdit *_mailboxEdit;
	QLineEdit *_userEdit;
	QLineEdit *_pwdEdit;
	QCheckBox *_savePass;
	QComboBox *_authCombo;

	KIO_Protocol *_this_protocol;
private slots:
	void protoChange( int );
};

#endif // KEG_KIOCFG_H
