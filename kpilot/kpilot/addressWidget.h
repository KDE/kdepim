/* addressWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/
#ifndef _KPILOT_ADDRESSWIDGET_H
#define _KPILOT_ADDRESSWIDGET_H

class QMultiLineEdit;
class QListBox;
class QComboBox;
class QTextView;

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
	void slotEditRecord();
	void slotCreateNewRecord();
	void slotDeleteRecord();
	void slotEditCancelled();
	void slotExport();

	void slotUpdateButtons();	// Enable/disable buttons

signals:
	void recordChanged(PilotAddress *);

protected slots:
	/**
	* When an edit window is closed, the corresponding record
	* is updated and possibly re-displayed.
	*/
	void slotUpdateRecord(PilotAddress*);

	/**
	* Pop up an edit window for a new record.
	*/
	void slotAddRecord(PilotAddress*);

	/**
	* Change category. This means that the display should be
	* cleared and that the list should be repopulated.
	*/
	void slotSetCategory(int);

private:
	void setupWidget();
	void updateWidget(); // Called with the lists have changed..
	void writeAddress(PilotAddress* which,PilotDatabase *db=0L);

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
	QComboBox            *fCatList;
	QTextView            *fAddrInfo;
	struct AddressAppInfo fAddressAppInfo;
	QPtrList<PilotAddress>   fAddressList;
	QListBox             *fListBox;
	QPushButton	     *fEditButton,*fDeleteButton;

protected:
	/**
	* Keep track of how many open address editing windows there
	* are. You can't sync when there are open windows.
	*/
	int fPendingAddresses;

public:
	typedef enum { PhoneNumberLength=16 } Constants ;
};

#endif
