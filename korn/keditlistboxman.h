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

#ifndef MK_KEDITLISTBOXMAN_H
#define MK_KEDITLISTBOXMAN_H

#include <keditlistbox.h>

class KConfig;

class QString;
class QWidget;

/**
 * This class is a extention on KEditListBox.
 * It also handles the configuration.
 * @author Mart Kelder (mart.kde@hccnet.nl)
 */
class KEditListBoxManager : public KEditListBox
{ Q_OBJECT
public:
	//TODO: remove the argument
	/**
	 * Constructor: @see KEditListBox::KEditListBox( const QString&, QWidget, const char * name, bool, int )
	 *
	 * @param parent the parent of this object
	 * @param name argument to be removed in future
	 * @param checkAtEntering the characters are checked when the name is entered
	 * @param buttons A list of buttons in the KEditListBox
	 */
	KEditListBoxManager(	QWidget *parent = 0, const char *name = 0,
			bool checkAtEntering=true, int buttons = All );
			
	/**
	 * The save as above, but with other options for KEditListBox.
	 *
	 * @param title title for the listbox
	 * @param parent the parent of this object
	 * @param name argument to be removed in future
	 * @param checkAtEntering the characters are checked when the name is entered
	 * @param buttons A list of buttons in the KEditListBox
	 */
	KEditListBoxManager(	const QString& title, QWidget *parent = 0,
			const char *name = 0, bool checkAtEntering=true,
			int buttons = All );
		
	/**
	 * The save as above, but with other options for KEditListBox.
	 */
	KEditListBoxManager(	const QString& title,
			const KEditListBox::CustomEditor &customEditor,
			QWidget *parent = 0, const char *name = 0,
			bool checkAtEntering = true, int buttons = All );
	
	/**
	 * Destructor
	 */		
	~KEditListBoxManager();
	
	
	/** 
	 * Set the KConfig object (required before doing something
	 * @param config The KConfig-object
	 */
	void setConfig( KConfig* config );
	/**
	 * Sets the groupName. groupName should contain at least one %1.
	 * It is used for makeing groupnames.
	 * @param name The groupname
	 */
	void setGroupName( const QString& name );
	
	/**
	 * Sets the subGroupName. subGroupName should contain %1 and %2.
	 * It is used to execute operations on a group and its subgroups.
	 * @param name The groupname
	 */
	void setSubGroupName( const QString& name );
private:
	/** 
	 * This functions is called from and only from the constructor to prevent writing the
	 * same code for all constructors
	 */
	void init();
	
	/**
	 * This function reads the names out the config.
	 */
	void readNames();
private:
	KConfig *_config; //Stores the KConfig-object
	QString *_groupName; //Stores the groupName string.
	QString *_subGroupName;
	int _prevCount;

private slots:
	//These comes directly from the KEditListBox itselfs.
	void slotChanged();
	void slotAdded( const QString& );
	void slotRemoved( const QString& );
	
	void slotActivated( Q3ListBoxItem* );
private:
	/**
	 * This private method moves an item. It is called from slotChanged().
	 * @param src The number of the source-group.
	 * @param dest The number of the destanation-group
	 */
	void moveItem( int src, int dest );
	
	/**
	 * This private functions switch to groups: first^=last; last^=first; first^=last
	 * @param first the first number of a group.
	 * @param last the second number of a group (and the last number).
	 */
	void changeItem( int first, int last );
	
	/**
	 * This function is called if the user change the name of the group
	 */
	void changedText();
	
signals:
	/**
	 * This signal is emitted when somebody selects an item
	 * @param text The text of the newly selected item.
	 */
	void activated( const QString& text );
	
	/**
	 * This signal is emitted when defaults have to be set.
	 * @param name The name of the object: this is filled in the KEditListBox;
	 * @param config The configuration in which the config have to be parsed.
	 * @param index The number of the item.
	 * this config is already in the right group.
	 */
	void setDefaults( const QString& name, const int index, KConfig* config );

	/**
	 * This signal is emitted when two elements are swapped.
	 *
	 * @param elem1 the index of one of the elements which is swapped
	 * @param elem2 the index of the other element which is swapped
	 */
	void elementsSwapped( int elem1, int elem2 );
	/**
	 * This signal is emitted when an element is deleted
	 *
	 * @param elem the index of the element which is deleted
	 */
	void elementDeleted( int elem );
	
};

#endif //MK_KEDITLISTBOXMAN_H
