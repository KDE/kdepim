/* addressWidget.h			KPilot
**
** Copyright (C) 1998-2001 by Dan Pilone
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
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
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

class KConfig;

class PilotDatabase;



#ifndef _KPILOT_PILOTCOMPONENT_H
#include "pilotComponent.h"
#endif

#ifndef _KPILOT_PILOTADDRESS_H
#include "pilotAddress.h"
#endif

class AddressWidget : public PilotComponent
{
Q_OBJECT

public:
	AddressWidget(QWidget* parent,const QString& dbpath);
	~AddressWidget();

	// Pilot Component Methods:
	void initialize();
	void preHotSync(char*);
	void postHotSync();

public slots:
	/**
	* Called when a particular address is selected. This slot displays
	* it in the viewer widget.
	*/
	void slotShowAddress(int);
	void slotEditRecord();
	void slotCreateNewRecord();
	void slotDeleteRecord();

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
	char *createTitle(PilotAddress *,int displayMode);

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
	QList<PilotAddress>   fAddressList;
	QListBox             *fListBox;
	QPushButton	     *fEditButton,*fDeleteButton;

public:
	typedef enum { PhoneNumberLength=16 } Constants ;
};

#else
#ifdef DEBUG
#warning "File doubly included"
#endif
#endif


// $Log$
// Revision 1.19  2001/09/23 21:44:56  adridg
// Myriad small changes
//
// Revision 1.18  2001/09/23 18:30:15  adridg
// Adjusted widget for new config
//
// Revision 1.17  2001/09/06 22:33:43  adridg
// Cruft cleanup
//
// Revision 1.16  2001/08/27 22:51:41  adridg
// MartinJ's beautification of the address viewer
//
// Revision 1.15  2001/04/16 13:48:35  adridg
// --enable-final cleanup and #warning reduction
//
// Revision 1.14  2001/03/24 15:59:22  adridg
// Some populateCategories changes for bug #22112
//
// Revision 1.13  2001/03/19 23:12:39  stern
// Made changes necessary for upcoming abbrowser conduit.
//
// Mainly, I added two public methods to PilotAddress that allow for easier
// setting and getting of phone fields.
//
// I also have added some documentation throughout as I have tried to figure
// out how everything works.
//
// Revision 1.12  2001/03/09 09:46:15  adridg
// Large-scale #include cleanup
//
// Revision 1.11  2001/03/04 11:22:12  adridg
// In response to bug 21392, replaced fixed-length lookup table by a subclass
// of QListBoxItem inserted into list box. This subclass carries data to
// lookup the relevant pilot record.
//
// Revision 1.10  2001/02/24 14:08:13  adridg
// Massive code cleanup, split KPilotLink
//
// Revision 1.9  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
