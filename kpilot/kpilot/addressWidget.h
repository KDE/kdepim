/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef __ADDRESS_WIDGET_H
#define __ADDRESS_WIDGET_H

class QMultiLineEdit;
class QListBox;
class QComboBox;
#include "pilotComponent.h"
#include "pilotAddress.h"
#include "kpilotlink.h"

class KPilotInstaller;
class KConfig;

class AddressWidget : public PilotComponent
    {
    Q_OBJECT

    public:
    AddressWidget(KPilotInstaller* installer, QWidget* parent);
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
	int getAllAddresses(PilotDatabase *addressDB,KConfig *);
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
