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

#include <kapplication.h>
#include <KAssistantDialog>

class KPageWidgetItem;
class CheckProgramPage;
class SelectComponentPage;
class ImportMailPage;
class ImportFilterPage;
class ImportSettingPage;
class ImportAddressbookPage;
class PimImportAbstract;

class ImportWizard : public KAssistantDialog {
  Q_OBJECT
public:
  ImportWizard( QWidget* parent=0);
  ~ImportWizard();

  void next();
  void reject();

public slots:
  void help();

private:
  void addImportModule(PimImportAbstract *);
  void checkModules();

  QMap<QString, PimImportAbstract*> mlistImport;

  KPageWidgetItem *mPage1;
  KPageWidgetItem *mPage2;
  KPageWidgetItem *mPage3;
  KPageWidgetItem *mPage4;
  KPageWidgetItem *mPage5;
  KPageWidgetItem *mPage6;

  CheckProgramPage *mCheckProgramPage;
  SelectComponentPage *mSelectComponentPage;
  ImportMailPage *mImportMailPage;
  ImportFilterPage *mImportFilterPage;
  ImportSettingPage *mImportSettingPage;
  ImportAddressbookPage *mImportAddressbookPage;
};

#endif /* IMPORTWIZARD_H */

