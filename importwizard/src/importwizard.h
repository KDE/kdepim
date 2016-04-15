/*
  Copyright (c) 2012-2016 Montel Laurent <montel@kde.org>

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

#include "abstractimporter.h"

#include <KAssistantDialog>

class KPageWidgetItem;
class SelectProgramPage;
class SelectComponentPage;
class ImportMailPage;
class ImportFilterPage;
class ImportSettingPage;
class ImportAddressbookPage;
class ImportFinishPage;
class ImportCalendarPage;
class AbstractImporter;
class ManualSelectFilterPage;
class ManualImportMailPage;

namespace Akonadi
{
class Collection;
}
class ImportWizard : public KAssistantDialog
{
    Q_OBJECT
public:
    enum WizardMode {
        AutoDetect = 0,
        Manual = 1
    };

    explicit ImportWizard(WizardMode mode, QWidget *parent = Q_NULLPTR);
    ~ImportWizard();

    void next() Q_DECL_OVERRIDE;
    void reject() Q_DECL_OVERRIDE;
    void back() Q_DECL_OVERRIDE;

    ImportMailPage *importMailPage() const;
    ImportFilterPage *importFilterPage() const;
    ImportAddressbookPage *importAddressBookPage() const;
    ImportSettingPage *importSettingPage() const;
    ImportFinishPage *importFinishPage() const;
    ImportCalendarPage *importCalendarPage() const;

    void addFinishInfo(const QString &log);
    void addFinishError(const QString &log);

private Q_SLOTS:
    void slotProgramSelected(const QString &program);
    void slotImportMailsClicked();
    void slotImportFiltersClicked();
    void slotProgramDoubleClicked();
    void slotAtLeastOneComponentSelected(bool b);
    void slotImportSettingsClicked();
    void slotImportAddressbookClicked();
    void slotImportCalendarClicked();
    void slotSelectManualSelectionChanged(bool b);
    void slotCollectionChanged(const Akonadi::Collection &selectedCollection);

private:
    void readConfig();
    void writeConfig();
    void initializeImportModule();
    void createAutomaticModePage();
    void addImportModule(AbstractImporter *);
    void checkModules();
    void setAppropriatePage(AbstractImporter::TypeSupportedOptions options);
    void enableAllImportButton();
    void createManualModePage();
    void updatePagesFromMode();

    QMap<QString, AbstractImporter *> mlistImport;

    WizardMode mMode;
    AbstractImporter *mSelectedPim;

    KPageWidgetItem *mSelectProgramPageItem;
    KPageWidgetItem *mSelectComponentPageItem;
    KPageWidgetItem *mImportMailPageItem;
    KPageWidgetItem *mImportFilterPageItem;
    KPageWidgetItem *mImportSettingPageItem;
    KPageWidgetItem *mImportAddressbookPageItem;
    KPageWidgetItem *mImportCalendarPageItem;
    KPageWidgetItem *mImportFinishPageItem;

    SelectProgramPage *mSelectProgramPage;
    SelectComponentPage *mSelectComponentPage;
    ImportMailPage *mImportMailPage;
    ImportFilterPage *mImportFilterPage;
    ImportSettingPage *mImportSettingPage;
    ImportAddressbookPage *mImportAddressbookPage;
    ImportFinishPage *mImportFinishPage;
    ImportCalendarPage *mImportCalendarPage;

    ManualSelectFilterPage *mSelfilterpage;
    ManualImportMailPage *mImportpage;
    KPageWidgetItem *mSelfilterpageItem;
    KPageWidgetItem *mImportpageItem;
};

#endif /* IMPORTWIZARD_H */

