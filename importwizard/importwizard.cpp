/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "thunderbird/thunderbirdimportdata.h"
#include "sylpheed/sylpheedimportdata.h"
#include "evolutionv3/evolutionv3importdata.h"
#include "evolutionv2/evolutionv2importdata.h"
#include "evolutionv1/evolutionv1importdata.h"
#include "opera/operaimportdata.h"
#include "oe/oeimportdata.h"
#include "mailapp/mailappimportdata.h"
#include "pmail/pmailimportdata.h"
#include "thebat/thebatimportdata.h"
#include "balsa/balsaimportdata.h"
#include "claws-mail/clawsmailimportdata.h"
#include "trojita/trojitaimportdata.h"
#include "kmail1/kmail1importdata.h"

#include <kaboutapplicationdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <KMessageBox>
#include <KHelpMenu>
#include <KMenu>
#include <akonadi/control.h>
#include <mailcommon/kernel/mailkernel.h>

ImportWizard::ImportWizard(QWidget *parent)
    : KAssistantDialog(parent), mSelectedPim( 0 )
{
    setModal(true);
    setWindowTitle( i18n( "PIM Import Tool" ) );
    KGlobal::locale()->insertCatalog( QLatin1String("libmailimporter") );
    KGlobal::locale()->insertCatalog( QLatin1String("libmailcommon") );
    KGlobal::locale()->insertCatalog( QLatin1String("libpimcommon") );

    ImportWizardKernel *kernel = new ImportWizardKernel( this );
    CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

    mSelectProgramPage = new SelectProgramPage(this);
    mPage1 = new KPageWidgetItem( mSelectProgramPage, i18n( "Detect program" ) );
    addPage( mPage1);

    mSelectComponentPage = new SelectComponentPage(this);
    mPage2 = new KPageWidgetItem( mSelectComponentPage, i18n( "Select material to import" ) );
    addPage( mPage2);

    mImportMailPage = new ImportMailPage(this);
    mPage3 = new KPageWidgetItem( mImportMailPage, i18n( "Import mail messages" ) );
    addPage( mPage3);

    mImportFilterPage = new ImportFilterPage(this);
    mPage4 = new KPageWidgetItem( mImportFilterPage, i18n( "Import mail filters" ) );
    addPage( mPage4 );

    mImportSettingPage = new ImportSettingPage(this);
    mPage5 = new KPageWidgetItem( mImportSettingPage, i18n( "Import settings" ) );
    addPage( mPage5);

    mImportAddressbookPage = new ImportAddressbookPage(this);
    mPage6 = new KPageWidgetItem( mImportAddressbookPage, i18n( "Import addressbooks" ) );
    addPage( mPage6 );

    mImportCalendarPage = new ImportCalendarPage(this);
    mPage7 = new KPageWidgetItem( mImportCalendarPage, i18n( "Import calendars" ) );
    addPage( mPage7 );


    mImportFinishPage = new ImportFinishPage(this);
    mPage8 = new KPageWidgetItem( mImportFinishPage, i18n( "Finish" ) );
    addPage( mPage8 );


    //Import module
    addImportModule(new ThunderbirdImportData(this));
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
    addImportModule(new KMail1ImportData(this));

    // Disable the 'next button to begin with.
    setValid( currentPage(), false );

    connect(mSelectProgramPage,SIGNAL(programSelected(QString)),this,SLOT(slotProgramSelected(QString)));
    connect(mSelectProgramPage, SIGNAL(doubleClicked()), this, SLOT(slotProgramDoubleClicked()) );
    connect(mImportMailPage,SIGNAL(importMailsClicked()),this,SLOT(slotImportMailsClicked()));
    connect(mImportFilterPage, SIGNAL(importFiltersClicked()), this, SLOT(slotImportFiltersClicked()) );
    connect(mImportSettingPage, SIGNAL(importSettingsClicked()), this, SLOT(slotImportSettingsClicked()) );
    connect(mImportAddressbookPage, SIGNAL(importAddressbookClicked()), this, SLOT(slotImportAddressbookClicked()) );
    connect(mImportCalendarPage, SIGNAL(importCalendarClicked()), this, SLOT(slotImportCalendarClicked()) );


    connect(mSelectComponentPage, SIGNAL(atLeastOneComponentSelected(bool)), this, SLOT(slotAtLeastOneComponentSelected(bool)) );

    resize( 640, 480 );
    Akonadi::Control::widgetNeedsAkonadi(this);

    checkModules();
    KMessageBox::information(this,i18n("Close KMail before importing data. Some plugins will modify KMail config file."));
    KHelpMenu *helpMenu = new KHelpMenu(this, KGlobal::mainComponent().aboutData(), true);
    setButtonMenu( Help, helpMenu->menu() );
}

ImportWizard::~ImportWizard()
{
    qDeleteAll(mlistImport);
}

void ImportWizard::slotProgramDoubleClicked()
{
    next();
}

void ImportWizard::slotImportAddressbookClicked()
{
    addFinishInfo( i18n( "Import addressbook from %1...", mSelectedPim->name() ) );
    mImportAddressbookPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importAddressBook();
    setValid(mPage6,result);
}

void ImportWizard::slotImportFiltersClicked()
{
    addFinishInfo( i18n( "Import filters from %1...", mSelectedPim->name() ) );
    mImportFilterPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importFilters();
    setValid(mPage4,result);
}

void ImportWizard::slotImportMailsClicked()
{
    addFinishInfo( i18n( "Import mails from %1...", mSelectedPim->name() ) );
    mImportMailPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importMails();
    setValid(mPage3,result);
}

void ImportWizard::slotImportSettingsClicked()
{
    addFinishInfo( i18n( "Import settings from %1...", mSelectedPim->name() ) );
    mImportSettingPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importSettings();
    setValid(mPage5,result);
}

void ImportWizard::slotImportCalendarClicked()
{
    addFinishInfo( i18n( "Import calendar from %1...", mSelectedPim->name() ) );
    mImportCalendarPage->setImportButtonEnabled(false);
    const bool result = mSelectedPim->importCalendar();
    setValid(mPage7,result);
}

void ImportWizard::slotProgramSelected(const QString& program)
{
    if (mlistImport.contains(program)) {
        mSelectedPim = mlistImport.value( program );
        setValid( currentPage(), true );
    }
}

void ImportWizard::checkModules()
{
    mSelectProgramPage->setFoundProgram(mlistImport.keys());
}

void ImportWizard::addImportModule(AbstractImporter *import)
{
    if ( import->foundMailer() )
        mlistImport.insert(import->name(),import);
    else
        delete import;
}

void ImportWizard::slotAtLeastOneComponentSelected( bool result )
{
    setValid(mPage2,result);
}

void ImportWizard::setAppropriatePage(AbstractImporter::TypeSupportedOptions options)
{
    setAppropriate(mPage3,(options & AbstractImporter::Mails));
    setAppropriate(mPage4,(options & AbstractImporter::Filters));
    setAppropriate(mPage5,(options & AbstractImporter::Settings));
    setAppropriate(mPage6,(options & AbstractImporter::AddressBooks));
    setAppropriate(mPage7,(options & AbstractImporter::Calendars));

}

void ImportWizard::next()
{
    if ( currentPage() == mPage1 ) {
        KAssistantDialog::next();
        mSelectProgramPage->disableSelectProgram();
        mSelectComponentPage->setEnabledComponent(mSelectedPim->supportedOption());
    } else if ( currentPage() == mPage2 ) {
        setAppropriatePage(mSelectComponentPage->selectedComponents());
        KAssistantDialog::next();
        setValid(mPage3,false);
    } else if ( currentPage() == mPage3 ) {
        KAssistantDialog::next();
        setValid(mPage4,false);
    } else if ( currentPage() == mPage4 ) {
        KAssistantDialog::next();
        setValid(mPage5,false);
    } else if ( currentPage() == mPage5 ) {
        KAssistantDialog::next();
        setValid(mPage6,false);
    } else if ( currentPage() == mPage6 ) {
        KAssistantDialog::next();
        setValid(mPage7,false);
    } else if ( currentPage() == mPage7 ) {
        KAssistantDialog::next();
        setValid(mPage8,true);
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
    if ( currentPage() == mPage1 ) {
        return;
    } else if ( currentPage() == mPage4 ||
               currentPage() == mPage5 ||
               currentPage() == mPage6 ||
               currentPage() == mPage7 ||
               currentPage() == mPage8 ) {
        enableAllImportButton();
    }
    KAssistantDialog::back();
}

void ImportWizard::reject()
{
    KAssistantDialog::reject();
}

ImportMailPage* ImportWizard::importMailPage() const
{
    return mImportMailPage;
}

ImportFilterPage* ImportWizard::importFilterPage() const
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

void ImportWizard::addFinishInfo( const QString& log )
{
    mImportFinishPage->addImportInfo( log );
}

void ImportWizard::addFinishError( const QString& log )
{
    mImportFinishPage->addImportError( log );
}

