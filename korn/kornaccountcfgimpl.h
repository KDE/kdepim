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

#ifndef MK_KORNACCOUNTCFGIMPL_H
#define MK_KORNACCOUNTCFGIMPL_H

#include "kornaccountcfg.h"

class KConfigGroup;
class KURLRequester;

class QLabel;
class QString;
class QWidget;

class KornAccountCfgImpl : public KornAccountCfg
{ Q_OBJECT
public:
	KornAccountCfgImpl( QWidget * parent = 0, const char * name = 0 );
	~KornAccountCfgImpl();
	
	void readConfig( KConfigGroup *config );
	void writeConfig();
	
protected slots:
	virtual void slotProtocolChanged( const QString& );
	
private slots:
	void slotOK();
	void slotCancel();
	
private:
	KConfigGroup *_config;
	int _fields;
	int _urlfields;
		
	void showHide( int fieldvalue, QLabel *label, QWidget* edit, KURLRequester* url, const QString& labelText );
	
	QString encrypt( const QString& ) const;
	QString decrypt( const QString& ) const;
};

#endif //MK_KORNACCOUNTCFGIMPL_H
