/* *******************************************************
   KPilot - Hot-Sync Software for Unix.
   Copyright 1998 by Dan Pilone
   This code is released under the GNU PUBLIC LICENSE.
   Please see the file 'COPYING' included in the KPilot
   distribution.
   *******************************************************
 */

#ifndef _WIDGET1_H_
#define _WIDGET1_H_

#include <qpushbt.h>
#include <qlined.h>
#include <qlabel.h>
#include <qwidget.h>
#include "pilotAddress.h"

class AddressEditor : public QWidget {
	Q_OBJECT
public:
	AddressEditor(PilotAddress* address, QWidget *parent=NULL, const char *name=NULL);
	~AddressEditor();
public slots:
signals:
	void recordChangeComplete ( PilotAddress* );
protected:
protected slots:
void commitChanges();
  void cancel();
private:
PilotAddress* fAddress;
  bool fDeleteOnCancel;
QLineEdit *fCustom4Field;
  QLineEdit *fCustom3Field;
  QLineEdit *fCustom2Field;
  QLineEdit *fCustom1Field;
  QLineEdit *fCountryField;
  QLineEdit *fZipField;
  QLineEdit *fStateField;
  QLineEdit *fCityField;
  QLineEdit *fAddressField;
  QLineEdit *fPhone5Field;
  QLineEdit *fPhone4Field;
  QLineEdit *fPhone3Field;
  QLineEdit *fPhone2Field;
  QLineEdit *fPhone1Field;
  QLineEdit *fCompanyField;
  QLineEdit *fTitleField;
  QLineEdit *fFirstNameField;
  QLineEdit *fLastNameField;
  QButton *fCancelButton;
  QButton *fOkButton;
  void initLayout(void);
  void fillFields();
private slots:
};
#endif /* _WIDGET1_H_ */
