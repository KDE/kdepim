/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2009, 2010 Montel Laurent <montel@kde.org>

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

#include "folderselectiondialog.h"

#include "foldercollection.h"
#include "foldertreeview.h"
#include "foldertreewidget.h"
#include "foldertreewidgetproxymodel.h"
#include "kernel/mailkernel.h"

#include <Collection>
#include <CollectionCreateJob>
#include <EntityMimeTypeFilterModel>
#include <EntityTreeModel>

#include <QInputDialog>
#include <KLocalizedString>
#include <KMessageBox>
#include <QMenu>

#include <QVBoxLayout>
#include <QShowEvent>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

namespace MailCommon
{

class FolderSelectionDialog::FolderSelectionDialogPrivate
{
public:
    FolderSelectionDialogPrivate()
        : folderTreeWidget(0),
          mUser1Button(0),
          mOkButton(0),
          mNotAllowToCreateNewFolder(false),
          mUseGlobalSettings(true)
    {
    }
    FolderTreeWidget *folderTreeWidget;
    QPushButton *mUser1Button;
    QPushButton *mOkButton;
    bool mNotAllowToCreateNewFolder;
    bool mUseGlobalSettings;
};

FolderSelectionDialog::FolderSelectionDialog(QWidget *parent, SelectionFolderOptions options)
    : QDialog(parent), d(new FolderSelectionDialogPrivate())
{
    setObjectName(QLatin1String("folder dialog"));

    d->mNotAllowToCreateNewFolder = (options & FolderSelectionDialog::NotAllowToCreateNewFolder);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    d->mOkButton = buttonBox->button(QDialogButtonBox::Ok);
    d->mOkButton->setDefault(true);
    d->mOkButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &FolderSelectionDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FolderSelectionDialog::reject);


    if (!d->mNotAllowToCreateNewFolder) {
        d->mUser1Button = new QPushButton;
        buttonBox->addButton(d->mUser1Button, QDialogButtonBox::ActionRole);
        KGuiItem::assign(d->mUser1Button, KGuiItem(i18n("&New Subfolder..."), QLatin1String("folder-new"),
                     i18n("Create a new subfolder under the currently selected folder")));
    }

    FolderTreeWidget::TreeViewOptions opt = FolderTreeWidget::None;
    if (options & FolderSelectionDialog::ShowUnreadCount) {
        opt |= FolderTreeWidget::ShowUnreadCount;
    }
    opt |= FolderTreeWidget::UseDistinctSelectionModel;

    FolderTreeWidgetProxyModel::FolderTreeWidgetProxyModelOptions optReadableProxy =
        FolderTreeWidgetProxyModel::None;

    if (options & FolderSelectionDialog::HideVirtualFolder) {
        optReadableProxy |= FolderTreeWidgetProxyModel::HideVirtualFolder;
    }

    optReadableProxy |= FolderTreeWidgetProxyModel::HideSpecificFolder;

    if (options & FolderSelectionDialog::HideOutboxFolder) {
        optReadableProxy |= FolderTreeWidgetProxyModel::HideOutboxFolder;
    }

    d->folderTreeWidget = new FolderTreeWidget(this, 0, opt, optReadableProxy);
    d->folderTreeWidget->readConfig();
    d->folderTreeWidget->disableContextMenuAndExtraColumn();
    d->folderTreeWidget->folderTreeWidgetProxyModel()->setEnabledCheck((options & EnableCheck));
    //Necessary otherwise we overwrite tooltip config for all application
    d->folderTreeWidget->folderTreeView()->disableSaveConfig();
    d->folderTreeWidget->folderTreeView()->setTooltipsPolicy(FolderTreeWidget::DisplayNever);
#ifndef QT_NO_DRAGANDDROP
    d->folderTreeWidget->folderTreeView()->setDragDropMode(QAbstractItemView::NoDragDrop);
#endif
    mainLayout->addWidget(d->folderTreeWidget);
    mainLayout->addWidget(buttonBox);



    d->mOkButton->setEnabled(false);
    if (!d->mNotAllowToCreateNewFolder) {
        d->mUser1Button->setEnabled(false);
        connect(d->mUser1Button, &QPushButton::clicked, this, &FolderSelectionDialog::slotAddChildFolder);
        d->folderTreeWidget->folderTreeView()->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(d->folderTreeWidget->folderTreeView(), SIGNAL(customContextMenuRequested(QPoint)),
                SLOT(slotFolderTreeWidgetContextMenuRequested(QPoint)));

    }

    connect(d->folderTreeWidget->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotSelectionChanged()));
    connect(d->folderTreeWidget->folderTreeWidgetProxyModel(),
            SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(rowsInserted(QModelIndex,int,int)));

    connect(d->folderTreeWidget->folderTreeView(),
            SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(slotDoubleClick(QModelIndex)));

    d->mUseGlobalSettings = !(options & NotUseGlobalSettings);
    readConfig();

}

FolderSelectionDialog::~FolderSelectionDialog()
{
    writeConfig();
    delete d;
}

void FolderSelectionDialog::slotFolderTreeWidgetContextMenuRequested(const QPoint &pos)
{
    if (d->mUser1Button && d->mUser1Button->isEnabled() && d->folderTreeWidget->folderTreeView()->indexAt(pos).isValid()) {
        QMenu menu;
        menu.addAction(i18n("&New Subfolder..."), this, SLOT(slotAddChildFolder()));
        menu.exec(QCursor::pos());
    }
}

void FolderSelectionDialog::slotDoubleClick(const QModelIndex &index)
{
    Q_UNUSED(index);
    const bool hasSelectedCollection =
        (d->folderTreeWidget->selectionModel()->selectedIndexes().count() > 0);
    if (hasSelectedCollection) {
        accept();
    }
}

void FolderSelectionDialog::focusTreeView()
{
    d->folderTreeWidget->folderTreeView()->expandAll();
    d->folderTreeWidget->folderTreeView()->setFocus();
}

void FolderSelectionDialog::showEvent(QShowEvent *event)
{
    if (!event->spontaneous()) {
        focusTreeView();
        FolderTreeView *view = d->folderTreeWidget->folderTreeView();
        view->scrollTo(view->currentIndex());
    }
    QDialog::showEvent(event);
}

void FolderSelectionDialog::rowsInserted(const QModelIndex &, int, int)
{
    d->folderTreeWidget->folderTreeView()->expandAll();
}

bool FolderSelectionDialog::canCreateCollection(Akonadi::Collection &parentCol)
{
    parentCol = selectedCollection();
    if (!parentCol.isValid()) {
        return false;
    }

    if ((parentCol.rights() & Akonadi::Collection::CanCreateCollection) &&
            parentCol.contentMimeTypes().contains(Akonadi::Collection::mimeType())) {
        return true;
    }
    return false;
}

void FolderSelectionDialog::slotAddChildFolder()
{
    Akonadi::Collection parentCol;
    if (canCreateCollection(parentCol)) {
        const QString name = QInputDialog::getText(this,
                             i18nc("@title:window", "New Folder"),
                             i18nc("@label:textbox, name of a thing", "Name"));

        if (name.isEmpty()) {
            return;
        }

        Akonadi::Collection col;
        col.setName(name);
        col.parentCollection().setId(parentCol.id());
        Akonadi::CollectionCreateJob *job = new Akonadi::CollectionCreateJob(col);
        connect(job, &Akonadi::CollectionCreateJob::result, this, &FolderSelectionDialog::collectionCreationResult);
    }
}

void FolderSelectionDialog::collectionCreationResult(KJob *job)
{
    if (job->error()) {
        KMessageBox::error(
            this,
            i18n("Could not create folder: %1", job->errorString()),
            i18n("Folder creation failed"));
    }
}

void FolderSelectionDialog::slotSelectionChanged()
{
    const bool enablebuttons =
        (d->folderTreeWidget->selectionModel()->selectedIndexes().count() > 0);
    d->mOkButton->setEnabled(enablebuttons);

    if (!d->mNotAllowToCreateNewFolder) {
        Akonadi::Collection parent;
        d->mUser1Button->setEnabled(canCreateCollection(parent));
        if (parent.isValid()) {
            const QSharedPointer<FolderCollection> fd(FolderCollection::forCollection(parent, false));
            d->mOkButton->setEnabled(fd->canCreateMessages());
        }
    }
}

void FolderSelectionDialog::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    d->folderTreeWidget->setSelectionMode(mode);
}

QAbstractItemView::SelectionMode FolderSelectionDialog::selectionMode() const
{
    return d->folderTreeWidget->selectionMode();
}

Akonadi::Collection FolderSelectionDialog::selectedCollection() const
{
    return d->folderTreeWidget->selectedCollection();
}

void FolderSelectionDialog::setSelectedCollection(const Akonadi::Collection &collection)
{
    d->folderTreeWidget->selectCollectionFolder(collection);
}

Akonadi::Collection::List FolderSelectionDialog::selectedCollections() const
{
    return d->folderTreeWidget->selectedCollections();
}

static const char *myConfigGroupName = "FolderSelectionDialog";

void FolderSelectionDialog::readConfig()
{
    KConfigGroup group(KernelIf->config(), myConfigGroupName);

    const QSize size = group.readEntry("Size", QSize(500, 300));
    if (size.isValid()) {
        resize(size);
    }
    if (d->mUseGlobalSettings) {
        const Akonadi::Collection::Id id = SettingsIf->lastSelectedFolder();
        if (id > -1) {
            const Akonadi::Collection col = Kernel::self()->collectionFromId(id);
            d->folderTreeWidget->selectCollectionFolder(col);
        }
    }
}

void FolderSelectionDialog::writeConfig()
{
    KConfigGroup group(KernelIf->config(), myConfigGroupName);
    group.writeEntry("Size", size());

    if (d->mUseGlobalSettings) {
        Akonadi::Collection col = selectedCollection();
        if (col.isValid()) {
            SettingsIf->setLastSelectedFolder(col.id());
        }
    }
}

void FolderSelectionDialog::hideEvent(QHideEvent *)
{
    d->folderTreeWidget->clearFilter();
}

}

