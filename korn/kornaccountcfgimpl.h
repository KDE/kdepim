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

#ifndef MK_KORNACCOUNTCFGIMPL_H
#define MK_KORNACCOUNTCFGIMPL_H

/**
 * @file
 *
 * This file contains the class KornAccountCfgImpl
 */

#include "kornaccountcfg.h"
#include <QWidget>
//Added by qt3to4:
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class KConfigGroup;
class KURLRequester;

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QString;
class QWidget;

class AccountInput;

template< class T > class QList;
template< class T > class QVector;

/**
 * This class contains the configuration for the accounts.
 * The class is construated from a ui-file and the Protocol* classes.
 */
class KornAccountCfgImpl : public QWidget, public Ui_KornAccountCfg
{ Q_OBJECT
public:
	/**
	 * Constructor
	 *
	 * @param parent the parent of this object
	 */
	KornAccountCfgImpl( QWidget * parent = 0 );
	/**
	 * Destructor
	 */
	~KornAccountCfgImpl();
	
	/**
	 * This function is called to read the config from a configuration file.
	 * The boxnr and accountnr is used to read and save the password
	 *
	 * @param config the configuration group
	 * @param entries the configuration entries
	 * @param boxnr the number of this box
	 * @param accountnr the number of this account
	 */
	void readConfig( KConfigGroup *config, QMap< QString, QString > *entries, int boxnr, int accountnr );
	/**
	 * This function writes the configuration to the configuration group.
	 * The configuration group is stored after reading.
	 */
	void writeConfig();

public slots:
	/**
	 * This slot should be called if the ssl-function changes.
	 * This function finds out what the new value is, and change the port-value if neccesairy.
	 */
	void slotSSLChanged();

protected slots:
	/**
	 * This function should be called if the protocol changes.
	 *
	 * @param protocol the name of the protocol as is selected
	 */
	virtual void slotProtocolChanged( const QString& protocol );
	
private slots:
	void slotOK();
	void slotCancel();
private:
	KConfigGroup *_config;
	int _fields;
	int _urlfields;
	int _boxnr, _accountnr;
		
	QVBoxLayout *_vlayout;
	QHBoxLayout *_protocolLayout;
	QVector< QWidget* > *_groupBoxes;
	
	QList< AccountInput* > *_accountinput;
};

#endif //MK_KORNACCOUNTCFGIMPL_H
