/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (c) 2009-2013 Montel Laurent <montel@kde.org>

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

#include "collectiongeneralpage.h"
#include "collectiontypeutil.h"

#include "contentstypecombobox.h"
#include "collectionannotationsattribute.h"
#include "newmailnotifierattribute.h"
#include "foldercollection.h"
#include "kernel/mailkernel.h"
#include "util/mailutil.h"

#include "pimcommon/util/pimutil.h"
#include <Akonadi/AgentManager>
#include <Akonadi/AttributeFactory>
#include <Akonadi/Collection>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/EntityDisplayAttribute>

#include <KPIMIdentities/IdentityCombo>

#include <KColorScheme>
#include <KComboBox>
#include <KDialog>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

// TODO Where should these be?
#define KOLAB_FOLDERTYPE "/shared/vendor/kolab/folder-type"
#define KOLAB_INCIDENCESFOR "/shared/vendor/kolab/incidences-for"
#define KOLAB_SHAREDSEEN "/shared/vendor/cmu/cyrus-imapd/sharedseen"

using namespace Akonadi;
using namespace MailCommon;

CollectionGeneralPage::CollectionGeneralPage( QWidget *parent )
    : CollectionPropertiesPage( parent ),
      mContentsComboBox(0),
      mIncidencesForComboBox(0),
      mNameEdit( 0 ),
      mFolderCollection( 0 )
{
    setObjectName( QLatin1String( "MailCommon::CollectionGeneralPage" ) );
    setPageTitle( i18nc( "@title:tab General settings for a folder.", "General" ) );
}

CollectionGeneralPage::~CollectionGeneralPage()
{
}

static void addLine( QWidget *parent, QVBoxLayout *layout )
{
    QFrame *line = new QFrame( parent );
    line->setGeometry( QRect( 80, 150, 250, 20 ) );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );
    line->setFrameShape( QFrame::HLine );
    layout->addWidget( line );
}

void CollectionGeneralPage::init( const Akonadi::Collection &collection )
{
    mIsLocalSystemFolder = CommonKernel->isSystemFolderCollection( collection ) ||
            Kernel::folderIsInbox( collection, true );

    mIsResourceFolder = ( collection.parentCollection() == Akonadi::Collection::root() );
    QLabel *label;

    QVBoxLayout *topLayout = new QVBoxLayout( this );
    topLayout->setSpacing( KDialog::spacingHint() );
    topLayout->setMargin( 0 );

    // Musn't be able to edit details for a non-resource, system folder.
    if ( ( !mIsLocalSystemFolder || mIsResourceFolder ) &&
         !mFolderCollection->isReadOnly() ) {

        QHBoxLayout *hl = new QHBoxLayout();
        topLayout->addItem( hl );
        hl->setSpacing( KDialog::spacingHint() );
        hl->setMargin( KDialog::marginHint() );
        label = new QLabel( i18nc( "@label:textbox Name of the folder.", "&Name:" ), this );
        hl->addWidget( label );

        mNameEdit = new KLineEdit( this );
        connect( mNameEdit, SIGNAL(textChanged(QString)), SLOT(slotNameChanged(QString)) );
        label->setBuddy( mNameEdit );
        hl->addWidget( mNameEdit );
    }

    // should new mail in this folder be ignored?
    QHBoxLayout *hbl = new QHBoxLayout();
    topLayout->addItem( hbl );
    hbl->setSpacing( KDialog::spacingHint() );
    hbl->setMargin( KDialog::marginHint() );
    mNotifyOnNewMailCheckBox =
            new QCheckBox( i18n( "Act on new/unread mail in this folder" ), this );
    mNotifyOnNewMailCheckBox->setWhatsThis(
                i18n( "<qt><p>If this option is enabled then you will be notified about "
                      "new/unread mail in this folder. Moreover, going to the "
                      "next/previous folder with unread messages will stop at this "
                      "folder.</p>"
                      "<p>Uncheck this option if you do not want to be notified about "
                      "new/unread mail in this folder and if you want this folder to "
                      "be skipped when going to the next/previous folder with unread "
                      "messages. This is useful for ignoring any new/unread mail in "
                      "your trash and spam folder.</p></qt>" ) );
    hbl->addWidget( mNotifyOnNewMailCheckBox );
    // should replies to mails in this folder be kept in this same folder?
    hbl = new QHBoxLayout();
    topLayout->addItem( hbl );
    hbl->setSpacing( KDialog::spacingHint() );
    hbl->setMargin( KDialog::marginHint() );
    mKeepRepliesInSameFolderCheckBox =
            new QCheckBox( i18n( "Keep replies in this folder" ), this );
    mKeepRepliesInSameFolderCheckBox->setWhatsThis(
                i18n( "Check this option if you want replies you write "
                      "to mails in this folder to be put in this same folder "
                      "after sending, instead of in the configured sent-mail folder." ) );
    hbl->addWidget( mKeepRepliesInSameFolderCheckBox );
    hbl->addStretch( 1 );
    // should this folder be shown in the folder selection dialog?
    hbl = new QHBoxLayout();
    topLayout->addItem( hbl );
    hbl->setSpacing( KDialog::spacingHint() );
    hbl->setMargin( KDialog::marginHint() );
    mHideInSelectionDialogCheckBox =
            new QCheckBox( i18n( "Hide this folder in the folder selection dialog" ), this );
    mHideInSelectionDialogCheckBox->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this option if you do not want this folder "
                       "to be shown in folder selection dialogs, such as the <interface>"
                       "Jump to Folder</interface> dialog." ) );
    hbl->addWidget( mHideInSelectionDialogCheckBox );
    hbl->addStretch( 1 );

    addLine( this, topLayout );
    // use grid layout for the following combobox settings
    QGridLayout *gl = new QGridLayout();
    gl->setMargin( KDialog::marginHint() );
    topLayout->addItem( gl );
    gl->setSpacing( KDialog::spacingHint() );
    gl->setColumnStretch( 1, 100 ); // make the second column use all available space
    int row = -1;

    // sender identity
    ++row;
    mUseDefaultIdentityCheckBox = new QCheckBox( i18n( "Use &default identity" ), this );
    gl->addWidget( mUseDefaultIdentityCheckBox );
    connect( mUseDefaultIdentityCheckBox, SIGNAL(stateChanged(int)),
             this, SLOT(slotIdentityCheckboxChanged()) );
    ++row;
    label = new QLabel( i18n( "&Sender identity:" ), this );
    gl->addWidget( label, row, 0 );
    mIdentityComboBox = new KPIMIdentities::IdentityCombo( KernelIf->identityManager(), this );
    label->setBuddy( mIdentityComboBox );
    gl->addWidget( mIdentityComboBox, row, 1 );
    mIdentityComboBox->setWhatsThis(
                i18n( "Select the sender identity to be used when writing new mail "
                      "or replying to mail in this folder. This means that if you are in "
                      "one of your work folders, you can make KMail use the corresponding "
                      "sender email address, signature and signing or encryption keys "
                      "automatically. Identities can be set up in the main configuration "
                      "dialog. (Settings -> Configure KMail)" ) );

    CollectionTypeUtil::FolderContentsType contentsType = CollectionTypeUtil::ContentsTypeMail;

    const CollectionAnnotationsAttribute *annotationAttribute =
            collection.attribute<CollectionAnnotationsAttribute>();

    const QMap<QByteArray, QByteArray> annotations =
            ( annotationAttribute ?
                  annotationAttribute->annotations() :
                  QMap<QByteArray, QByteArray>() );

    const bool sharedSeen = ( annotations.value( KOLAB_SHAREDSEEN ) == "true" );

    CollectionTypeUtil collectionUtil;
    const CollectionTypeUtil::IncidencesFor incidencesFor =
            collectionUtil.incidencesForFromString( QLatin1String(annotations.value( KOLAB_INCIDENCESFOR )) );

    const CollectionTypeUtil::FolderContentsType folderType = collectionUtil.typeFromKolabName( annotations.value( KOLAB_FOLDERTYPE ) );

    // Only do make this settable, if the IMAP resource is enabled
    // and it's not the personal folders (those must not be changed)
    if ( PimCommon::Util::isImapResource(collection.resource()) ) {
        ++row;
        label = new QLabel( i18n( "&Folder contents:" ), this );
        gl->addWidget( label, row, 0 );
        mContentsComboBox = new KComboBox( this );
        label->setBuddy( mContentsComboBox );
        gl->addWidget( mContentsComboBox, row, 1 );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeMail ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeCalendar ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeContact ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeNote ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeTask ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeJournal ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeConfiguration ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeFreebusy ) );
        mContentsComboBox->addItem( collectionUtil.folderContentDescription( CollectionTypeUtil::ContentsTypeFile ) );

        mContentsComboBox->setCurrentIndex( contentsType );

        connect( mContentsComboBox, SIGNAL(activated(int)),
                 this, SLOT(slotFolderContentsSelectionChanged(int)) );

        if ( mFolderCollection->isReadOnly() || mIsResourceFolder ) {
            mContentsComboBox->setEnabled( false );
        }

    }

    // Kolab incidences-for annotation.
    // Show incidences-for combobox if the contents type can be changed (new folder),
    // or if it's set to calendar or task (existing folder)
    const bool folderTypeComboboxEnabled = ( folderType == CollectionTypeUtil::ContentsTypeCalendar || folderType == CollectionTypeUtil::ContentsTypeTask );
    ++row;
    mIncidencesForComboBox = new ContentsTypeComboBox(this);
    gl->addWidget(mIncidencesForComboBox, row, 0, 1, 1);

    mIncidencesForComboBox->setCurrentIndex( incidencesFor );
    mIncidencesForComboBox->setEnabled(folderTypeComboboxEnabled);

    if ( PimCommon::Util::isImapResource(collection.resource()) ) {
        mSharedSeenFlagsCheckBox = new QCheckBox( this );
        mSharedSeenFlagsCheckBox->setText( i18n( "Share unread state with all users" ) );
        mSharedSeenFlagsCheckBox->setChecked( sharedSeen );
        ++row;
        gl->addWidget( mSharedSeenFlagsCheckBox, row, 0, 1, 1 );
        mSharedSeenFlagsCheckBox->setWhatsThis(
                    i18n( "If enabled, the unread state of messages in this folder will be "
                          "the same for all users having access to this folder. If disabled "
                          "(the default), every user with access to this folder has their "
                          "own unread state." ) );
    } else {
        mSharedSeenFlagsCheckBox = 0;
    }

    topLayout->addStretch( 100 ); // eat all superfluous space
}

void CollectionGeneralPage::load( const Akonadi::Collection &collection )
{
    mFolderCollection = FolderCollection::forCollection( collection );
    init( collection );

    if ( mNameEdit ) {
        const QString displayName = collection.displayName();

        if ( !mIsLocalSystemFolder || mIsResourceFolder ) {
            mNameEdit->setText( displayName );
        }
    }

    // folder identity
    mIdentityComboBox->setCurrentIdentity( mFolderCollection->identity() );
    mUseDefaultIdentityCheckBox->setChecked( mFolderCollection->useDefaultIdentity() );

    // ignore new mail
    mNotifyOnNewMailCheckBox->setChecked( !Util::ignoreNewMailInFolder(collection) );

    const bool keepInFolder = ( mFolderCollection->canCreateMessages() &&
                                mFolderCollection->putRepliesInSameFolder() );

    mKeepRepliesInSameFolderCheckBox->setChecked( keepInFolder );
    mKeepRepliesInSameFolderCheckBox->setEnabled( mFolderCollection->canCreateMessages() );
    mHideInSelectionDialogCheckBox->setChecked( mFolderCollection->hideInSelectionDialog() );

    if ( mContentsComboBox ) {
        const CollectionAnnotationsAttribute *annotationsAttribute =
                collection.attribute<CollectionAnnotationsAttribute>();

        if ( annotationsAttribute ) {
            const QMap<QByteArray, QByteArray> annotations = annotationsAttribute->annotations();
            if ( annotations.contains( KOLAB_FOLDERTYPE ) ) {
                CollectionTypeUtil collectionUtil;
                mContentsComboBox->setCurrentItem(
                            collectionUtil.typeNameFromKolabType( annotations[ KOLAB_FOLDERTYPE ] ) );
            }
        }
    }
}

void CollectionGeneralPage::save( Collection &collection )
{
    if ( mNameEdit ) {
        if ( !mIsLocalSystemFolder ) {
            const QString nameFolder(mNameEdit->text().trimmed());
            bool canRenameFolder =  !(nameFolder.startsWith( QLatin1Char('.') ) ||
                                      nameFolder.endsWith( QLatin1Char('.') ) ||
                                      nameFolder.contains( QLatin1Char( '/' ) ) ||
                                      nameFolder.isEmpty());

            if ( mIsResourceFolder && (PimCommon::Util::isImapResource(collection.resource()))) {
                collection.setName( nameFolder );
                Akonadi::AgentInstance instance =
                        Akonadi::AgentManager::self()->instance( collection.resource() );
                instance.setName( nameFolder );
            } else if (canRenameFolder) {
                if ( collection.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
                     !collection.attribute<Akonadi::EntityDisplayAttribute>()->displayName().isEmpty() ) {
                    collection.attribute<Akonadi::EntityDisplayAttribute>()->setDisplayName(
                                nameFolder );
                } else if ( !nameFolder.isEmpty() ) {
                    collection.setName( nameFolder );
                }
            }
        }
    }

    if (!mNotifyOnNewMailCheckBox->isChecked()) {
        MailCommon::NewMailNotifierAttribute *newMailNotifierAttr = collection.attribute<MailCommon::NewMailNotifierAttribute>( Akonadi::Entity::AddIfMissing );
        newMailNotifierAttr->setIgnoreNewMail(true);
    } else {
        collection.removeAttribute<MailCommon::NewMailNotifierAttribute>();
    }

    CollectionAnnotationsAttribute *annotationsAttribute =
            collection.attribute<CollectionAnnotationsAttribute>( Entity::AddIfMissing );

    QMap<QByteArray, QByteArray> annotations = annotationsAttribute->annotations();
    if ( mSharedSeenFlagsCheckBox && mSharedSeenFlagsCheckBox->isEnabled() ) {
        annotations[ KOLAB_SHAREDSEEN ] = mSharedSeenFlagsCheckBox->isChecked() ? "true" : "false";
    }

    CollectionTypeUtil collectionUtil;
    if ( mIncidencesForComboBox && mIncidencesForComboBox->isEnabled() ) {
        annotations[ KOLAB_INCIDENCESFOR ] =
                collectionUtil.incidencesForToString(
                    static_cast<CollectionTypeUtil::IncidencesFor>( mIncidencesForComboBox->currentIndex() ) ).toLatin1();
    }

    if ( mContentsComboBox ) {
        const CollectionTypeUtil::FolderContentsType type =
                collectionUtil.contentsTypeFromString( mContentsComboBox->currentText() );

        const QByteArray kolabName = collectionUtil.kolabNameFromType( type ) ;
        if ( !kolabName.isEmpty() ) {
            QString iconName;
            switch( type ) {
            case CollectionTypeUtil::ContentsTypeCalendar:
                iconName= QString::fromLatin1( "view-calendar" );
                break;
            case CollectionTypeUtil::ContentsTypeContact:
                iconName= QString::fromLatin1( "view-pim-contacts" );
                break;
            case CollectionTypeUtil::ContentsTypeNote:
                iconName = QString::fromLatin1( "view-pim-notes" );
                break;
            case CollectionTypeUtil::ContentsTypeTask:
                iconName = QString::fromLatin1( "view-pim-tasks" );
                break;
            case CollectionTypeUtil::ContentsTypeJournal:
                iconName = QString::fromLatin1( "view-pim-journal" );
                break;
            case CollectionTypeUtil::ContentsTypeConfiguration:
                iconName = QString::fromLatin1( "configure" );
                break;
            case CollectionTypeUtil::ContentsTypeFreebusy:
                iconName = QString::fromLatin1( "view-calendar-agenda" );
                break;
            case CollectionTypeUtil::ContentsTypeFile:
                iconName = QString::fromLatin1( "document-open" );
                break;
            case CollectionTypeUtil::ContentsTypeMail:
            default:
                break;
            }

            Akonadi::EntityDisplayAttribute *attribute =
                    collection.attribute<Akonadi::EntityDisplayAttribute>( Akonadi::Entity::AddIfMissing );
            attribute->setIconName( iconName );
            new Akonadi::CollectionModifyJob( collection );
            annotations[ KOLAB_FOLDERTYPE ] = kolabName;
        }
    }
    if ( annotations.isEmpty() ) {
        collection.removeAttribute<CollectionAnnotationsAttribute>();
    } else {
        annotationsAttribute->setAnnotations( annotations );
    }

    if ( mFolderCollection ) {
        mFolderCollection->setIdentity( mIdentityComboBox->currentIdentity() );
        mFolderCollection->setUseDefaultIdentity( mUseDefaultIdentityCheckBox->isChecked() );

        mFolderCollection->setPutRepliesInSameFolder( mKeepRepliesInSameFolderCheckBox->isChecked() );
        mFolderCollection->setHideInSelectionDialog( mHideInSelectionDialogCheckBox->isChecked() );
        mFolderCollection->writeConfig();
    }
}

void CollectionGeneralPage::slotIdentityCheckboxChanged()
{
    mIdentityComboBox->setEnabled( !mUseDefaultIdentityCheckBox->isChecked() );
}

void CollectionGeneralPage::slotFolderContentsSelectionChanged( int )
{
    CollectionTypeUtil collectionUtil;
    const CollectionTypeUtil::FolderContentsType type =
            collectionUtil.contentsTypeFromString( mContentsComboBox->currentText() );

    if ( type != CollectionTypeUtil::ContentsTypeMail ) {
        const QString message =
                i18n( "You have configured this folder to contain groupware information. "
                      "That means that this folder will disappear once the configuration "
                      "dialog is closed." );

        KMessageBox::information( this, message );
    }

    const bool enable = ( type == CollectionTypeUtil::ContentsTypeCalendar ||
                          type == CollectionTypeUtil::ContentsTypeTask );

    if ( mIncidencesForComboBox ) {
        mIncidencesForComboBox->setEnabled( enable );
    }
}

void CollectionGeneralPage::slotNameChanged( const QString &name )
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;
    if ( name.startsWith( QLatin1Char('.') ) ||
         name.endsWith( QLatin1Char('.') ) ||
         name.contains( QLatin1Char( '/' ) ) ||
         name.isEmpty() ) {
        if (mColorName.isEmpty()) {
            const KColorScheme::BackgroundRole bgColorScheme( KColorScheme::NegativeBackground );
            KStatefulBrush bgBrush( KColorScheme::View, bgColorScheme );
            mColorName = bgBrush.brush( this ).color().name();
        }
        styleSheet = QString::fromLatin1( "QLineEdit{ background-color:%1 }" ).
                arg( mColorName );
    }
    setStyleSheet(styleSheet);
#endif
}

