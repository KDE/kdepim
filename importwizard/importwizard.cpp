/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "importwizard.h"
#include "importwizardkernel.h"
#include "selectprogrampage.h"
#include "selectcomponentpage.h"
#include "importmailpage.h"
#include "importfilterpage.h"
#include "importsettingpage.h"
#include "importaddressbookpage.h"
#include "importcalendarpage.h"
#include "importfinishpage.h"

#include "autodetect/thunderbird/thunderbirdimportdata.h"
#include "autodetect/sylpheed/sylpheedimportdata.h"
#include "autodetect/evolutionv3/evolutionv3importdata.h"
#include "autodetect/evolutionv2/evolutionv2importdata.h"
#include "autodetect/evolutionv1/evolutionv1importdata.h"
#include "autodetect/icedove/icedoveimportdata.h"
#include "autodetect/opera/operaimportdata.h"
#include "autodetect/oe/oeimportdata.h"
#include "autodetect/mailapp/mailappimportdata.h"
#include "autodetect/pmail/pmailimportdata.h"
#include "autodetect/thebat/thebatimportdata.h"
#include "autodetect/balsa/balsaimportdata.h"
#include "autodetect/claws-mail/clawsmailimportdata.h"
#include "autodetect/trojita/trojitaimportdata.h"

#include "manual/manualimportmailpage.h"
#include "manual/manualselectfilterpage.h"
#include "manual/importwizardfilterinfogui.h"

#include "mailimporter/filterinfo.h"

#include <QAction>
#include <KAboutData>
#include <KLocalizedString>
#include "importwizard_debug.h"
#include <KMessageBox>
#include <KHelpMenu>
#include <AkonadiWidgets/controlgui.h>
#include <mailcommon/kernel/mailkernel.h>
#include <QPushButton>

ImportWizard::ImportWizard(WizardMode mode, QWidget *parent)
    : KAssistantDialog(parent),
      mMode(mode),
      mSelectedPim(0)
{
    setModal(true);
    setWindowTitle(i18n("PIM Import Tool"));
    setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Help);

    ImportWizardKernel *kernel = new ImportWizardKernel(this);
    CommonKernel->registerKernelIf(kernel);   //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf(kernel);   //SettingsIf is used in FolderTreeWidget
    createAutomaticModePage();
    createManualModePage();
    initializeImportModule();

    // Disable the 'next button to begin with.
    setValid(currentPage(), false);

    connect(mSelectProgramPage, &SelectProgramPage::programSelected, this, &ImportWizard::slotProgramSelected);
    connect(mSelectProgramPage, &SelectProgramPage::doubleClicked, this, &ImportWizard::slotProgramDoubleClicked);
    connect(mImportMailPage, &ImportMailPage::importMailsClicked, this, &ImportWizard::slotImportMailsClicked);
    connect(mImportFilterPage, &ImportFilterPage::importFiltersClicked, this, &ImportWizard::slotImportFiltersClicked);
    connect(mImportSettingPage, &ImportSettingPage::importSettingsClicked, this, &ImportWizard::slotImportSettingsClicked);
    connect(mImportAddressbookPage, &ImportAddressbookPage::importAddressbookClicked, this, &ImportWizard::slotImportAddressbookClicked);
    connect(mImportCalendarPage, &ImportCalendarPage::importCalendarClicked, this, &ImportWizard::slotImportCalendarClicked);

    connect(mSelectComponentPage, &SelectComponentPage::atLeastOneComponentSelected, this, &ImportWizard::slotAtLeastOneComponentSelected);

    resize(640, 480);
    Akonadi::ControlGui::widgetNeedsAkonadi(this);

    checkModules();
    KMessageBox::information(this, i18n("Close KMail before importing data. Some plugins will modify KMail config file."));
    KHelpMenu *helpMenu = new KHelpMenu(this, KAboutData::applicationData(), true);
    //Initialize menu
    QMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(QIcon::fromTheme(QStringLiteral("kmail")));
    button(QDialogButtonBox::Help)->setMenu(menu);
    updatePagesFromMode();
    readConfig();
}

ImportWizard::~ImportWizard()
{
    qDeleteAll(mlistImport);
    writeConfig();
}

void ImportWizard::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "FolderSelectionDialog");
    if (group.hasKey("LastSelectedFolder")) {
        mSelfilterpage->widget()->mCollectionRequestor->setCollection(CommonKernel->collectionFromId(group.readEntry("LastSelectedFolder", -1)));
    }
}

void ImportWizard::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "FolderSelectionDialog");
    group.writeEntry("LastSelectedFolder", mSelfilterpage->widget()->mCollectionRequestor->collection().id());
    group.sync();
}

void ImportWizard::updatePagesFromMode()
{
    const bool autodetectMode = (mMode == AutoDetect);
    setAppropriate(mSelectProgramPageItem, autodetectMode);
    setAppropriate(mSelectComponentPageItem, autodetectMode);
    setAppropriate(mImportMailPageItem, autodetectMode);
    setAppropriate(mImportFilterPageItem, autodetectMode);
    setAppropriate(mImportSettingPageItem, autodetectMode);
    setAppropriate(mImportAddressbookPageItem, autodetectMode);
    setAppropriate(mImportCalendarPageItem, autodetectMode);
    setAppropriate(mImportFinishPageItem, autodetectMode);

    setAppropriate(mSelfilterpageItem, !autodetectMode);
    setAppropriate(mImportpageItem, !autodetectMode);
    if (autodetectMode) {
        setCurrentPage(mSelectProgramPageItem);
    } else {
        setCurrentPage(mSelfilterpageItem);
    }
}

void ImportWizard::createManualModePage()
{
    mSelfilterpage = new ManualSelectFilterPage(this);
    mSelfilterpageItem = new KPageWidgetItem(mSelfilterpage, i18n("Step 1: Select Filter"));

    addPage(mSelfilterpageItem);

    mImportpage = new ManualImportMailPage(this);
    mImportpageItem = new KPageWidgetItem(mImportpage, i18n("Step 2: Importing..."));
    addPage(mImportpageItem);

    // Disable the 'next button to begin with.
    setValid(mSelfilterpageItem, false);

    connect(mSelfilterpage->widget()->mCollectionRequestor, &MailCommon::FolderRequester::folderChanged, this, &ImportWizard::slotCollectionChanged);
}

void ImportWizard::slotCollectionChanged(const Akonadi::Collection &selectedCollection)
{
    if (selectedCollection.isValid()) {
        setValid(mSelfilterpageItem, true);
    } else {
        setValid(mSelfilterpageItem, false);
    }
}

void ImportWizard::reject()
{
    if (currentPage() == mImportpageItem) {
        MailImporter::FilterInfo::terminateASAP();    // ie. import in progress
    }
    KAssistantDialog::reject();
}

void ImportWizard::createAutomaticModePage()
{
    mSelectProgramPage = new SelectProgramPage(this);
    mSelectProgramPageItem = new KPageWidgetItem(mSelectProgramPage, i18n("Detect program"));
    connect(mSelectProgramPage, &SelectProgramPage::selectManualSelectionChanged, this, &ImportWizard::slotSelectManualSelectionChanged);
    addPage(mSelectProgramPageItem);

    mSelectComponentPage = new SelectComponentPage(this);
    mSelectComponentPageItem = new KPageWidgetItem(mSelectComponentPage, i18n("Select material to import"));
    addPage(mSelectComponentPageItem);

    mImportMailPage = new ImportMailPage(this);
    mImportMailPageItem = new KPageWidgetItem(mImportMailPage, i18n("Import mail messages"));
    addPage(mImportMailPageItem);

    mImportFilterPage = new ImportFilterPage(this);
    mImportFilterPageItem = new KPageWidgetItem(mImportFilterPage, i18n("Import mail filters"));
    addPage(mImportFilterPageItem);

    mImportSettingPage = new ImportSettingPage(this);
    mImportSettingPageItem = new KPageWidgetItem(mImportSettingPage, i18n("Import settings"));
    addPage(mImportSettingPageItem);

    mImportAddressbookPage = new ImportAddressbookPage(this);
    mImportAddressbookPageItem = new KPageWidgetItem(mImportAddressbookPage, i18n("Import addressbooks"));
    addPage(mImportAddressbookPageItem);

    mImportCalendarPage = new ImportCalendarPage(this);
    mImportCalendarPageItem = new KPageWidgetItem(mImportCalendarPage, i18n("Import calendars"));
    addPage(mImportCalendarPageItem);

    mImportFinishPage = new ImportFinishPage(this);
    mImportFinishPageItem = new KPageWidgetItem(mImportFinishPage, i18n("Finish"));
    addPage(mImportFinishPageItem);
}

void ImportWizard::initializeImportModule()
{
    //Import module
    addImportModule(new ThunderbirdImportData(this));
    addImportModule(new IcedoveImportData(this));
    addImportModule(new SylpheedImportData(this));
    addImportModule(new Evolutionv3ImportData(this));
    addImportModule(new Evolutionv2ImportData(this));
    addImportModule(new Evolutionv1ImportData(this));
    addImportModule(new OperaImportData(this));
#ifdef Q_OS_WIN
    addImportModule(new OeImportData(this));
#endif
#ifdef Q_OS_MAC
    addImportModule(new MailAppImportData(this));
#endif

#ifdef Q_OS_WIN
    addImportModule(new PMailImportData(this));
    addImportModule(new TheBatImportData(this));
#endif

    addImportModule(new BalsaImportData(this));
    addImportModule(new ClawsMailImportData(this));
    addImportModule(new TrojitaImportData(this));
}

void ImportWizard::slotProgramDoubleClicked()
{
    next();
}

void ImportWizard::slotImportAddressbookClicked()
{
    addFinishInfo(i18n("Import addressbook from %1...", mSelectedPim->name()));
    mImportAddressbookPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importAddressBook();
    setValid(mImportAddressbookPageItem, result);
}

void ImportWizard::slotImportFiltersClicked()
{
    addFinishInfo(i18n("Import filters from %1...", mSelectedPim->name()));
    mImportFilterPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importFilters();
    setValid(mImportFilterPageItem, result);
}

void ImportWizard::slotImportMailsClicked()
{
    addFinishInfo(i18n("Import mails from %1...", mSelectedPim->name()));
    mImportMailPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importMails();
    setValid(mImportMailPageItem, result);
}

void ImportWizard::slotImportSettingsClicked()
{
    addFinishInfo(i18n("Import settings from %1...", mSelectedPim->name()));
    mImportSettingPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importSettings();
    setValid(mImportSettingPageItem, result);
}

void ImportWizard::slotImportCalendarClicked()
{
    addFinishInfo(i18n("Import calendar from %1...", mSelectedPim->name()));
    mImportCalendarPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importCalendar();
    setValid(mImportCalendarPageItem, result);
}

void ImportWizard::slotProgramSelected(const QString &program)
{
    if (mlistImport.contains(program)) {
        mSelectedPim = mlistImport.value(program);
        setValid(currentPage(), true);
    }
}

void ImportWizard::checkModules()
{
    mSelectProgramPage->setFoundProgram(mlistImport.keys());
}

void ImportWizard::addImportModule(AbstractImporter *import)
{
    if (import->foundMailer()) {
        mlistImport.insert(import->name(), import);
    } else {
        delete import;
    }
}

void ImportWizard::slotAtLeastOneComponentSelected(bool result)
{
    setValid(mSelectComponentPageItem, result);
}

void ImportWizard::setAppropriatePage(AbstractImporter::TypeSupportedOptions options)
{
    setAppropriate(mImportMailPageItem, (options & AbstractImporter::Mails));
    setAppropriate(mImportFilterPageItem, (options & AbstractImporter::Filters));
    setAppropriate(mImportSettingPageItem, (options & AbstractImporter::Settings));
    setAppropriate(mImportAddressbookPageItem, (options & AbstractImporter::AddressBooks));
    setAppropriate(mImportCalendarPageItem, (options & AbstractImporter::Calendars));
}

void ImportWizard::next()
{
    if (currentPage() == mSelectProgramPageItem) {
        KAssistantDialog::next();
        mSelectProgramPage->disableSelectProgram();
        mSelectComponentPage->setEnabledComponent(mSelectedPim->supportedOption());
    } else if (currentPage() == mSelectComponentPageItem) {
        setAppropriatePage(mSelectComponentPage->selectedComponents());
        KAssistantDialog::next();
        setValid(mImportMailPageItem, false);
    } else if (currentPage() == mImportMailPageItem) {
        KAssistantDialog::next();
        setValid(mImportFilterPageItem, false);
    } else if (currentPage() == mImportFilterPageItem) {
        KAssistantDialog::next();
        setValid(mImportSettingPageItem, false);
    } else if (currentPage() == mImportSettingPageItem) {
        KAssistantDialog::next();
        setValid(mImportAddressbookPageItem, false);
    } else if (currentPage() == mImportAddressbookPageItem) {
        KAssistantDialog::next();
        setValid(mImportCalendarPageItem, false);
    } else if (currentPage() == mImportCalendarPageItem) {
        KAssistantDialog::next();
        setValid(mImportFinishPageItem, true);
    } else if (currentPage() == mSelfilterpageItem) {
        // Save selected filter
        MailImporter::Filter *selectedFilter = mSelfilterpage->getSelectedFilter();
        Akonadi::Collection selectedCollection = mSelfilterpage->widget()->mCollectionRequestor->collection();
        // without filter don't go next
        if (!selectedFilter) {
            return;
        }
        // Ensure we have a valid collection.
        if (!selectedCollection.isValid()) {
            return;
        }
        // Goto next page
        KAssistantDialog::next();
        // Disable back & finish
        setValid(currentPage(), false);

        finishButton()->setEnabled(false);

        MailImporter::FilterInfo *info = new MailImporter::FilterInfo();
        ImportWizardFilterInfoGui *infoGui = new ImportWizardFilterInfoGui(mImportpage, this);
        info->setFilterInfoGui(infoGui);
        info->setStatusMessage(i18n("Import in progress"));
        info->setRemoveDupMessage(mSelfilterpage->removeDupMsg_checked());
        info->clear(); // Clear info from last time
        info->setRootCollection(selectedCollection);
        selectedFilter->setFilterInfo(info);
        selectedFilter->import();
        info->setStatusMessage(i18n("Import finished"));
        // Cleanup
        delete info;
        // Enable finish & back buttons
        setValid(currentPage(), true);
        finishButton()->setEnabled(true);
    } else {
        KAssistantDialog::next();
    }
}

void ImportWizard::enableAllImportButton()
{
    mImportMailPage->setImportButtonEnabled(true);
    mImportFilterPage->setImportButtonEnabled(true);
    mImportSettingPage->setImportButtonEnabled(true);
    mImportAddressbookPage->setImportButtonEnabled(true);
    mImportCalendarPage->setImportButtonEnabled(true);
}

void ImportWizard::back()
{
    if (currentPage() == mSelectProgramPageItem) {
        return;
    } else if (currentPage() == mImportFilterPageItem ||
               currentPage() == mImportSettingPageItem ||
               currentPage() == mImportAddressbookPageItem ||
               currentPage() == mImportCalendarPageItem ||
               currentPage() == mImportFinishPageItem) {
        enableAllImportButton();
    }
    KAssistantDialog::back();
}

ImportMailPage *ImportWizard::importMailPage() const
{
    return mImportMailPage;
}

ImportFilterPage *ImportWizard::importFilterPage() const
{
    return mImportFilterPage;
}

ImportAddressbookPage *ImportWizard::importAddressBookPage() const
{
    return mImportAddressbookPage;
}

ImportSettingPage *ImportWizard::importSettingPage() const
{
    return mImportSettingPage;
}
ImportFinishPage *ImportWizard::importFinishPage() const
{
    return mImportFinishPage;
}

ImportCalendarPage *ImportWizard::importCalendarPage() const
{
    return mImportCalendarPage;
}

void ImportWizard::addFinishInfo(const QString &log)
{
    mImportFinishPage->addImportInfo(log);
}

void ImportWizard::addFinishError(const QString &log)
{
    mImportFinishPage->addImportError(log);
}

void ImportWizard::slotSelectManualSelectionChanged(bool b)
{
    mMode = b ? Manual : AutoDetect;
    updatePagesFromMode();
}
