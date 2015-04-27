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

#include <QAction>
#include <KAboutData>
#include <KLocalizedString>
#include "importwizard_debug.h"
#include <KMessageBox>
#include <KHelpMenu>
#include <AkonadiCore/control.h>
#include <mailcommon/kernel/mailkernel.h>
#include <QPushButton>

ImportWizard::ImportWizard(QWidget *parent)
    : KAssistantDialog(parent), mSelectedPim(0)
{
    setModal(true);
    setWindowTitle(i18n("PIM Import Tool"));
    setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Help);

    ImportWizardKernel *kernel = new ImportWizardKernel(this);
    CommonKernel->registerKernelIf(kernel);   //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf(kernel);   //SettingsIf is used in FolderTreeWidget
    createAutomaticModePage();
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
    Akonadi::Control::widgetNeedsAkonadi(this);

    checkModules();
    KMessageBox::information(this, i18n("Close KMail before importing data. Some plugins will modify KMail config file."));
    KHelpMenu *helpMenu = new KHelpMenu(this, KAboutData::applicationData(), true);
    //Initialize menu
    QMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(QIcon::fromTheme(QStringLiteral("kmail")));
    button(QDialogButtonBox::Help)->setMenu(menu);
}

ImportWizard::~ImportWizard()
{
    qDeleteAll(mlistImport);
}

void ImportWizard::createAutomaticModePage()
{
    mSelectProgramPage = new SelectProgramPage(this);
    mSelectProgramPageItem = new KPageWidgetItem(mSelectProgramPage, i18n("Detect program"));
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
    mPage8 = new KPageWidgetItem(mImportFinishPage, i18n("Finish"));
    addPage(mPage8);
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
        setValid(mPage8, true);
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
               currentPage() == mPage8) {
        enableAllImportButton();
    }
    KAssistantDialog::back();
}

void ImportWizard::reject()
{
    KAssistantDialog::reject();
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

