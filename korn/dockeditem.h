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

#ifndef MK_DOCKEDITEM_H
#define MK_DOCKEDITEM_H

/**
 * @file
 *
 * This file contains the class DocketItem.
 */

#include "boxcontaineritem.h"

class SystemTray;

class KConfig;

template< class T > class QList;

/**
 * This class is an implementation of a BoxContainerItem for the
 * systemtray view. If also is a KSystemTray reimplementation.
 *
 * @see BoxContainerItem
 * @author Mart Kelder <mart.kde@hccnet.nl>
 */
class DockedItem : public BoxContainerItem
{ Q_OBJECT
public:
	/**
	 * This contructor gives all it parameters to its parents.
	 *
	 * @param parent the parent window
	 */
	DockedItem( QWidget * parent = 0 );
	/**
	 * Empty destructor; does nothing at the moment
	 */
	~DockedItem();

	/**
	 * This functions shows the element in the systay.
	 */
	virtual void showBox();
	
	/**
	 * This functions reads the config. It used the parent
	 * version for the main things, but it is possible to
	 * add some configurations over here.
	 *
	 * @param config The KConfig-instance which contains the settings of this tray-item.
	 * @param index The index of the box in the configuration file
	 */
	virtual void readConfig( BoxSettings *config, BoxSettings *config_settings, const int index );
	
public slots:	
	/**
	 * This functions sets the number of messages to be displayed, and warns if there are new messages waiting.
	 *
	 * @param count The number of messages waiting
	 * @param newMessages if true than the settings for displaying new messages will be used.
	 */
	virtual void setCount( const int count, const bool newMessages ) ;
	
	/**
	 * This function sets the tooltip @p string to a box.
	 *
	 * @param string The tooltip to be added.
	 */
	virtual void setTooltip( const QString& string );

	/**
	 * This function displays a passive popup containing some headers of a number of messages.
	 *
	 * @param list the list which contains some headers of the new messages
	 * @param total the total number of messages
	 * @param date true is the date should also be printed; false otherwise
	 * @param name the name of the account
	 */
	void slotShowPassivePopup( QList< KornMailSubject >* list, int total, bool date, const QString& name );

	/**
	 * This function displays an error message in a passive popup.
	 *
	 * @param error the error string
	 * @param name the name of the account
	 */
	void slotShowPassivePopup( const QString& error, const QString& name );
	
protected:
	/**
	 * This function popup's the KMenu inmideately.
	 */
	virtual void doPopup();
	
private:
	void setAnimIcon( const QString& anim );
	
private:
	SystemTray *_systemtray;
};

#endif //MK_DOCKEDITEM_H
