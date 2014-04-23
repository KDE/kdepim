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
#include "addarchivemaildialog.h"
#include "mailcommon/folder/folderrequester.h"

#include <Collection>

#include <KLocale>
#include <KComboBox>
#include <KUrlRequester>
#include <KIntSpinBox>
#include <KSeparator>

#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>

AddArchiveMailDialog::AddArchiveMailDialog(ArchiveMailInfo *info,QWidget *parent)
    : KDialog(parent),
      mInfo(info)
{
    if (info)
        setCaption( i18n( "Modify Archive Mail" ) );
    else
        setCaption( i18n( "Add Archive Mail" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    setWindowIcon( KIcon( QLatin1String("kmail") ) );
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
    if (info) //Don't autorize to modify folder when we just modify item.
        mFolderRequester->setEnabled( false );
    folderLabel->setBuddy( mFolderRequester );
    mainLayout->addWidget( mFolderRequester, row, 1 );
    ++row;

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
    mainLayout->addWidget( mFormatComboBox, row, 1 );
    ++row;

    mRecursiveCheckBox = new QCheckBox( i18n( "Archive all subfolders" ), mainWidget );
    mainLayout->addWidget( mRecursiveCheckBox, row, 0, 1, 2, Qt::AlignLeft );
    mRecursiveCheckBox->setChecked( true );
    ++row;

    QLabel *pathLabel = new QLabel( i18n( "Path:" ), mainWidget );
    mainLayout->addWidget( pathLabel, row, 0 );
    mPath = new KUrlRequester(mainWidget);
    connect(mPath, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateOkButton()));
    mPath->setMode(KFile::Directory);
    mainLayout->addWidget(mPath);
    ++row;

    QLabel *dateLabel = new QLabel( i18n( "Backup each:" ), mainWidget );
    mainLayout->addWidget( dateLabel, row, 0 );

    QHBoxLayout * hlayout = new QHBoxLayout;
    mDays = new QSpinBox(mainWidget);
    mDays->setMinimum(1);
    mDays->setMaximum(3600);
    hlayout->addWidget(mDays);

    mUnits = new KComboBox(mainWidget);
    QStringList unitsList;
    unitsList<<i18n("Days");
    unitsList<<i18n("Weeks");
    unitsList<<i18n("Months");
    unitsList<<i18n("Years");
    mUnits->addItems(unitsList);
    hlayout->addWidget(mUnits);

    mainLayout->addLayout(hlayout, row, 1);
    ++row;

    QLabel *maxCountlabel = new QLabel( i18n( "Maximum number of archive:" ), mainWidget );
    mainLayout->addWidget( maxCountlabel, row, 0 );
    mMaximumArchive = new KIntSpinBox( mainWidget );
    mMaximumArchive->setMinimum(0);
    mMaximumArchive->setMaximum(9999);
    mMaximumArchive->setSpecialValueText(i18n("unlimited"));
    maxCountlabel->setBuddy( mMaximumArchive );
    mainLayout->addWidget( mMaximumArchive, row, 1 );
    ++row;

    mainLayout->addWidget(new KSeparator, row, 0, row, 2);
    mainLayout->setColumnStretch( 1, 1 );
    mainLayout->addItem( new QSpacerItem( 1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding ), row, 0 );

    if (mInfo) {
        load(mInfo);
    } else {
        enableButtonOk(false);
    }

    // Make it a bit bigger, else the folder requester cuts off the text too early
    resize( 500, minimumSize().height() );
}

AddArchiveMailDialog::~AddArchiveMailDialog()
{
}

void AddArchiveMailDialog::load(ArchiveMailInfo *info)
{
    mPath->setUrl(info->url());
    mRecursiveCheckBox->setChecked(info->saveSubCollection());
    mFolderRequester->setCollection(Akonadi::Collection(info->saveCollectionId()));
    mFormatComboBox->setCurrentIndex(static_cast<int>(info->archiveType()));
    mDays->setValue(info->archiveAge());
    mUnits->setCurrentIndex(static_cast<int>(info->archiveUnit()));
    mMaximumArchive->setValue(info->maximumArchiveCount());
    slotUpdateOkButton();
}

ArchiveMailInfo* AddArchiveMailDialog::info()
{
    if (!mInfo) {
        mInfo = new ArchiveMailInfo();
    }
    mInfo->setSaveSubCollection(mRecursiveCheckBox->isChecked());
    mInfo->setArchiveType(static_cast<MailCommon::BackupJob::ArchiveType>(mFormatComboBox->currentIndex()));
    mInfo->setSaveCollectionId(mFolderRequester->collection().id());
    mInfo->setUrl(mPath->url());
    mInfo->setArchiveAge(mDays->value());
    mInfo->setArchiveUnit(static_cast<ArchiveMailInfo::ArchiveUnit>(mUnits->currentIndex()));
    mInfo->setMaximumArchiveCount(mMaximumArchive->value());
    return mInfo;
}

void AddArchiveMailDialog::slotUpdateOkButton()
{
    const bool valid = (!mPath->url().isEmpty() && mFolderRequester->collection().isValid());
    enableButtonOk(valid);
}

void AddArchiveMailDialog::slotFolderChanged(const Akonadi::Collection& collection)
{
    Q_UNUSED(collection);
    slotUpdateOkButton();
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

void AddArchiveMailDialog::setSelectedFolder(const Akonadi::Collection &collection)
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

void AddArchiveMailDialog::setPath(const KUrl &url)
{
    mPath->setUrl(url);
}

void AddArchiveMailDialog::setMaximumArchiveCount(int max)
{
    mMaximumArchive->setValue(max);
}

int AddArchiveMailDialog::maximumArchiveCount() const
{
    return mMaximumArchive->value();
}


