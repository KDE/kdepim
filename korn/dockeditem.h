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

#ifndef MK_DOCKEDITEM_H
#define MK_DOCKEDITEM_H

#include "boxcontaineritem.h"

class SystemTray;

class KConfig;

class QPixmap;

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
	 * @param parant The parent window
	 * @param name The name of the QObject's parents.
	 */
	DockedItem( QWidget * parent = 0, const char * name = 0 );
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
	 * @param config The KConfig-instance which contains the settings of this tray-item.
	 * @param index The index of the box in the configuration file
	 */
	virtual void readConfig( KConfig* config, const int index );
	
public slots:	
	/**
	 * This functions sets the number of messages to be displayed, and warns if there are new messages waiting.
	 * @param count The number of messages waiting
	 * @param newMessages if true than the settings for displaying new messages will be used.
	 */
	virtual void setCount( const int count, const bool newMessages ) ;
	
	/**
	 * This function sets the tooltip @p string to a box.
	 * @param string The tooltip to be added.
	 */
	virtual void setTooltip( const QString& string );
	
	void slotShowPassivePopup( QPtrList< KornMailSubject >* list, int total, bool date, const QString& );
	
protected:
	/**
	 * This function popup's the KPopupMenu inmideately.
	 */
	virtual void doPopup();
	
private:
	void setAnimIcon( const QString& anim );
	
private:
	SystemTray *_systemtray;
};

#endif //MK_DOCKEDITEM_H
