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

#ifndef MK_BOXCONTAINER_H
#define MK_BOXCONTAINER_H

#include <qobject.h>

class BoxContainerItem;

template< class T > class QPtrList;

class KConfig;

/**
 * This class is the base for all BoxContainers. A BoxContainer is a place
 * where BoxContainerItems can be placed. BoxContainerItems are the boxes you see.
 */
class BoxContainer : public QObject
{ Q_OBJECT
public:
	/**
	 * Constructor: everything is passed to QObject.
	 *
	 * @param parent The parent of this object
	 * @param name The name of this object
	 */
	BoxContainer( QObject *parent = 0, const char * name = 0 );
	~BoxContainer();
	
	/**
	 * This method reads the config from a certain KConfig instance.
	 *
	 * @param config The KConfig-instance to read the config from.
	 */
	void readConfig( KConfig* config );

	/**
	 * This method writes the config to a certain KConfig instance.
	 *
	 * @param config The KConfig-instance to write the config to.
	 */
	void writeConfig( KConfig *config );
	
	/**
	 * Shows all childs and itself
	 */
	virtual void showBox();
public slots:
	/**
	 * This slot is triggered if the configuration window has to be shown.
	 * This call is passed through.
	 */
	void slotShowConfiguration();

protected:
	/**
	 * This methos adds a child to the list.
	 *
	 * @param item The item to be added.
	 */
	virtual void addItem( BoxContainerItem* item );
	
	/**
	 * This method creates a new BoxContainerItem instance of the same
	 * type as the BoxContainer.
	 *
	 * @return A new instance to a BoxContainerItem of the same type.
	 */
	virtual BoxContainerItem* newBoxInstance() const = 0;
	
	/**
	 * The list of BoxContainerItems.
	 */
	QPtrList< BoxContainerItem > *_items;
	
signals:
	/**
	 * This signal is used to pass the slotShowConfiguration call through
	 */
	void showConfiguration();
};

#endif //MK_BOXCONTAINER_H

