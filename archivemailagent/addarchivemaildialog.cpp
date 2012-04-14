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
#include "addarchivemaildialog.h"
#include "mailcommon/folderrequester.h"

#include <Akonadi/Collection>

#include <KLocale>
#include <KComboBox>

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

AddArchiveMailDialog::AddArchiveMailDialog(QWidget *parent)
  :KDialog(parent)
{
  setCaption( i18n( "Add Archive Mail" ) );
  setButtons( Ok|Cancel );
  setDefaultButton( Ok );
  setModal( true );
  QWidget *mainWidget = new QWidget( this );
  QGridLayout *mainLayout = new QGridLayout( mainWidget );
  mainLayout->setSpacing( KDialog::spacingHint() );
  mainLayout->setMargin( KDialog::marginHint() );
  setMainWidget( mainWidget );

  int row = 0;

  QLabel *folderLabel = new QLabel( i18n( "&Folder:" ), mainWidget );
  mainLayout->addWidget( folderLabel, row, 0 );
  mFolderRequester = new MailCommon::FolderRequester( mainWidget );
  mFolderRequester->setMustBeReadWrite( false );
  mFolderRequester->setNotAllowToCreateNewFolder( true );
  connect( mFolderRequester, SIGNAL(folderChanged(Akonadi::Collection)), SLOT(slotFolderChanged(Akonadi::Collection)) );
  folderLabel->setBuddy( mFolderRequester );
  mainLayout->addWidget( mFolderRequester, row, 1 );
  row++;

  QLabel *formatLabel = new QLabel( i18n( "F&ormat:" ), mainWidget );
  mainLayout->addWidget( formatLabel, row, 0 );
  mFormatComboBox = new KComboBox( mainWidget );
  formatLabel->setBuddy( mFormatComboBox );

  // These combobox values have to stay in sync with the ArchiveType enum from BackupJob!
  mFormatComboBox->addItem( i18n( "Compressed Zip Archive (.zip)" ) );
  mFormatComboBox->addItem( i18n( "Uncompressed Archive (.tar)" ) );
  mFormatComboBox->addItem( i18n( "BZ2-Compressed Tar Archive (.tar.bz2)" ) );
  mFormatComboBox->addItem( i18n( "GZ-Compressed Tar Archive (.tar.gz)" ) );
  mFormatComboBox->setCurrentIndex( 2 );
  connect( mFormatComboBox, SIGNAL(activated(int)),
           this, SLOT(slotFixFileExtension()) );
  mainLayout->addWidget( mFormatComboBox, row, 1 );
  row++;

  mRecursiveCheckBox = new QCheckBox( i18n( "Archive all subfolders" ), mainWidget );
  connect( mRecursiveCheckBox, SIGNAL(clicked()), this, SLOT(slotRecursiveCheckboxClicked()) );
  mainLayout->addWidget( mRecursiveCheckBox, row, 0, 1, 2, Qt::AlignLeft );
  mRecursiveCheckBox->setChecked( true );
  row++;
  mainLayout->setColumnStretch( 1, 1 );
  mainLayout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding ), row, 0 );

  // Make it a bit bigger, else the folder requester cuts off the text too early
  resize( 500, minimumSize().height() );


}

AddArchiveMailDialog::~AddArchiveMailDialog()
{

}

void AddArchiveMailDialog::slotFolderChanged(const Akonadi::Collection& collection)
{

}

#include "addarchivemaildialog.moc"
