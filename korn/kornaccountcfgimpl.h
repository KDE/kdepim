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

class TQVBoxLayout;
class TQHBoxLayout;
class TQLabel;
class TQString;
class TQWidget;

class AccountInput;

template< class T > class TQPtrList;
template< class T > class TQPtrVector;

class KornAccountCfgImpl : public KornAccountCfg
{ Q_OBJECT
public:
	KornAccountCfgImpl( TQWidget * parent = 0, const char * name = 0 );
	~KornAccountCfgImpl();
	
	void readConfig( KConfigGroup *config, TQMap< TQString, TQString > *entries, int boxnr, int accountnr );
	void writeConfig();

public slots:
	void slotSSLChanged();
	
protected slots:
	virtual void slotProtocolChanged( const TQString& );
	
private slots:
	void slotOK();
	void slotCancel();
private:
	KConfigGroup *_config;
	int _fields;
	int _urlfields;
	int _boxnr, _accountnr;
		
	TQVBoxLayout *_vlayout;
	TQHBoxLayout *_protocolLayout;
	TQPtrVector< TQWidget > *_groupBoxes;
	
	TQPtrList< AccountInput > *_accountinput;
};

#endif //MK_KORNACCOUNTCFGIMPL_H
