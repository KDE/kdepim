/*
    This file is part of KOrganizer.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqlayout.h>
#include <tqhgroupbox.h>
#include <tqvgroupbox.h>
#include <tqvbuttongroup.h>
#include <tqradiobutton.h>
#include <tqcheckbox.h>
#include <tqlineedit.h>
#include <tqhbox.h>
#include <tqvbox.h>
#include <tqpushbutton.h>
#include <tqfiledialog.h>
#include <tqtextstream.h>
#include <tqlabel.h>

#include <klocale.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <kurl.h>
#include <kio/job.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include "koglobals.h"
#include <kurlrequester.h>
#include <kio/netaccess.h>
#include <knotifyclient.h>
#include <ktempfile.h>
#include <kmessagebox.h>

#include <libkcal/calendar.h>
#include <libkcal/htmlexportsettings.h>

#include <libkdepim/kdateedit.h>
#include <libkdepim/kdateedit.h>

#include "koprefs.h"
#include "kocore.h"

#include "exportwebdialog.h"
#include "exportwebdialog.moc"


// FIXME: The basic structure of this dialog has been copied from KPrefsDialog,
//        because we want custom buttons, a Tabbed dialog, and a different
//        headline... Maybe we should try to achieve the same without code
//        duplication.
ExportWebDialog::ExportWebDialog( HTMLExportSettings *settings, TQWidget *parent,
                                  const char *name)
  : KDialogBase( Tabbed,i18n("Export Calendar as Web Page"),Help|Default|User1|Cancel, User1, parent, name, false, false, i18n("Export") ),
    KPrefsWidManager( settings ), mSettings( settings )
{
  setupGeneralPage();
  setupEventPage();
  setupTodoPage();
// Disabled bacause the functionality is not yet implemented.
//  setupJournalPage();
//  setupFreeBusyPage();
//  setupAdvancedPage();

  connect( this, TQT_SIGNAL( user1Clicked() ), TQT_SLOT( slotOk() ) );
  connect( this, TQT_SIGNAL( cancelClicked() ), TQT_SLOT( reject() ) );

  readConfig();
}

ExportWebDialog::~ExportWebDialog()
{
}

void ExportWebDialog::setDefaults()
{
  setWidDefaults();
}

void ExportWebDialog::readConfig()
{
  readWidConfig();
  usrReadConfig();
}

void ExportWebDialog::writeConfig()
{
  writeWidConfig();
  usrWriteConfig();
  readConfig();
}

void ExportWebDialog::slotApply()
{
  writeConfig();
  emit configChanged();
}

void ExportWebDialog::slotOk()
{
  slotApply();
  emit exportHTML( mSettings );
  accept();
}

void ExportWebDialog::slotDefault()
{
  kdDebug(5850) << "KPrefsDialog::slotDefault()" << endl;

  if (KMessageBox::warningContinueCancel(this,
      i18n("You are about to set all preferences to default values. All "
      "custom modifications will be lost."),i18n("Setting Default Preferences"),
      i18n("Reset to Defaults"))
    == KMessageBox::Continue) setDefaults();
}


void ExportWebDialog::setupGeneralPage()
{
  mGeneralPage = addPage( i18n("General") );
  TQVBoxLayout *topLayout = new TQVBoxLayout(mGeneralPage, 10);

  TQGroupBox *rangeGroup = new TQHGroupBox( i18n("Date Range"), mGeneralPage );
  topLayout->addWidget( rangeGroup );
  addWidDate( mSettings->dateStartItem(), rangeGroup );
  addWidDate( mSettings->dateEndItem(), rangeGroup );

  TQButtonGroup *typeGroup = new TQVButtonGroup( i18n("View Type"), mGeneralPage );
  topLayout->addWidget( typeGroup );
//  addWidBool( mSettings->weekViewItem(), typeGroup );
  addWidBool( mSettings->monthViewItem(), typeGroup );
  addWidBool( mSettings->eventViewItem(), typeGroup );
  addWidBool( mSettings->todoViewItem(), typeGroup );
//  addWidBool( mSettings->journalViewItem(), typeGroup );
//  addWidBool( mSettings->freeBusyViewItem(), typeGroup );
  addWidBool( mSettings->excludePrivateItem(), typeGroup );
  addWidBool( mSettings->excludeConfidentialItem(), typeGroup );

  TQGroupBox *destGroup = new TQVGroupBox(i18n("Destination"), mGeneralPage );
  topLayout->addWidget(destGroup);
  KPrefsWidPath *pathWid = addWidPath( mSettings->outputFileItem(),
                                       destGroup, "text/html", KFile::File );
  connect( pathWid->urlRequester(), TQT_SIGNAL( textChanged( const TQString & ) ),
           TQT_SLOT( slotTextChanged( const TQString & ) ) );

  topLayout->addStretch( 1 );
}

void ExportWebDialog::slotTextChanged( const TQString & _text)
{
    enableButton( User1, !_text.isEmpty() );
}

void ExportWebDialog::setupTodoPage()
{
  mTodoPage = addPage(i18n("To-dos"));
  TQVBoxLayout *topLayout = new TQVBoxLayout( mTodoPage, 10 );

  TQHBox *hbox = new TQHBox( mTodoPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->todoListTitleItem(), hbox );

  TQVBox *vbox = new TQVBox( mTodoPage );
  topLayout->addWidget( vbox );
  addWidBool( mSettings->taskDueDateItem(), vbox );
  addWidBool( mSettings->taskLocationItem(), vbox );
  addWidBool( mSettings->taskCategoriesItem(), vbox );
  addWidBool( mSettings->taskAttendeesItem(), vbox );
//  addWidBool( mSettings->taskExcludePrivateItem(), vbox );
//  addWidBool( mSettings->taskExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}

void ExportWebDialog::setupEventPage()
{
  mEventPage = addPage(i18n("Events"));
  TQVBoxLayout *topLayout = new TQVBoxLayout( mEventPage, 10 );

  TQHBox *hbox = new TQHBox( mEventPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->eventTitleItem(), hbox );

  TQVBox *vbox = new TQVBox( mEventPage );
  topLayout->addWidget( vbox );
  addWidBool( mSettings->eventLocationItem(), vbox );
  addWidBool( mSettings->eventCategoriesItem(), vbox );
  addWidBool( mSettings->eventAttendeesItem(), vbox );
//  addWidBool( mSettings->eventExcludePrivateItem(), vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}
/*
void ExportWebDialog::setupJournalPage()
{
  mJournalPage = addPage(i18n("Journal"));
  TQVBoxLayout *topLayout = new TQVBoxLayout( mJournalPage, 10 );

  TQHBox *hbox = new TQHBox( mJournalPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->journalTitleItem(), hbox );

  TQVBox *vbox = new TQVBox( mJournalPage );
  topLayout->addWidget( vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}

void ExportWebDialog::setupFreeBusyPage()
{
  mFreeBusyPage = addPage(i18n("Free/Busy"));
  TQVBoxLayout *topLayout = new TQVBoxLayout( mFreeBusyPage, 10 );

  TQHBox *hbox = new TQHBox( mFreeBusyPage );
  topLayout->addWidget( hbox );
  addWidString( mSettings->journalTitleItem(), hbox );

  TQVBox *vbox = new TQVBox( mFreeBusyPage );
  topLayout->addWidget( vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}

void ExportWebDialog::setupAdvancedPage()
{
  mAdvancedPage = addPage(i18n("Advanced"));
  TQVBoxLayout *topLayout = new TQVBoxLayout( mAdvancedPage, 10 );

  TQVBox *vbox = new TQVBox( mAdvancedPage );
  topLayout->addWidget( vbox );
//  addWidBool( mSettings->eventExcludeConfidentialItem(), vbox );

  topLayout->addStretch(1);
}
*/
