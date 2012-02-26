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

#include "importwizard.h"
#include "importwizardkernel.h"
#include "checkprogrampage.h"
#include "selectcomponentpage.h"
#include "importmailpage.h"
#include "importfilterpage.h"
#include "importsettingpage.h"
#include "importaddressbookpage.h"

#include "thunderbird/thunderbirdimportdata.h"
#include "sylpheed/sylpheedimportdata.h"

#include <kaboutapplicationdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <akonadi/control.h>
#include <mailcommon/mailkernel.h>

ImportWizard::ImportWizard(QWidget *parent)
  : KAssistantDialog(parent), mSelectedPim( 0 )
{
  setModal(true);
  setWindowTitle( i18n( "PIM Import Tool" ) );

  ImportWizardKernel *kernel = new ImportWizardKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

  mCheckProgramPage = new CheckProgramPage(this);
  mPage1 = new KPageWidgetItem( mCheckProgramPage, i18n( "Step 1: Detect pim" ) );
  addPage( mPage1);

  mSelectComponentPage = new SelectComponentPage(this);
  mPage2 = new KPageWidgetItem( mSelectComponentPage, i18n( "Step 2: Select import components" ) );
  addPage( mPage2);

  mImportMailPage = new ImportMailPage(this);
  mPage3 = new KPageWidgetItem( mImportMailPage, i18n( "Step 3: Import mails" ) );
  addPage( mPage3);

  mImportFilterPage = new ImportFilterPage(this);
  mPage4 = new KPageWidgetItem( mImportFilterPage, i18n( "Step 4: Import filters" ) );
  addPage( mPage4 );

  mImportSettingPage = new ImportSettingPage(this);
  mPage5 = new KPageWidgetItem( mImportSettingPage, i18n( "Step 5: Import settings" ) );
  addPage( mPage5);

  mImportAddressbookPage = new ImportAddressbookPage(this);
  mPage6 = new KPageWidgetItem( mImportAddressbookPage, i18n( "Step 6: Import addressbooks" ) );
  addPage( mPage6 );

  //Import module
  addImportModule(new ThunderbirdImportData(mImportMailPage));
  addImportModule(new SylpheedImportData(mImportMailPage));

  // Disable the 'next button to begin with.
  setValid( currentPage(), false );

  connect(this,SIGNAL(helpClicked()),this,SLOT(help()));
  connect(mCheckProgramPage,SIGNAL(programSelected(QString)),this,SLOT(slotProgramSelected(QString)));
  Akonadi::Control::widgetNeedsAkonadi(this);

  checkModules();
}

ImportWizard::~ImportWizard()
{
  qDeleteAll(mlistImport);
}

void ImportWizard::slotProgramSelected(const QString& program)
{
  
  if(mlistImport.contains(program)) {
    mSelectedPim = mlistImport.value( program );
    setValid( currentPage(), true );
  }
}

void ImportWizard::checkModules()
{
  mCheckProgramPage->setFoundProgram(mlistImport.keys());
}

void ImportWizard::addImportModule(PimImportAbstract *import)
{
  mlistImport.insert(import->name(),import);
}

void ImportWizard::help()
{
  KAboutApplicationDialog a( KGlobal::mainComponent().aboutData(), this );
  a.exec();
}

void ImportWizard::setAppropriatePage(PimImportAbstract::TypeSupportedOptions options)
{
    setAppropriate(mPage6,(options & PimImportAbstract::AddressBook));
    setAppropriate(mPage4,(options & PimImportAbstract::Filters));
    setAppropriate(mPage3,(options & PimImportAbstract::Mails));
    setAppropriate(mPage5,(options & PimImportAbstract::Settings));

}

void ImportWizard::next()
{
  if( currentPage() == mPage1 ) {
      KAssistantDialog::next();
      mCheckProgramPage->disableSelectProgram();
      mSelectComponentPage->setEnabledComponent(mSelectedPim->supportedOption());
    } else if( currentPage() == mPage2 ) {
      setAppropriatePage(mSelectComponentPage->selectedComponents());
      KAssistantDialog::next();
    } else if( currentPage() == mPage3 ) {
      setValid(mPage3,false);
      KAssistantDialog::next();
      setValid(mPage3,mSelectedPim->importMails());
    } else if( currentPage() == mPage4 ) {
      setValid(mPage4,false);
      KAssistantDialog::next();
      setValid(mPage4,mSelectedPim->importFilters());
    } else if( currentPage() == mPage5 ) {
      KAssistantDialog::next();
      mSelectedPim->importSettings();
    } else if( currentPage() == mPage6 ) {
      KAssistantDialog::next();
      mSelectedPim->importAddressBook();
    } else {
      KAssistantDialog::next();
    }
}

void ImportWizard::reject()
{
  KAssistantDialog::reject();
}

#include "importwizard.moc"
