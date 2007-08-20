/* addressWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone <dan@kpilot.org>
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
**
** This file defines the address-viewing widget used in KPilot
** to display the Pilot's address records.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#ifndef _KPILOT_ADDRESSWIDGET_H
#define _KPILOT_ADDRESSWIDGET_H

#include <Q3PtrList>

class Q3ListBox;
class Q3TextView;

class KComboBox;

class PilotDatabase;


#include "pilotComponent.h"
#include "pilotAddress.h"

class AddressWidget : public PilotComponent
{
Q_OBJECT

public:
	AddressWidget(QWidget* parent,const QString& dbpath);
	~AddressWidget();

	// Pilot Component Methods:
	virtual void showComponent();
	virtual void hideComponent();
	virtual bool preHotSync(QString &);
	virtual void postHotSync();

public slots:
	/**
	* Called when a particular address is selected. This slot displays
	* it in the viewer widget.
	*/
	void slotShowAddress(int);
	void slotExport();

	void slotUpdateButtons();	// Enable/disable buttons

signals:
	void recordChanged(PilotAddress *);

protected slots:

	/**
	* Change category. This means that the display should be
	* cleared and that the list should be repopulated.
	*/
	void slotSetCategory(int);

private:
	void setupWidget();
	void updateWidget(); // Called with the lists have changed..

	/**
	* getAllAddresses reads the database and places all
	* the addresses from the database in the list
	* in memory --- not the list on the screen.
	* @see fAddressList
	*/
	int getAllAddresses(PilotDatabase *addressDB);

	/**
	* Create a sensible "title" for an address, composed
	* of first + last name if possible.
	*/
	QString createTitle(PilotAddress *,int displayMode);

	/**
	* We use a QComboBox fCatList to hold the user-visible names
	* of all the categories. The QTextView fAddrInfo is for
	* displaying the currently selected address, if any.
	* The QListBox fListBox lists all the addresses in the
	* currently selected category.
	*
	* The entire address database is read into memory in the
	* QList fAddressList. We need the appinfo block from the
	* database to determine which categories there are; this
	* is held in fAddressAppInfo.
	*
	* The two buttons should speak for themselves.
	*/
	KComboBox            *fCatList;
	Q3TextView            *fAddrInfo;
	PilotAddressInfo     *fAddressAppInfo;
	Q3PtrList<PilotAddress>   fAddressList;
	Q3ListBox             *fListBox;
	QPushButton	     *fExportButton;

public:
	typedef enum { PhoneNumberLength=16 } Constants ;
};

#endif
