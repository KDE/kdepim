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

#include "pilotComponent.h"
#include <qmlined.h>
#include <qlistbox.h>
#include <qcombo.h>
#include "pilotAddress.h"
#include "kpilotlink.h"

class KPilotInstaller;

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
      void writeAddress(PilotAddress* which);

	char *createTitle(PilotAddress *,int displayMode);

      QComboBox*            fCatList;
      QMultiLineEdit*       fTextWidget;
      struct AddressAppInfo fAddressAppInfo;
      QList<PilotAddress>   fAddressList;
      unsigned int          fLookupTable[1000];
      QListBox*             fListBox;
    };

#endif
