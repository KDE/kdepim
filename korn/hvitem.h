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

#ifndef MK_HVITEM_H
#define MK_HVITEM_H

#include <boxcontaineritem.h>

class KActionCollection;
class KPopupMenu;

class Label;

/**
 * Item for a horizontal or vertical displayed window.
 */
class HVItem : public BoxContainerItem
{ Q_OBJECT
public:
	HVItem( QWidget *parent = 0, const char *name = 0 );
	~HVItem();
	
	virtual void showBox();
	
public slots:
	/**
	 * Sets the number of new messages.
	 *
	 * @param count The number of unread messages.
	 * @param newMessages Are there any new messages?
	 */
	void setCount( const int count, const bool newMessages );
	
	/**
	 * This function sets the tooltip @p string to a box.
	 * @param string The tooltip to be added.
	 */
	virtual void setTooltip( const QString& string );
	
	/**
	 * This slot triggered when the passive popup is to be shown. It is transported
	 * to BoxContainerItem, but so it is possible to change the arguments.
	 */
	void slotShowPassivePopup( QPtrList< KornMailSubject >* list, int total, bool date, const QString& );
	
	/**
	 * Trigered if the popup-menu is to be shown
	 */
	virtual void doPopup();
private:
	Label *_label;
	KPopupMenu *_popup;
	KActionCollection *_actions;
};

#endif //MK_HVITEM_H
