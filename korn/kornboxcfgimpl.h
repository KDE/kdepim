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

#ifndef MK_KORNBOXCFGIMPL_H
#define MK_KORNBOXCFGIMPL_H

#include "kornboxcfg.h"

class KConfig;
class KConfigGroup;
class KDialogBase;

class QFont;
class QString;
class QWidget;

class KornBoxCfgImpl : public KornBoxCfg
{ Q_OBJECT
public:
	KornBoxCfgImpl( QWidget *parent, const char * name );
	~KornBoxCfgImpl();
	
	/**
	 * This method write the current configuration to a specified KConfig-object.
	 *
	 * @param config The config where in which the configuration should be added.
	 * @param index The index of the selected config. This is the group-number.
	 */
	void writeConfig( KConfig * config, const int index );
	
	/**
	 * This method write the current configuration to a specified KConfig-object.
	 *
	 * @param config The config where in which the configuration should be added. Note that this object is stored locally until the object is destroyed.
	 * @param index The index of the selected config. This is the group-number.
	 */
	void readConfig( KConfig * config, const int index );
	
private:
	void readViewConfig();
	void readEventConfig();
	void readAccountsConfig();
	void readDCOPConfig();
	
	void writeViewConfig( KConfig* config );
	void writeEventConfig( KConfig *config );
	void writeAccountsConfig( KConfig *config );
	void writeDCOPConfig( KConfig *config );
	
	KConfig* _config;
	KConfigGroup *_group;
	KDialogBase *_base;
	int _index;
	QString *_anims[ 2 ];
	QFont *_fonts[ 2 ];
	
protected slots:
	virtual void slotEditBox();
	virtual void slotActivated( const QString& );
	virtual void slotActivated( const int );
	virtual void slotSetDefaults( const QString&, const int, KConfig* );
	virtual void slotChangeNormalAnim();
	virtual void slotChangeNewAnim();
	virtual void slotChangeNormalFont();
	virtual void slotChangeNewFont();
	virtual void slotNormalAnimToggled( bool enabled );
	virtual void slotNewAnimToggled( bool enabled );
	
private slots:
	void slotOK();
	void slotCancel();
	void slotDialogDestroyed();
};

#endif //MK_KORNBOXCFGIMPL_H
