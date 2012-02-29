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
#include "importfinishpage.h"

#include "thunderbird/thunderbirdimportdata.h"
#include "sylpheed/sylpheedimportdata.h"
#include "evolutionv3/evolutionv3importdata.h"
#include "evolutionv2/evolutionv2importdata.h"
#include "evolutionv1/evolutionv1importdata.h"

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
  mPage1 = new KPageWidgetItem( mCheckProgramPage, i18n( "Detect program" ) );
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

  mImportFinishPage = new ImportFinishPage(this);
  mPage7 = new KPageWidgetItem( mImportFinishPage, i18n( "Finish" ) );
  addPage( mPage7 );

  
  //Import module
  addImportModule(new ThunderbirdImportData(this));
  addImportModule(new SylpheedImportData(this));
  addImportModule(new Evolutionv3ImportData(this));
  addImportModule(new Evolutionv2ImportData(this));
  addImportModule(new Evolutionv1ImportData(this));

  // Disable the 'next button to begin with.
  setValid( currentPage(), false );

  connect(this,SIGNAL(helpClicked()),this,SLOT(help()));
  connect(mCheckProgramPage,SIGNAL(programSelected(QString)),this,SLOT(slotProgramSelected(QString)));
  connect(mImportMailPage,SIGNAL(importMailsClicked()),this,SLOT(slotImportMailsClicked()));
  Akonadi::Control::widgetNeedsAkonadi(this);

  checkModules();
}

ImportWizard::~ImportWizard()
{
  qDeleteAll(mlistImport);
}

void ImportWizard::slotImportFiltersClicked()
{
  const bool result = mSelectedPim->importFilters();
  setValid(mPage4,result);
}

void ImportWizard::slotImportMailsClicked()
{
    const bool result = mSelectedPim->importMails();
    setValid(mPage3,result);
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
  if ( import->foundMailer() )
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
      setValid(mPage3,false);
    } else if( currentPage() == mPage3 ) {
      KAssistantDialog::next();
      setValid(mPage4,false);
    } else if( currentPage() == mPage4 ) {
      setValid(mPage5,false);
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

ImportMailPage* ImportWizard::importMailPage()
{
  return mImportMailPage;
}

ImportFilterPage* ImportWizard::importFilterPage()
{
  return mImportFilterPage;
}


#include "importwizard.moc"
