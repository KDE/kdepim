/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef IMPORTWIZARD_H
#define IMPORTWIZARD_H

#include "pimimportabstract.h"

#include <kapplication.h>
#include <KAssistantDialog>

class KPageWidgetItem;
class SelectProgramPage;
class SelectComponentPage;
class ImportMailPage;
class ImportFilterPage;
class ImportSettingPage;
class ImportAddressbookPage;
class ImportFinishPage;
class PimImportAbstract;

class ImportWizard : public KAssistantDialog {
  Q_OBJECT
public:
  ImportWizard( QWidget* parent=0);
  ~ImportWizard();

  void next();
  void reject();
  
  ImportMailPage* importMailPage();
  ImportFilterPage* importFilterPage();
  ImportAddressbookPage *importAddressBookPage();
  ImportSettingPage *importSettingPage();
  ImportFinishPage *importFinishPage();
private slots:
  void help();
  void slotProgramSelected(const QString& program);
  void slotImportMailsClicked();
  void slotImportFiltersClicked();
  void slotProgramDoubleClicked();
  void slotAtLeastOneComponentSelected( bool b );

private:
  void addImportModule(PimImportAbstract *);
  void checkModules();
  void setAppropriatePage(PimImportAbstract::TypeSupportedOptions options);

  QMap<QString, PimImportAbstract*> mlistImport;

  PimImportAbstract *mSelectedPim;
  
  KPageWidgetItem *mPage1;
  KPageWidgetItem *mPage2;
  KPageWidgetItem *mPage3;
  KPageWidgetItem *mPage4;
  KPageWidgetItem *mPage5;
  KPageWidgetItem *mPage6;
  KPageWidgetItem *mPage7;

  SelectProgramPage *mSelectProgramPage;
  SelectComponentPage *mSelectComponentPage;
  ImportMailPage *mImportMailPage;
  ImportFilterPage *mImportFilterPage;
  ImportSettingPage *mImportSettingPage;
  ImportAddressbookPage *mImportAddressbookPage;
  ImportFinishPage *mImportFinishPage;
};

#endif /* IMPORTWIZARD_H */

