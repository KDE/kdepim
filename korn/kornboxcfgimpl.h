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

#ifndef MK_KORNBOXCFGIMPL_H
#define MK_KORNBOXCFGIMPL_H

/**
 * @file
 *
 * This file the class KornBoxCfgImpl
 */

#include <QWidget>
#include "ui_kornboxcfg.h"

class BoxSettings;
class Settings;

class KConfig;
class KDialog;

class QFont;
class QString;
class QModelIndex;
class QWidget;

/**
 * This file is the implementation of the KornBoxCfg class, which is created by uic.
 * It contains the configuration for the boxes.
 */
class KornBoxCfgImpl : public QWidget, public Ui_KornBoxCfg
{ Q_OBJECT
public:
	/**
	 * Constructor
	 *
	 * @param parent the parent of this object, usually of type KDialog*
	 */
	KornBoxCfgImpl( QWidget *parent, Settings *glob_settings, BoxSettings *settings );
	/**
	 * Destructor
	 */
	~KornBoxCfgImpl();
	
	/**
	 * This method write the current configuration to a specified KConfig-object.
	 *
	 * @param config The config where in which the configuration should be added.
	 * @param index The index of the selected config. This is the group-number.
	 */
	void writeConfig();
	
	/**
	 * This method write the current configuration to a specified KConfig-object.
	 *
	 * @param config The config where in which the configuration should be added.
	 * 		Note that this object is stored locally until the object is destroyed.
	 * @param index The index of the selected config. This is the group-number.
	 */
	void readConfig( KConfig * config, const int index );
	
private:
	void readViewConfig();
	void readEventConfig();
	void readAccountsConfig();
	
	void writeViewConfig();
	void writeEventConfig();
	
	Settings *m_glob_settings;
	BoxSettings *m_settings;
	KDialog *m_base;
	int m_index;
	QString *m_anims[ 2 ];
	QFont *m_fonts[ 2 ];
	
private slots:
	void slotEditBox();
	void slotActivated( const QModelIndex& );
	void slotChangeNormalAnim();
	void slotChangeNewAnim();
	void slotChangeNormalFont();
	void slotChangeNewFont();
	void slotNormalAnimToggled( bool enabled );
	void slotNewAnimToggled( bool enabled );
	
	void slotOK();
	void slotCancel();
	void slotDialogDestroyed();
	void slotAddAccount();
	void slotRemoveAccount();
	void slotMoveUp();
	void slotMoveDown();
};

#endif //MK_KORNBOXCFGIMPL_H
