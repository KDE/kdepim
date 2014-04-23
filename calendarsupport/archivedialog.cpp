/*
  Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

// ArchiveDialog -- archive/delete past events.

#include "archivedialog.h"
#include "eventarchiver.h"
#include "kcalprefs.h"

#include <Akonadi/Calendar/IncidenceChanger>

#include <KComboBox>
#include <KDateComboBox>
#include <KFileDialog>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNumInput>
#include <KUrl>
#include <KUrlRequester>
#include <KVBox>

#include <QButtonGroup>
#include <QCheckBox>
#include <QDateTime>
#include <QFrame>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QWhatsThis>

using namespace CalendarSupport;

ArchiveDialog::ArchiveDialog( const Akonadi::ETMCalendar::Ptr &cal,
                              Akonadi::IncidenceChanger *changer,
                              QWidget *parent )
  : KDialog (parent)
{
  setCaption( i18nc( "@title:window", "Archive/Delete Past Events and To-dos" ) );
  setButtons( User1|Cancel );
  setDefaultButton( User1 );
  setModal( false );
  showButtonSeparator( true );
  setButtonText( User1, i18nc( "@action:button", "&Archive" ) );
  mCalendar = cal;
  mChanger = changer;

  QFrame *topFrame = new QFrame( this );
  setMainWidget( topFrame );
  QVBoxLayout *topLayout = new QVBoxLayout( topFrame );
  topLayout->setSpacing( spacingHint() );
#ifndef KDEPIM_MOBILE_UI
  QLabel *descLabel = new QLabel( topFrame );
  descLabel->setText(
    i18nc( "@info:whatsthis",
           "Archiving saves old items into the given file and "
           "then deletes them in the current calendar. If the archive file "
           "already exists they will be added. "
           "(<link url=\"#\">How to restore</link>)" ) );
  descLabel->setWhatsThis(
    i18nc( "@info:whatsthis",
           "In order to add an archive to your calendar, use the Merge Calendar "
           "function. You can view an archive by opening it like you would any "
           "other calendar. It is not saved in a special format, but as "
           "vCalendar." ) );
  descLabel->setTextInteractionFlags(
    Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard |
    Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard );
  descLabel->setWordWrap( true );
  topLayout->addWidget( descLabel );
  connect( descLabel, SIGNAL(linkActivated(QString)), SLOT(showWhatsThis()) );
#endif

  QButtonGroup *radioBG = new QButtonGroup( this );
  connect( radioBG, SIGNAL(buttonClicked(int)), SLOT(slotActionChanged()) );

  QHBoxLayout *dateLayout = new QHBoxLayout();
  dateLayout->setMargin( 0 );
  mArchiveOnceRB = new QRadioButton( i18nc( "@option:radio",
                                            "Archive now items older than:" ),
                                     topFrame );
  mArchiveOnceRB->setToolTip(
    i18nc( "@info:tooltip",
           "Enable one time archiving or purging of older items" ) );
  mArchiveOnceRB->setWhatsThis(
    i18nc( "@info:whatsthis",
           "If you check this box, events and to-dos older than the specified age "
           "will be archived or purged. The items will be archived unless the "
           "\"Delete only\" option is enabled; else the items will be purged "
           "and not saved." ) );

  dateLayout->addWidget( mArchiveOnceRB );
  radioBG->addButton( mArchiveOnceRB );
  mDateEdit = new KDateComboBox( topFrame );
  mDateEdit->setToolTip(
    i18nc( "@info:tooltip",
           "Set the one time archiving cut-off date" ) );
  mDateEdit->setWhatsThis(
    i18nc( "@info:whatsthis",
           "The date before which items should be archived. All older events "
           "and to-dos will be saved and deleted, the newer (and events "
           "exactly on that date) will be kept." ) );
  dateLayout->addWidget( mDateEdit );
  topLayout->addLayout( dateLayout );

  // Checkbox, numinput and combo for auto-archiving (similar to kmail's
  // mExpireFolderCheckBox/mReadExpiryTimeNumInput in kmfolderdia.cpp)
  KHBox *autoArchiveHBox = new KHBox( topFrame );
  topLayout->addWidget( autoArchiveHBox );
  mAutoArchiveRB = new QRadioButton( i18nc( "@option:radio",
                                            "Automaticall&y archive items older than:" ),
                                     autoArchiveHBox );
  mAutoArchiveRB->setToolTip(
    i18nc( "@info:tooltip",
            "Enable automatic archiving or purging of older items" ) );
  mAutoArchiveRB->setWhatsThis(
    i18nc( "@info:whatsthis",
           "If this feature is enabled, the application will regularly check if "
           "events and to-dos have to be archived; this means you will not "
           "need to use this dialog box again, except to change the settings." ) );
  radioBG->addButton( mAutoArchiveRB );

  mExpiryTimeNumInput = new KIntNumInput( autoArchiveHBox );
  mExpiryTimeNumInput->setRange( 1, 500, 1 );
  mExpiryTimeNumInput->setSliderEnabled( false );
  mExpiryTimeNumInput->setEnabled( false );
  mExpiryTimeNumInput->setValue( 7 );
  mExpiryTimeNumInput->setToolTip(
    i18nc( "@info:tooltip",
           "Set the archival age in days, weeks or months" ) );
  mExpiryTimeNumInput->setWhatsThis(
    i18nc( "@info:whatsthis",
           "The age of the events and to-dos to archive. All older items "
           "will be saved and deleted, the newer will be kept." ) );

  mExpiryUnitsComboBox = new KComboBox( autoArchiveHBox );
  mExpiryUnitsComboBox->setToolTip(
    i18nc( "@info:tooltip",
           "Set the units for the automatic archive age" ) );
  mExpiryUnitsComboBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select the time units (days, weeks or months) for automatic archiving." ) );
  // Those items must match the "Expiry Unit" enum in the kcfg file!
  mExpiryUnitsComboBox->addItem(
    i18nc( "@item:inlistbox expires in daily units", "Day(s)" ) );
  mExpiryUnitsComboBox->addItem(
    i18nc( "@item:inlistbox expiration in weekly units", "Week(s)" ) );
  mExpiryUnitsComboBox->addItem(
    i18nc( "@item:inlistbox expiration in monthly units", "Month(s)" ) );
  mExpiryUnitsComboBox->setEnabled( false );

  QHBoxLayout *fileLayout = new QHBoxLayout();
  fileLayout->setMargin( 0 );
  fileLayout->setSpacing( spacingHint() );
  QLabel *l = new QLabel( i18nc( "@label", "Archive &file:" ), topFrame );
  fileLayout->addWidget( l );
  mArchiveFile = new KUrlRequester( KCalPrefs::instance()->mArchiveFile, topFrame );
  mArchiveFile->setMode( KFile::File );
  mArchiveFile->setFilter( i18nc( "@label filter for KUrlRequester", "*.ics|iCalendar Files" ) );
  mArchiveFile->setToolTip(
    i18nc( "@info:tooltip",
           "Set the location of the archive" ) );
  mArchiveFile->setWhatsThis(
    i18nc( "@info:whatsthis",
           "The path of the archive file. The events and to-dos will be appended "
           "to the specified file, so any events that are already in the file "
           "will not be modified or deleted. You can later load or merge the "
           "file like any other calendar. It is not saved in a special "
           "format, it uses the iCalendar format." ) );
//QT5
  //mArchiveFile->fileDialog()->setOperationMode( KFileDialog::Saving );
  l->setBuddy( mArchiveFile->lineEdit() );
  fileLayout->addWidget( mArchiveFile );
  topLayout->addLayout( fileLayout );

#ifndef KDEPIM_MOBILE_UI
  QGroupBox *typeBox = new QGroupBox( i18nc( "@title:group", "Type of Items to Archive" ) );
  typeBox->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Here you can select which items "
           "should be archived. Events are archived if they "
           "ended before the date given above; to-dos are archived if "
           "they were finished before the date." ) );

  topLayout->addWidget( typeBox );
  QBoxLayout *typeLayout = new QVBoxLayout( typeBox );
#else
  QBoxLayout *typeLayout = new QHBoxLayout();
  topLayout->addLayout( typeLayout );
#endif

  mEvents = new QCheckBox( i18nc( "@option:check", "Archive &Events" ) );
  mEvents->setToolTip(
    i18nc( "@option:check", "Archive or purge events" ) );
  mEvents->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select this option to archive events if they ended before the date given above." ) );
  typeLayout->addWidget( mEvents );

  mTodos = new QCheckBox( i18nc( "@option:check", "Archive Completed &To-dos" ) );
  mTodos->setToolTip(
    i18nc( "@option:check", "Archive or purge completed to-dos" ) );
  mTodos->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select this option to archive to-dos if they were completed "
           "before the date given above." ) );
  typeLayout->addWidget( mTodos );

  mDeleteCb = new QCheckBox( i18nc( "@option:check", "&Delete only, do not save" ), topFrame );
  mDeleteCb->setToolTip(
    i18nc( "@info:tooltip",
           "Purge the old items without saving them" ) );
  mDeleteCb->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Select this option to delete old events and to-dos without saving "
           "them. It is not possible to recover the events later." ) );
  topLayout->addWidget( mDeleteCb );
  connect( mDeleteCb, SIGNAL(toggled(bool)), mArchiveFile, SLOT(setDisabled(bool)) );
  connect( mDeleteCb, SIGNAL(toggled(bool)), this, SLOT(slotEnableUser1()) );
  connect( mArchiveFile->lineEdit(), SIGNAL(textChanged(QString)),
           this, SLOT(slotEnableUser1()) );

  // Load settings from KCalPrefs
  mExpiryTimeNumInput->setValue( KCalPrefs::instance()->mExpiryTime );
  mExpiryUnitsComboBox->setCurrentIndex( KCalPrefs::instance()->mExpiryUnit );
  mDeleteCb->setChecked( KCalPrefs::instance()->mArchiveAction == KCalPrefs::actionDelete );
  mEvents->setChecked( KCalPrefs::instance()->mArchiveEvents );
  mTodos->setChecked( KCalPrefs::instance()->mArchiveTodos );

  slotEnableUser1();

  // The focus should go to a useful field by default, not to the top richtext-label
  if ( KCalPrefs::instance()->mAutoArchive ) {
    mAutoArchiveRB->setChecked( true );
    mAutoArchiveRB->setFocus();
  } else {
    mArchiveOnceRB->setChecked( true );
    mArchiveOnceRB->setFocus();
  }
  slotActionChanged();
  connect( this, SIGNAL(user1Clicked()), this, SLOT(slotUser1()) );
}

ArchiveDialog::~ArchiveDialog()
{
}

void ArchiveDialog::slotEnableUser1()
{
  const bool state = ( mDeleteCb->isChecked() || !mArchiveFile->lineEdit()->text().isEmpty() );
  enableButton( KDialog::User1, state );
}

void ArchiveDialog::slotActionChanged()
{
  mDateEdit->setEnabled( mArchiveOnceRB->isChecked() );
  mExpiryTimeNumInput->setEnabled( mAutoArchiveRB->isChecked() );
  mExpiryUnitsComboBox->setEnabled( mAutoArchiveRB->isChecked() );
}

// Archive old events
void ArchiveDialog::slotUser1()
{
  EventArchiver archiver;
  connect( &archiver, SIGNAL(eventsDeleted()), this, SLOT(slotEventsDeleted()) );

  KCalPrefs::instance()->mAutoArchive = mAutoArchiveRB->isChecked();
  KCalPrefs::instance()->mExpiryTime = mExpiryTimeNumInput->value();
  KCalPrefs::instance()->mExpiryUnit = mExpiryUnitsComboBox->currentIndex();

  if ( mDeleteCb->isChecked() ) {
    KCalPrefs::instance()->mArchiveAction = KCalPrefs::actionDelete;
  } else {
    KCalPrefs::instance()->mArchiveAction = KCalPrefs::actionArchive;

    // Get destination URL
    KUrl destUrl( mArchiveFile->url() );
    if ( !destUrl.isValid() ) {
      KMessageBox::sorry( this, i18nc( "@info", "The archive file name is not valid." ) );
      return;
    }
    // Force filename to be ending with vCalendar extension
    QString filename = destUrl.fileName();
    if ( !filename.endsWith( QLatin1String( ".vcs" ) ) &&
         !filename.endsWith( QLatin1String( ".ics" ) ) ) {
      filename.append( QLatin1String( ".ics" ) );
      destUrl.setFileName( filename );
    }

    KCalPrefs::instance()->mArchiveFile = destUrl.url();
  }
  if ( KCalPrefs::instance()->mAutoArchive ) {
    archiver.runAuto( mCalendar, mChanger, this, true /*with gui*/);
    emit autoArchivingSettingsModified();
    accept();
  } else {
    archiver.runOnce( mCalendar, mChanger, mDateEdit->date(), this );
    accept();
  }
}

void ArchiveDialog::slotEventsDeleted()
{
  emit eventsDeleted();
  if ( !KCalPrefs::instance()->mAutoArchive ) {
    accept();
  }
}

void ArchiveDialog::showWhatsThis()
{
  QWidget *widget = qobject_cast< QWidget * >( sender() );
  if ( widget && !widget->whatsThis().isEmpty() ) {
    QWhatsThis::showText( QCursor::pos(), widget->whatsThis() );
  }
}

