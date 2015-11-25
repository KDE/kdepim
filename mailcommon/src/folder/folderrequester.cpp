/*
 * Copyright (c) 2004 Carsten Burghardt <burghardt@kde.org>
 * Copyright (c) 2009-2015 Montel Laurent <montel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "folderrequester.h"
#include "folderselectiondialog.h"
#include "util/mailutil.h"
#include "kernel/mailkernel.h"

#include <CollectionFetchJob>

#include <QDialog>
#include <KIconLoader>
#include <KLineEdit>
#include <KLocalizedString>
#include <QIcon>

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QToolButton>
#include <KConfigGroup>

namespace MailCommon
{

class FolderRequesterPrivate
{
public:
    FolderRequesterPrivate()
        : mEdit(Q_NULLPTR),
          mMustBeReadWrite(true),
          mShowOutbox(true),
          mNotCreateNewFolder(false)
    {
    }
    Akonadi::Collection mCollection;
    KLineEdit *mEdit;
    bool mMustBeReadWrite;
    bool mShowOutbox;
    bool mNotCreateNewFolder;
};

FolderRequester::FolderRequester(QWidget *parent)
    : QWidget(parent),
      d(new MailCommon::FolderRequesterPrivate)
{
    QHBoxLayout *hlay = new QHBoxLayout(this);
    hlay->setContentsMargins(0, 0, 0, 0);

    d->mEdit = new KLineEdit(this);
    d->mEdit->setPlaceholderText(i18n("Select Folder"));
    d->mEdit->setTrapReturnKey(true);
    d->mEdit->setReadOnly(true);
    hlay->addWidget(d->mEdit);

    QToolButton *button = new QToolButton(this);
    button->setIcon(QIcon::fromTheme(QStringLiteral("folder")));
    button->setIconSize(QSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall));
    hlay->addWidget(button);
    connect(button, &QToolButton::clicked, this, &FolderRequester::slotOpenDialog);

    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                              QSizePolicy::Fixed));
    setFocusPolicy(Qt::StrongFocus);
}

//-----------------------------------------------------------------------------
void FolderRequester::slotOpenDialog()
{
    FolderSelectionDialog::SelectionFolderOptions options = FolderSelectionDialog::EnableCheck;
    options |= FolderSelectionDialog::HideVirtualFolder;
    options |= FolderSelectionDialog::NotUseGlobalSettings;
    if (d->mNotCreateNewFolder) {
        options |= FolderSelectionDialog::NotAllowToCreateNewFolder;
    }
    if (!d->mShowOutbox) {
        options |= FolderSelectionDialog::HideOutboxFolder;
    }

    QScopedPointer<FolderSelectionDialog> dlg(
        new FolderSelectionDialog(this, options));

    dlg->setWindowTitle(i18n("Select Folder"));
    dlg->setModal(false);
    dlg->setSelectedCollection(d->mCollection);

    if (dlg->exec() && dlg) {
        setCollection(dlg->selectedCollection(), false);
    }
}

//-----------------------------------------------------------------------------
FolderRequester::~FolderRequester()
{
    delete d;
}

Akonadi::Collection FolderRequester::collection() const
{
    return d->mCollection;
}

void FolderRequester::setCollectionFullPath(const Akonadi::Collection &col)
{
    if (KernelIf->collectionModel()) {
        d->mEdit->setText(Util::fullCollectionPath(col));
    } else {
        d->mEdit->clear();
    }
}

//-----------------------------------------------------------------------------
void FolderRequester::setCollection(const Akonadi::Collection &collection, bool fetchCollection)
{
    d->mCollection = collection;
    if (d->mCollection.isValid()) {
        if (fetchCollection) {
            Akonadi::CollectionFetchJob *job =
                new Akonadi::CollectionFetchJob(d->mCollection, Akonadi::CollectionFetchJob::Base, this);

            connect(job, &Akonadi::CollectionFetchJob::result, this, &FolderRequester::slotCollectionsReceived);
        } else {
            setCollectionFullPath(d->mCollection);
        }
    } else if (!d->mMustBeReadWrite) {   // the Local Folders root node was selected
        d->mEdit->setText(i18n("Local Folders"));
    }

    Q_EMIT folderChanged(d->mCollection);
}

void FolderRequester::slotCollectionsReceived(KJob *job)
{
    if (job->error()) {
        d->mCollection = Akonadi::Collection();
        d->mEdit->setText(i18n("Please select a folder"));
        return;
    }

    const Akonadi::CollectionFetchJob *fetchJob = qobject_cast<Akonadi::CollectionFetchJob *>(job);
    const Akonadi::Collection::List collections = fetchJob->collections();

    if (!collections.isEmpty()) {
        const Akonadi::Collection collection = collections.first();
        // in case this is still the collection we are interested in, update
        if (collection.id() == d->mCollection.id()) {
            d->mCollection = collection;
            setCollectionFullPath(collection);
        }
    } else {
        // the requested collection doesn't exists anymore
        d->mCollection = Akonadi::Collection();
        d->mEdit->setText(i18n("Please select a folder"));
    }
}

bool FolderRequester::hasCollection() const
{
    return d->mCollection.isValid();
}

//-----------------------------------------------------------------------------
void FolderRequester::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Space) {
        slotOpenDialog();
    } else {
        e->ignore();
    }
}

void FolderRequester::setMustBeReadWrite(bool readwrite)
{
    d->mMustBeReadWrite = readwrite;
}

void FolderRequester::setShowOutbox(bool show)
{
    d->mShowOutbox = show;
}

void FolderRequester::setNotAllowToCreateNewFolder(bool notCreateNewFolder)
{
    d->mNotCreateNewFolder = notCreateNewFolder;
}

} // namespace MailCommon

