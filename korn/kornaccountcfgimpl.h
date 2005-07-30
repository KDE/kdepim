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

#include "kornaccountcfg.h"

class KConfigGroup;
class KURLRequester;

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QString;
class QWidget;

class AccountInput;

template< class T > class QPtrList;
template< class T > class QPtrVector;

class KornAccountCfgImpl : public KornAccountCfg
{ Q_OBJECT
public:
	KornAccountCfgImpl( QWidget * parent = 0, const char * name = 0 );
	~KornAccountCfgImpl();
	
	void readConfig( KConfigGroup *config, QMap< QString, QString > *entries, int boxnr, int accountnr );
	void writeConfig();

public slots:
	void slotSSLChanged();
	
protected slots:
	virtual void slotProtocolChanged( const QString& );
	
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
	QPtrVector< QWidget > *_groupBoxes;
	
	QPtrList< AccountInput > *_accountinput;
};

#endif //MK_KORNACCOUNTCFGIMPL_H
