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
#include <KUrlRequester>

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>

AddArchiveMailDialog::AddArchiveMailDialog(const ArchiveMailInfo& info,QWidget *parent)
  :KDialog(parent)
{
  if(info.isEmpty())
    setCaption( i18n( "Add Archive Mail" ) );
  else
    setCaption( i18n( "Modify Archive Mail" ) );
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

  QLabel *pathLabel = new QLabel( i18n( "Path:" ), mainWidget );
  mainLayout->addWidget( pathLabel, row, 0 );
  mPath = new KUrlRequester(mainWidget);
  mPath->setMode(KFile::Directory);
  mainLayout->addWidget(mPath);
  row++;

  mainLayout->setColumnStretch( 1, 1 );
  mainLayout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding ), row, 0 );

  if(!info.isEmpty()) {
    load(info);
  }

  // Make it a bit bigger, else the folder requester cuts off the text too early
  resize( 500, minimumSize().height() );


}

AddArchiveMailDialog::~AddArchiveMailDialog()
{

}

void AddArchiveMailDialog::load(const ArchiveMailInfo& info)
{
  mPath->setUrl(info.url());
  mRecursiveCheckBox->setChecked(info.saveSubCollection());
  mFolderRequester->setCollection(Akonadi::Collection(info.saveCollectionId()));
  mFormatComboBox->setCurrentIndex(static_cast<int>(info.archiveType()));  
#if 0 //TODO
  void setArchiveUnit( ArchiveMailInfo::ArchiveUnit unit );
  ArchiveMailInfo::ArchiveUnit archiveUnit() const;

  void setArchiveAge( int age );
  int archiveAge() const;

  void setLastDateSaved( const QDate& date );
  QDate lastDateSaved() const;
#endif

  //TODO
}

ArchiveMailInfo AddArchiveMailDialog::info()
{
  ArchiveMailInfo newInfo;
  newInfo.setSaveSubCollection(mRecursiveCheckBox->isChecked());
  newInfo.setArchiveType(static_cast<MailCommon::BackupJob::ArchiveType>(mFormatComboBox->currentIndex()));
  newInfo.setSaveCollectionId(mFolderRequester->collection().id());
  newInfo.setUrl(mPath->url());
  //TODO
  return newInfo;
}

void AddArchiveMailDialog::slotFolderChanged(const Akonadi::Collection& collection)
{

}

void AddArchiveMailDialog::setArchiveType(MailCommon::BackupJob::ArchiveType type)
{
  mFormatComboBox->setCurrentIndex((int)type);
}

MailCommon::BackupJob::ArchiveType AddArchiveMailDialog::archiveType() const
{
  return static_cast<MailCommon::BackupJob::ArchiveType>(mFormatComboBox->currentIndex());
}

void AddArchiveMailDialog::setRecursive( bool b )
{
  mRecursiveCheckBox->setChecked(b);
}

bool AddArchiveMailDialog::recursive() const
{
  return mRecursiveCheckBox->isChecked();
}

void AddArchiveMailDialog::setSelectedFolder(const Akonadi::Collection& collection)
{
  mFolderRequester->setCollection(collection);
}

Akonadi::Collection AddArchiveMailDialog::selectedFolder() const
{
  return mFolderRequester->collection();
}

KUrl AddArchiveMailDialog::path() const
{
  return mPath->url();
}

void AddArchiveMailDialog::setPath(const KUrl&url)
{
  mPath->setUrl(url);
}


#include "addarchivemaildialog.moc"
