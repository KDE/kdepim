/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2011 Montel Laurent <montel@kde.org>

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

#include "filteractionmissingargumentdialog.h"
#include "kmfilterdialog.h"
#include "folderrequester.h"
#include "kernel/mailkernel.h"
#include "util/mailutil.h"
#include "addtagdialog.h"

#include <Akonadi/EntityMimeTypeFilterModel>

#include <Mailtransport/TransportComboBox>
#include <Mailtransport/Transport>
#include <Mailtransport/TransportManager>

#include <KPIMIdentities/IdentityCombo>

#include <KLocale>
#include <KUrlRequester>

#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>

FilterActionMissingCollectionDialog::FilterActionMissingCollectionDialog(
        const Akonadi::Collection::List &list, const QString &filtername,
        const QString &argStr, QWidget *parent )
    : KDialog( parent ),
      mListwidget( 0 )
{
    setModal( true );
    setCaption( i18n( "Select Folder" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );
    QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
    QLabel *lab = new QLabel( i18n( "Folder path was \"%1\".", argStr ) );
    lab->setWordWrap(true);
    lay->addWidget( lab );
    if ( !list.isEmpty() ) {
        lab = new QLabel( i18n( "The following folders can be used for this filter:" ) );
        lab->setWordWrap(true);
        lay->addWidget( lab );
        mListwidget = new QListWidget( this );
        lay->addWidget( mListwidget );
        const int numberOfItems( list.count() );
        for ( int i = 0; i< numberOfItems; ++i ) {
            Akonadi::Collection col = list.at( i );
            QListWidgetItem *item = new QListWidgetItem( MailCommon::Util::fullCollectionPath( col ) );
            item->setData( FilterActionMissingCollectionDialog::IdentifyCollection, col.id() );
            mListwidget->addItem( item );
        }
        connect( mListwidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                 SLOT(slotCurrentItemChanged()));
        connect( mListwidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
                 SLOT(slotDoubleItemClicked(QListWidgetItem*)));

    }

    QLabel *label = new QLabel( this );
    label->setWordWrap(true);
    if (filtername.isEmpty())
        label->setText( i18n( "Please select a folder" ));
    else
        label->setText( i18n( "Filter folder is missing. "
                              "Please select a folder to use with filter \"%1\"",
                              filtername ) );
    lay->addWidget( label );
    mFolderRequester = new MailCommon::FolderRequester( this );
    connect( mFolderRequester, SIGNAL(folderChanged(Akonadi::Collection)),
             this, SLOT(slotFolderChanged(Akonadi::Collection)) );
    lay->addWidget( mFolderRequester );
    enableButtonOk( false );
    readConfig();
}

FilterActionMissingCollectionDialog::~FilterActionMissingCollectionDialog()
{
    writeConfig();
}

void FilterActionMissingCollectionDialog::readConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingCollectionDialog" );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void FilterActionMissingCollectionDialog::writeConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingCollectionDialog" );
    group.writeEntry( "Size", size() );
}


void FilterActionMissingCollectionDialog::slotFolderChanged( const Akonadi::Collection &col )
{
    enableButtonOk( col.isValid() );
}

void FilterActionMissingCollectionDialog::slotDoubleItemClicked( QListWidgetItem *item )
{
    if ( !item ) {
        return;
    }

    const Akonadi::Collection::Id id =
            item->data( FilterActionMissingCollectionDialog::IdentifyCollection ).toLongLong();

    mFolderRequester->setCollection( Akonadi::Collection( id ) );
    accept();
}

void FilterActionMissingCollectionDialog::slotCurrentItemChanged()
{
    QListWidgetItem *currentItem = mListwidget->currentItem();
    if ( currentItem ) {
        const Akonadi::Collection::Id id =
                currentItem->data( FilterActionMissingCollectionDialog::IdentifyCollection ).toLongLong();
        mFolderRequester->setCollection( Akonadi::Collection( id ) );
    }
}

Akonadi::Collection FilterActionMissingCollectionDialog::selectedCollection() const
{
    return mFolderRequester->collection();
}

void FilterActionMissingCollectionDialog::getPotentialFolders( const QAbstractItemModel *model,
                                                               const QModelIndex &parentIndex,
                                                               const QString &lastElement,
                                                               Akonadi::Collection::List &list )
{
    const int rowCount = model->rowCount( parentIndex );
    for ( int row = 0; row < rowCount; ++row ) {
        const QModelIndex index = model->index( row, 0, parentIndex );
        if ( model->rowCount( index ) > 0 ) {
            getPotentialFolders( model, index, lastElement, list );
        }
        if ( model->data( index ).toString() == lastElement ) {
            list << model->data(
                        index, Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
        }
    }
}

Akonadi::Collection::List FilterActionMissingCollectionDialog::potentialCorrectFolders(
        const QString &path, bool &exactPath )
{
    Akonadi::Collection::List lst;
    const QString realPath = MailCommon::Util::realFolderPath( path );
    if ( realPath.isEmpty() ) {
        return lst;
    }

    const int lastSlash = realPath.lastIndexOf( QLatin1Char( '/' ) );
    QString lastElement;
    if ( lastSlash == -1 ) {
        lastElement = realPath;
    } else {
        lastElement = realPath.right( realPath.length() - lastSlash - 1 );
    }

    if ( KernelIf->collectionModel() ) {
        FilterActionMissingCollectionDialog::getPotentialFolders(
                    KernelIf->collectionModel(), QModelIndex(), lastElement, lst );

        const int numberOfItems( lst.count() );
        for ( int i = 0; i < numberOfItems; ++i ) {
            if ( MailCommon::Util::fullCollectionPath( lst.at( i ) ) == realPath ) {
                exactPath = true;
                return  Akonadi::Collection::List() << lst.at( i );
            }
        }
    }
    return lst;
}

FilterActionMissingIdentityDialog::FilterActionMissingIdentityDialog( const QString &filtername,
                                                                      QWidget *parent )
    : KDialog( parent )
{
    setModal( true );
    setCaption( i18n( "Select Identity" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );
    QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
    QLabel *label = new QLabel( this );
    label->setText( i18n( "Filter identity is missing. "
                          "Please select an identity to use with filter \"%1\"",
                          filtername ) );
    label->setWordWrap(true);
    lay->addWidget( label );
    mComboBoxIdentity = new KPIMIdentities::IdentityCombo( KernelIf->identityManager(), this );
    lay->addWidget( mComboBoxIdentity );
    readConfig();
}

FilterActionMissingIdentityDialog::~FilterActionMissingIdentityDialog()
{
    writeConfig();
}

void FilterActionMissingIdentityDialog::readConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingMissingIdentity" );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void FilterActionMissingIdentityDialog::writeConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingMissingIdentity" );
    group.writeEntry( "Size", size() );
}


int FilterActionMissingIdentityDialog::selectedIdentity() const
{
    return mComboBoxIdentity->currentIdentity();
}

FilterActionMissingTransportDialog::FilterActionMissingTransportDialog( const QString &filtername,
                                                                        QWidget *parent )
    : KDialog( parent )
{
    setModal( true );
    setCaption( i18n( "Select Transport" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );
    QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
    QLabel *label = new QLabel( this );
    label->setText( i18n( "Filter transport is missing. "
                          "Please select a transport to use with filter \"%1\"",
                          filtername ) );
    label->setWordWrap(true);
    lay->addWidget( label );
    mComboBoxTransport = new MailTransport::TransportComboBox( this );
    lay->addWidget( mComboBoxTransport );
    readConfig();
}

FilterActionMissingTransportDialog::~FilterActionMissingTransportDialog()
{
    writeConfig();
}

void FilterActionMissingTransportDialog::readConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingTransportDialog" );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void FilterActionMissingTransportDialog::writeConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingTransportDialog" );
    group.writeEntry( "Size", size() );
}


int FilterActionMissingTransportDialog::selectedTransport() const
{
    return mComboBoxTransport->currentTransportId();
}

FilterActionMissingTemplateDialog::FilterActionMissingTemplateDialog(
        const QStringList &templateList, const QString &filtername, QWidget *parent )
    : KDialog( parent )
{
    setModal( true );
    setCaption( i18n( "Select Template" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );
    QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
    QLabel *label = new QLabel( this );
    label->setText( i18n( "Filter template is missing. "
                          "Please select a template to use with filter \"%1\"",
                          filtername ) );
    label->setWordWrap(true);
    lay->addWidget( label );
    mComboBoxTemplate = new KComboBox( this );
    mComboBoxTemplate->addItems( templateList );
    lay->addWidget( mComboBoxTemplate );
    readConfig();
}

FilterActionMissingTemplateDialog::~FilterActionMissingTemplateDialog()
{
    writeConfig();
}


void FilterActionMissingTemplateDialog::readConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingTemplateDialog" );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void FilterActionMissingTemplateDialog::writeConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingTemplateDialog" );
    group.writeEntry( "Size", size() );
}

QString FilterActionMissingTemplateDialog::selectedTemplate() const
{
    if ( mComboBoxTemplate->currentIndex() == 0 ) {
        return QString();
    } else {
        return mComboBoxTemplate->currentText();
    }
}

FilterActionMissingAccountDialog::FilterActionMissingAccountDialog( const QStringList &lstAccount,
                                                                    const QString &filtername,
                                                                    QWidget *parent )
    : KDialog( parent )
{
    setModal( true );
    setCaption( i18n( "Select Account" ) );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    showButtonSeparator( true );
    QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
    QLabel *label = new QLabel( this );
    label->setText( i18n( "Filter account is missing. "
                          "Please select account to use with filter \"%1\"",
                          filtername ) );
    label->setWordWrap(true);
    lay->addWidget( label );
    mAccountList = new MailCommon::AccountList( this );
    mAccountList->applyOnAccount( lstAccount );
    lay->addWidget( mAccountList );
    readConfig();
}

FilterActionMissingAccountDialog::~FilterActionMissingAccountDialog()
{
    writeConfig();
}

void FilterActionMissingAccountDialog::readConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingAccountDialog" );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void FilterActionMissingAccountDialog::writeConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingAccountDialog" );
    group.writeEntry( "Size", size() );
}

QStringList FilterActionMissingAccountDialog::selectedAccount() const
{
    return mAccountList->selectedAccount();
}

bool FilterActionMissingAccountDialog::allAccountExist( const QStringList &lst )
{
    const Akonadi::AgentInstance::List lstAgent = MailCommon::Util::agentInstances();

    const int numberOfAccount( lst.count() );
    const int numberOfAgent( lstAgent.count() );

    for ( int i = 0; i <numberOfAccount; ++i ) {
        bool found = false;
        const QString accountName( lst.at( i ) );
        for ( int j=0; j < numberOfAgent;++j ) {
            if ( lstAgent.at( j ).identifier() ==  accountName ) {
                found = true;
                break;
            }
        }
        if ( !found ) {
            return false;
        }
    }
    return true;
}

FilterActionMissingTagDialog::FilterActionMissingTagDialog(
        const QMap<QUrl, QString> &tagList, const QString &filtername,
        const QString &argsStr, QWidget *parent )
    : KDialog( parent )
{
    setModal( true );
    setCaption( i18n( "Select Tag" ) );
    setButtons( Ok | User1 | Cancel );
    setDefaultButton( Ok );
    setButtonText(KDialog::User1, i18n("Add Tag..."));
    showButtonSeparator( true );
    QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
    QLabel *label = new QLabel( i18n( "Tag was \"%1\".", argsStr ) );
    lay->addWidget( label );

    label = new QLabel( this );
    label->setText( i18n( "Filter tag is missing. "
                          "Please select a tag to use with filter \"%1\"",
                          filtername ) );
    label->setWordWrap(true);
    lay->addWidget( label );
    mTagList = new QListWidget( this );

    QMapIterator<QUrl, QString> map(tagList);
    while (map.hasNext()) {
        map.next();
        QListWidgetItem *item = new QListWidgetItem( map.value() );
        item->setData(UrlData, map.key().toString());
        mTagList->addItem(item);
    }

    connect(this,SIGNAL(user1Clicked()),SLOT(slotAddTag()));
    connect( mTagList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
             this, SLOT(accept()) );
    lay->addWidget( mTagList );
    readConfig();
}

FilterActionMissingTagDialog::~FilterActionMissingTagDialog()
{
    writeConfig();
}

void FilterActionMissingTagDialog::readConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingTagDialog" );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void FilterActionMissingTagDialog::writeConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingTagDialog" );
    group.writeEntry( "Size", size() );
}

QString FilterActionMissingTagDialog::selectedTag() const
{
    if ( mTagList->currentItem() ) {
        return mTagList->currentItem()->data(UrlData).toString();
    }
    return QString();
}

void FilterActionMissingTagDialog::slotAddTag()
{
    QPointer<MailCommon::AddTagDialog> dlg = new MailCommon::AddTagDialog(QList<KActionCollection*>(),this);
    if (dlg->exec())  {
        QListWidgetItem *item = new QListWidgetItem( dlg->label() );
        item->setData(UrlData, dlg->tag().url().url());
        mTagList->addItem(item);
    }
    delete dlg;
}

FilterActionMissingSoundUrlDialog::FilterActionMissingSoundUrlDialog( const QString &filtername,
                                                                      const QString &argStr,
                                                                      QWidget *parent )
    :KDialog(parent)
{
    setModal( true );
    setButtons( Ok | Cancel );
    setDefaultButton( Ok );
    setCaption( i18n( "Select sound" ) );
    showButtonSeparator( true );
    QVBoxLayout *lay = new QVBoxLayout( mainWidget() );
    QLabel *label = new QLabel( i18n( "Sound file was \"%1\".", argStr ) );
    lay->addWidget( label );

    label = new QLabel( this );
    label->setText( i18n( "Sound file is missing. "
                          "Please select a sound to use with filter \"%1\"",
                          filtername ) );
    label->setWordWrap(true);
    lay->addWidget( label );
    mUrlWidget = new KUrlRequester( this );
    lay->addWidget( mUrlWidget );
    readConfig();
}

FilterActionMissingSoundUrlDialog::~FilterActionMissingSoundUrlDialog()
{
    writeConfig();
}

QString FilterActionMissingSoundUrlDialog::soundUrl() const
{
    return mUrlWidget->url().path();
}

void FilterActionMissingSoundUrlDialog::readConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingSoundUrlDialog" );

    const QSize size = group.readEntry( "Size", QSize(500, 300) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void FilterActionMissingSoundUrlDialog::writeConfig()
{
    KConfigGroup group( KernelIf->config(), "FilterActionMissingSoundUrlDialog" );
    group.writeEntry( "Size", size() );
}



