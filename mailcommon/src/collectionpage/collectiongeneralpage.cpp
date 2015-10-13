/*
  Copyright (c) 2009-2015 Montel Laurent <montel@kde.org>

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

#include "incidencesforwidget.h"
#include "contenttypewidget.h"
#include "attributes/collectionannotationsattribute.h"
#include "folder/foldercollection.h"
#include "kernel/mailkernel.h"
#include "util/mailutil.h"

#include "PimCommon/PimUtil"
#include <AgentManager>
#include <AttributeFactory>
#include <Collection>
#include <CollectionModifyJob>
#include <EntityDisplayAttribute>
#include <AkonadiCore/NewMailNotifierAttribute>
#include <KIdentityManagement/IdentityCombo>
#include "PimCommon/ImapResourceCapabilitiesManager"

#include <KColorScheme>
#include <KComboBox>
#include <QDialog>
#include <KLineEdit>
#include <KLocalizedString>
#include <KMessageBox>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

using namespace Akonadi;
using namespace MailCommon;

CollectionGeneralPage::CollectionGeneralPage(QWidget *parent)
    : CollectionPropertiesPage(parent),
      mContentsComboBox(0),
      mIncidencesForComboBox(0),
      mSharedSeenFlagsCheckBox(0),
      mNameEdit(0),
      mFolderCollection(0)
{
    setObjectName(QStringLiteral("MailCommon::CollectionGeneralPage"));
    setPageTitle(i18nc("@title:tab General settings for a folder.", "General"));
}

CollectionGeneralPage::~CollectionGeneralPage()
{
}

void CollectionGeneralPage::addLine(QWidget *parent, QVBoxLayout *layout)
{
    QFrame *line = new QFrame(parent);
    line->setGeometry(QRect(80, 150, 250, 20));
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setFrameShape(QFrame::HLine);
    layout->addWidget(line);
}

void CollectionGeneralPage::init(const Akonadi::Collection &collection)
{
    mIsLocalSystemFolder = CommonKernel->isSystemFolderCollection(collection) ||
                           Kernel::folderIsInbox(collection, true);

    mIsResourceFolder = (collection.parentCollection() == Akonadi::Collection::root());
    QLabel *label;

    QVBoxLayout *topLayout = new QVBoxLayout(this);

    // Musn't be able to edit details for a non-resource, system folder.
    if ((!mIsLocalSystemFolder || mIsResourceFolder) &&
            !mFolderCollection->isReadOnly()) {

        QHBoxLayout *hl = new QHBoxLayout();
        topLayout->addItem(hl);
        label = new QLabel(i18nc("@label:textbox Name of the folder.", "&Name:"), this);
        hl->addWidget(label);

        mNameEdit = new KLineEdit(this);
        mNameEdit->setTrapReturnKey(true);

        connect(mNameEdit, &KLineEdit::textChanged, this, &CollectionGeneralPage::slotNameChanged);
        label->setBuddy(mNameEdit);
        hl->addWidget(mNameEdit);
    }

    // should new mail in this folder be ignored?
    QHBoxLayout *hbl = new QHBoxLayout();
    topLayout->addItem(hbl);
    mNotifyOnNewMailCheckBox =
        new QCheckBox(i18n("Act on new/unread mail in this folder"), this);
    mNotifyOnNewMailCheckBox->setWhatsThis(
        i18n("<qt><p>If this option is enabled then you will be notified about "
             "new/unread mail in this folder. Moreover, going to the "
             "next/previous folder with unread messages will stop at this "
             "folder.</p>"
             "<p>Uncheck this option if you do not want to be notified about "
             "new/unread mail in this folder and if you want this folder to "
             "be skipped when going to the next/previous folder with unread "
             "messages. This is useful for ignoring any new/unread mail in "
             "your trash and spam folder.</p></qt>"));
    hbl->addWidget(mNotifyOnNewMailCheckBox);
    // should replies to mails in this folder be kept in this same folder?
    hbl = new QHBoxLayout();
    topLayout->addItem(hbl);
    mKeepRepliesInSameFolderCheckBox =
        new QCheckBox(i18n("Keep replies in this folder"), this);
    mKeepRepliesInSameFolderCheckBox->setWhatsThis(
        i18n("Check this option if you want replies you write "
             "to mails in this folder to be put in this same folder "
             "after sending, instead of in the configured sent-mail folder."));
    hbl->addWidget(mKeepRepliesInSameFolderCheckBox);
    hbl->addStretch(1);
    // should this folder be shown in the folder selection dialog?
    hbl = new QHBoxLayout();
    topLayout->addItem(hbl);
    mHideInSelectionDialogCheckBox =
        new QCheckBox(i18n("Hide this folder in the folder selection dialog"), this);
    mHideInSelectionDialogCheckBox->setWhatsThis(
        xi18nc("@info:whatsthis",
               "Check this option if you do not want this folder "
               "to be shown in folder selection dialogs, such as the <interface>"
               "Jump to Folder</interface> dialog."));
    hbl->addWidget(mHideInSelectionDialogCheckBox);
    hbl->addStretch(1);

    addLine(this, topLayout);
    // use grid layout for the following combobox settings
    QGridLayout *gl = new QGridLayout();
    topLayout->addItem(gl);
    gl->setColumnStretch(1, 100);   // make the second column use all available space
    int row = -1;

    // sender identity
    ++row;
    mUseDefaultIdentityCheckBox = new QCheckBox(i18n("Use &default identity"), this);
    gl->addWidget(mUseDefaultIdentityCheckBox);
    connect(mUseDefaultIdentityCheckBox, &QCheckBox::stateChanged, this, &CollectionGeneralPage::slotIdentityCheckboxChanged);
    ++row;
    label = new QLabel(i18n("&Sender identity:"), this);
    gl->addWidget(label, row, 0);
    mIdentityComboBox = new KIdentityManagement::IdentityCombo(KernelIf->identityManager(), this);
    label->setBuddy(mIdentityComboBox);
    gl->addWidget(mIdentityComboBox, row, 1);
    mIdentityComboBox->setWhatsThis(
        i18n("Select the sender identity to be used when writing new mail "
             "or replying to mail in this folder. This means that if you are in "
             "one of your work folders, you can make KMail use the corresponding "
             "sender email address, signature and signing or encryption keys "
             "automatically. Identities can be set up in the main configuration "
             "dialog. (Settings -> Configure KMail)"));

    // Only do make this settable, if the IMAP resource is enabled
    // and it's not the personal folders (those must not be changed)
    const QString collectionResource = collection.resource();
    if (CommonKernel->imapResourceManager()->hasAnnotationSupport(collectionResource)) {
        CollectionTypeUtil::FolderContentsType contentsType = CollectionTypeUtil::ContentsTypeMail;

        const CollectionAnnotationsAttribute *annotationAttribute =
            collection.attribute<CollectionAnnotationsAttribute>();

        const QMap<QByteArray, QByteArray> annotations =
            (annotationAttribute ?
             annotationAttribute->annotations() :
             QMap<QByteArray, QByteArray>());

        const bool sharedSeen = (annotations.value(CollectionTypeUtil::kolabSharedSeen()) == "true");

        CollectionTypeUtil collectionUtil;
        const CollectionTypeUtil::IncidencesFor incidencesFor =
            collectionUtil.incidencesForFromString(QLatin1String(annotations.value(CollectionTypeUtil::kolabIncidencesFor())));

        const CollectionTypeUtil::FolderContentsType folderType = collectionUtil.typeFromKolabName(annotations.value(CollectionTypeUtil::kolabFolderType()));

        ++row;
        mContentsComboBox = new ContentTypeWidget(this);
        gl->addWidget(mContentsComboBox, row, 0, 1, 2);
        mContentsComboBox->setCurrentIndex(contentsType);

        connect(mContentsComboBox, &ContentTypeWidget::activated, this, &CollectionGeneralPage::slotFolderContentsSelectionChanged);

        if (mFolderCollection->isReadOnly() || mIsResourceFolder) {
            mContentsComboBox->setEnabled(false);
        }

        // Kolab incidences-for annotation.
        // Show incidences-for combobox if the contents type can be changed (new folder),
        // or if it's set to calendar or task (existing folder)
        const bool folderTypeComboboxEnabled = (folderType == CollectionTypeUtil::ContentsTypeCalendar || folderType == CollectionTypeUtil::ContentsTypeTask);
        ++row;
        mIncidencesForComboBox = new IncidencesForWidget(this);
        gl->addWidget(mIncidencesForComboBox, row, 0, 1, 2);

        mIncidencesForComboBox->setCurrentIndex(incidencesFor);
        mIncidencesForComboBox->setEnabled(folderTypeComboboxEnabled);

        mSharedSeenFlagsCheckBox = new QCheckBox(this);
        mSharedSeenFlagsCheckBox->setText(i18n("Share unread state with all users"));
        mSharedSeenFlagsCheckBox->setChecked(sharedSeen);
        ++row;
        gl->addWidget(mSharedSeenFlagsCheckBox, row, 0, 1, 1);
        mSharedSeenFlagsCheckBox->setWhatsThis(
            i18n("If enabled, the unread state of messages in this folder will be "
                 "the same for all users having access to this folder. If disabled "
                 "(the default), every user with access to this folder has their "
                 "own unread state."));
    }

    topLayout->addStretch(100);   // eat all superfluous space
}

void CollectionGeneralPage::load(const Akonadi::Collection &collection)
{
    mFolderCollection = FolderCollection::forCollection(collection);
    init(collection);

    if (mNameEdit) {
        const QString displayName = collection.displayName();

        if (!mIsLocalSystemFolder || mIsResourceFolder) {
            mNameEdit->setText(displayName);
        }
    }

    // folder identity
    mIdentityComboBox->setCurrentIdentity(mFolderCollection->identity());
    mUseDefaultIdentityCheckBox->setChecked(mFolderCollection->useDefaultIdentity());

    // ignore new mail
    mNotifyOnNewMailCheckBox->setChecked(!Util::ignoreNewMailInFolder(collection));

    const bool keepInFolder = (mFolderCollection->canCreateMessages() &&
                               mFolderCollection->putRepliesInSameFolder());

    mKeepRepliesInSameFolderCheckBox->setChecked(keepInFolder);
    mKeepRepliesInSameFolderCheckBox->setEnabled(mFolderCollection->canCreateMessages());
    mHideInSelectionDialogCheckBox->setChecked(mFolderCollection->hideInSelectionDialog());

    if (mContentsComboBox) {
        const CollectionAnnotationsAttribute *annotationsAttribute =
            collection.attribute<CollectionAnnotationsAttribute>();

        if (annotationsAttribute) {
            const QMap<QByteArray, QByteArray> annotations = annotationsAttribute->annotations();
            if (annotations.contains(CollectionTypeUtil::kolabFolderType())) {
                CollectionTypeUtil collectionUtil;
                mContentsComboBox->setCurrentItem(
                    collectionUtil.typeNameFromKolabType(annotations[ CollectionTypeUtil::kolabFolderType() ]));
            }
        }
    }
}

void CollectionGeneralPage::save(Collection &collection)
{
    if (mNameEdit) {
        if (!mIsLocalSystemFolder) {
            const QString nameFolder(mNameEdit->text().trimmed());
            bool canRenameFolder =  !(nameFolder.startsWith(QLatin1Char('.')) ||
                                      nameFolder.endsWith(QLatin1Char('.')) ||
                                      nameFolder.contains(QLatin1Char('/')) ||
                                      nameFolder.isEmpty());

            if (mIsResourceFolder && (PimCommon::Util::isImapResource(collection.resource()))) {
                collection.setName(nameFolder);
                Akonadi::AgentInstance instance =
                    Akonadi::AgentManager::self()->instance(collection.resource());
                instance.setName(nameFolder);
            } else if (canRenameFolder) {
                if (collection.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
                        !collection.attribute<Akonadi::EntityDisplayAttribute>()->displayName().isEmpty()) {
                    collection.attribute<Akonadi::EntityDisplayAttribute>()->setDisplayName(
                        nameFolder);
                } else if (!nameFolder.isEmpty()) {
                    collection.setName(nameFolder);
                }
            }
        }
    }

    if (!mNotifyOnNewMailCheckBox->isChecked()) {
        Akonadi::NewMailNotifierAttribute *newMailNotifierAttr = collection.attribute<Akonadi::NewMailNotifierAttribute>(Akonadi::Collection::AddIfMissing);
        newMailNotifierAttr->setIgnoreNewMail(true);
    } else {
        collection.removeAttribute<Akonadi::NewMailNotifierAttribute>();
    }

    CollectionAnnotationsAttribute *annotationsAttribute =
        collection.attribute<CollectionAnnotationsAttribute>(Collection::AddIfMissing);

    QMap<QByteArray, QByteArray> annotations = annotationsAttribute->annotations();
    if (mSharedSeenFlagsCheckBox && mSharedSeenFlagsCheckBox->isEnabled()) {
        annotations[ CollectionTypeUtil::kolabSharedSeen() ] = mSharedSeenFlagsCheckBox->isChecked() ? "true" : "false";
    }

    CollectionTypeUtil collectionUtil;
    if (mIncidencesForComboBox && mIncidencesForComboBox->isEnabled()) {
        annotations[ CollectionTypeUtil::kolabIncidencesFor() ] =
            collectionUtil.incidencesForToString(
                static_cast<CollectionTypeUtil::IncidencesFor>(mIncidencesForComboBox->currentIndex())).toLatin1();
    }

    if (mContentsComboBox) {
        const CollectionTypeUtil::FolderContentsType type =
            collectionUtil.contentsTypeFromString(mContentsComboBox->currentText());

        const QByteArray kolabName = collectionUtil.kolabNameFromType(type);
        if (!kolabName.isEmpty()) {
            const QString iconName = collectionUtil.iconNameFromContentsType(type);
            Akonadi::EntityDisplayAttribute *attribute =
                collection.attribute<Akonadi::EntityDisplayAttribute>(Akonadi::Collection::AddIfMissing);
            attribute->setIconName(iconName);
            new Akonadi::CollectionModifyJob(collection);
            annotations[ CollectionTypeUtil::kolabFolderType() ] = kolabName;
        }
    }
    if (annotations.isEmpty()) {
        collection.removeAttribute<CollectionAnnotationsAttribute>();
    } else {
        annotationsAttribute->setAnnotations(annotations);
    }

    if (mFolderCollection) {
        mFolderCollection->setIdentity(mIdentityComboBox->currentIdentity());
        mFolderCollection->setUseDefaultIdentity(mUseDefaultIdentityCheckBox->isChecked());

        mFolderCollection->setPutRepliesInSameFolder(mKeepRepliesInSameFolderCheckBox->isChecked());
        mFolderCollection->setHideInSelectionDialog(mHideInSelectionDialogCheckBox->isChecked());
        mFolderCollection->writeConfig();
    }
}

void CollectionGeneralPage::slotIdentityCheckboxChanged()
{
    mIdentityComboBox->setEnabled(!mUseDefaultIdentityCheckBox->isChecked());
    if (mFolderCollection && mUseDefaultIdentityCheckBox->isChecked()) {
        mIdentityComboBox->setCurrentIdentity(mFolderCollection->fallBackIdentity());
    }
}

void CollectionGeneralPage::slotFolderContentsSelectionChanged(int)
{
    CollectionTypeUtil collectionUtil;
    const CollectionTypeUtil::FolderContentsType type =
        collectionUtil.contentsTypeFromString(mContentsComboBox->currentText());

    if (type != CollectionTypeUtil::ContentsTypeMail) {
        const QString message =
            i18n("You have configured this folder to contain groupware information. "
                 "That means that this folder will disappear once the configuration "
                 "dialog is closed.");

        KMessageBox::information(this, message);
    }

    const bool enable = (type == CollectionTypeUtil::ContentsTypeCalendar ||
                         type == CollectionTypeUtil::ContentsTypeTask);

    if (mIncidencesForComboBox) {
        mIncidencesForComboBox->setEnabled(enable);
    }
}

void CollectionGeneralPage::slotNameChanged(const QString &name)
{
#ifndef QT_NO_STYLE_STYLESHEET
    QString styleSheet;
    if (name.startsWith(QLatin1Char('.')) ||
            name.endsWith(QLatin1Char('.')) ||
            name.contains(QLatin1Char('/')) ||
            name.isEmpty()) {
        if (mColorName.isEmpty()) {
            const KColorScheme::BackgroundRole bgColorScheme(KColorScheme::NegativeBackground);
            KStatefulBrush bgBrush(KColorScheme::View, bgColorScheme);
            mColorName = bgBrush.brush(this).color().name();
        }
        styleSheet = QStringLiteral("QLineEdit{ background-color:%1 }").
                     arg(mColorName);
    }
    setStyleSheet(styleSheet);
#endif
}

