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
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#ifndef __ADDRESS_WIDGET_H
#define __ADDRESS_WIDGET_H

class QMultiLineEdit;
class QListBox;
class QComboBox;

class KConfig;

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
      void initialize();
      void preHotSync(char*);
      void postHotSync();
      bool saveData();

    public slots:
    void slotShowAddress(int);
      void slotImportAddressList();
      void slotExportAddressList();
      void slotEditRecord(); // Edits the currently selected record.
      void slotCreateNewRecord();
      void slotDeleteRecord(); // Deletes the currently selected record

	void slotUpdateButtons();	// Enable/disable buttons

signals:
	void recordChanged(PilotAddress *);

    protected:
      char* getFieldBySymbol(PilotAddress* rec, const char* symbol);
      void setFieldBySymbol(PilotAddress* rec, const char* symbol, const char* text);
      PilotAddress* findAddress(const char* text, const char* symbol);
 protected slots:
 void slotUpdateRecord(PilotAddress*);
      void slotAddRecord(PilotAddress*);
      void slotSetCategory(int);

private:
      void setupWidget();
      void updateWidget(); // Called with the lists have changed..
      void writeAddress(PilotAddress* which,PilotDatabase *db=0L);

	/**
	* setupCategories extracts the category names
	* from a structure and fills up the list box.
	*/
	void setupCategories();
	/**
	* getAllAddresses reads the database and places all
	* the addresses from the database in the list
	* in memory --- not the list on the screen.
	* @see fAddressList
	*/
	int getAllAddresses(PilotDatabase *addressDB,KConfig& );
	char *createTitle(PilotAddress *,int displayMode);

      QComboBox*            fCatList;
      QMultiLineEdit*       fTextWidget;
      struct AddressAppInfo fAddressAppInfo;
      QList<PilotAddress>   fAddressList;
      unsigned int          fLookupTable[1000];
      QListBox*             fListBox;
	QPushButton	*fEditButton,*fDeleteButton;

public:
	typedef enum { PhoneNumberLength=16 } Constants ;
    };

#endif


// $Log$
// Revision 1.9  2001/02/06 08:05:19  adridg
// Fixed copyright notices, added CVS log, added surrounding #ifdefs. No code changes.
//
