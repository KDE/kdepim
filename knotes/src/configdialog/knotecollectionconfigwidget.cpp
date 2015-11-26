﻿/*
    Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "knotecollectionconfigwidget.h"
#include "attributes/showfoldernotesattribute.h"
#include <Akonadi/Notes/NoteUtils>
#include "notesharedglobalconfig.h"
#include "AkonadiWidgets/ManageAccountWidget"

#include <AkonadiCore/CollectionModifyJob>
#include <AkonadiCore/CollectionFilterProxyModel>
#include <KRecursiveFilterProxyModel>
#include <QInputDialog>

#include <AkonadiWidgets/CollectionRequester>
#include <AkonadiCore/ChangeRecorder>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/Collection>
#include <AkonadiWidgets/EntityTreeView>
#include <AkonadiCore/EntityDisplayAttribute>

#include <KMime/Message>

#include <KCheckableProxyModel>

#include <KLocalizedString>
#include <QPushButton>
#include <QLineEdit>
#include "knotes_debug.h"
#include <KMessageBox>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>

KNoteCollectionConfigWidget::KNoteCollectionConfigWidget(QWidget *parent)
    : QWidget(parent),
      mCanUpdateStatus(false)
{
    QHBoxLayout *mainLayout = new QHBoxLayout;
    setLayout(mainLayout);

    QTabWidget *tabWidget = new QTabWidget;
    mainLayout->addWidget(tabWidget);

    QWidget *collectionWidget = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    collectionWidget->setLayout(vbox);
    tabWidget->addTab(collectionWidget, i18n("Folders"));

    QLabel *label = new QLabel(i18n("Select which KNotes folders to show:"));
    vbox->addWidget(label);

    // Create a new change recorder.
    mChangeRecorder = new Akonadi::ChangeRecorder(this);
    mChangeRecorder->setMimeTypeMonitored(Akonadi::NoteUtils::noteMimeType());
    mChangeRecorder->fetchCollection(true);
    mChangeRecorder->setAllMonitored(true);

    mModel = new Akonadi::EntityTreeModel(mChangeRecorder, this);
    // Set the model to show only collections, not items.
    mModel->setItemPopulationStrategy(Akonadi::EntityTreeModel::NoItemPopulation);

    Akonadi::CollectionFilterProxyModel *mimeTypeProxy = new Akonadi::CollectionFilterProxyModel(this);
    mimeTypeProxy->setExcludeVirtualCollections(true);
    mimeTypeProxy->addMimeTypeFilters(QStringList() << Akonadi::NoteUtils::noteMimeType());
    mimeTypeProxy->setSourceModel(mModel);

    // Create the Check proxy model.
    mSelectionModel = new QItemSelectionModel(mimeTypeProxy);
    mCheckProxy = new KCheckableProxyModel(this);
    mCheckProxy->setSelectionModel(mSelectionModel);
    mCheckProxy->setSourceModel(mimeTypeProxy);

    connect(mModel, &Akonadi::EntityTreeModel::rowsInserted, this, &KNoteCollectionConfigWidget::slotCollectionsInserted);

    connect(mCheckProxy, &KCheckableProxyModel::dataChanged, this, &KNoteCollectionConfigWidget::slotDataChanged);
    mCollectionFilter = new KRecursiveFilterProxyModel(this);
    mCollectionFilter->setSourceModel(mCheckProxy);
    mCollectionFilter->setDynamicSortFilter(true);
    mCollectionFilter->setFilterCaseSensitivity(Qt::CaseInsensitive);

    QLineEdit *searchLine = new QLineEdit(this);
    searchLine->setPlaceholderText(i18n("Search..."));
    searchLine->setClearButtonEnabled(true);
    connect(searchLine, &QLineEdit::textChanged, this, &KNoteCollectionConfigWidget::slotSetCollectionFilter);

    vbox->addWidget(searchLine);

    mFolderView = new Akonadi::EntityTreeView(this);
    mFolderView->setDragEnabled(false);
    mFolderView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mFolderView->setAlternatingRowColors(true);
    vbox->addWidget(mFolderView);

    mFolderView->setModel(mCollectionFilter);
    connect(mFolderView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &KNoteCollectionConfigWidget::slotUpdateButtons);

    QHBoxLayout *hbox = new QHBoxLayout;
    vbox->addLayout(hbox);

    QPushButton *button = new QPushButton(i18n("&Select All"), this);
    connect(button, &QPushButton::clicked, this, &KNoteCollectionConfigWidget::slotSelectAllCollections);
    hbox->addWidget(button);

    button = new QPushButton(i18n("&Unselect All"), this);
    connect(button, &QPushButton::clicked, this, &KNoteCollectionConfigWidget::slotUnselectAllCollections);
    hbox->addWidget(button);
    hbox->addStretch(1);

    mRenameCollection = new QPushButton(i18n("Rename notes..."), this);
    connect(mRenameCollection, &QPushButton::clicked, this, &KNoteCollectionConfigWidget::slotRenameCollection);
    hbox->addWidget(mRenameCollection);

    vbox->addWidget(new QLabel(i18nc("@info", "Select the folder where the note will be saved:")));
    mDefaultSaveFolder = new Akonadi::CollectionRequester(Akonadi::Collection(NoteShared::NoteSharedGlobalConfig::self()->defaultFolder()));
    mDefaultSaveFolder->setMimeTypeFilter(QStringList() << Akonadi::NoteUtils::noteMimeType());
    mDefaultSaveFolder->setContentMimeTypes(QStringList() << QStringLiteral("application/x-vnd.akonadi.note")
                                            << QStringLiteral("text/x-vnd.akonadi.note")
                                            << QStringLiteral("inode/directory"));
    Akonadi::CollectionDialog::CollectionDialogOptions options;
    options |= Akonadi::CollectionDialog::AllowToCreateNewChildCollection;
    options |= Akonadi::CollectionDialog::KeepTreeExpanded;
    mDefaultSaveFolder->changeCollectionDialogOptions(options);
    connect(mDefaultSaveFolder, &Akonadi::CollectionRequester::collectionChanged, this, &KNoteCollectionConfigWidget::slotDataChanged);

    vbox->addWidget(mDefaultSaveFolder);

    QWidget *accountWidget = new QWidget;
    QVBoxLayout *vboxAccountWidget = new QVBoxLayout;
    accountWidget->setLayout(vboxAccountWidget);

    Akonadi::ManageAccountWidget *manageAccountWidget = new Akonadi::ManageAccountWidget(this);
    vboxAccountWidget->addWidget(manageAccountWidget);

    manageAccountWidget->setMimeTypeFilter(QStringList() << Akonadi::NoteUtils::noteMimeType());
    manageAccountWidget->setCapabilityFilter(QStringList() << QStringLiteral("Resource"));  // show only resources, no agents
    tabWidget->addTab(accountWidget, i18n("Accounts"));

    QTimer::singleShot(1000, this, &KNoteCollectionConfigWidget::slotUpdateCollectionStatus);
    slotUpdateButtons();
}

KNoteCollectionConfigWidget::~KNoteCollectionConfigWidget()
{

}

void KNoteCollectionConfigWidget::slotUpdateButtons()
{
    mRenameCollection->setEnabled(mFolderView->selectionModel()->hasSelection());
}

void KNoteCollectionConfigWidget::slotRenameCollection()
{
    const QModelIndexList rows = mFolderView->selectionModel()->selectedRows();

    if (rows.size() != 1) {
        return;
    }

    QModelIndex idx = rows.at(0);

    QString title = idx.data().toString();

    Akonadi::Collection col = idx.data(Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();
    Q_ASSERT(col.isValid());
    if (!col.isValid()) {
        return;
    }

    bool ok;
    const QString name = QInputDialog::getText(this, i18n("Rename Notes"),
                         i18n("Name:"), QLineEdit::Normal, title, &ok);

    if (ok) {
        if (col.hasAttribute<Akonadi::EntityDisplayAttribute>() &&
                !col.attribute<Akonadi::EntityDisplayAttribute>()->displayName().isEmpty()) {
            col.attribute<Akonadi::EntityDisplayAttribute>()->setDisplayName(name);
        } else if (!name.isEmpty()) {
            col.setName(name);
        }

        Akonadi::CollectionModifyJob *job = new Akonadi::CollectionModifyJob(col, this);
        connect(job, &Akonadi::CollectionModifyJob::result, this, &KNoteCollectionConfigWidget::slotCollectionModifyFinished);
        job->start();
    }
}

void KNoteCollectionConfigWidget::slotCollectionModifyFinished(KJob *job)
{
    if (job->error()) {
        KMessageBox::error(this, i18n("An error was occurred during renaming: %1", job->errorString()), i18n("Rename note"));
    }
}

void KNoteCollectionConfigWidget::slotDataChanged()
{
    Q_EMIT emitChanged(true);
}

void KNoteCollectionConfigWidget::slotSetCollectionFilter(const QString &filter)
{
    mCollectionFilter->setFilterWildcard(filter);
    mFolderView->expandAll();
}

void KNoteCollectionConfigWidget::slotUpdateCollectionStatus()
{
    mCanUpdateStatus = true;
    updateStatus(QModelIndex());
}

void KNoteCollectionConfigWidget::slotSelectAllCollections()
{
    forceStatus(QModelIndex(), true);
    Q_EMIT emitChanged(true);
}

void KNoteCollectionConfigWidget::slotUnselectAllCollections()
{
    forceStatus(QModelIndex(), false);
    Q_EMIT emitChanged(true);
}

void KNoteCollectionConfigWidget::updateStatus(const QModelIndex &parent)
{
    if (!mCanUpdateStatus) {
        return;
    }

    const int nbCol = mCheckProxy->rowCount(parent);
    for (int i = 0; i < nbCol; ++i) {
        const QModelIndex child = mCheckProxy->index(i, 0, parent);

        const Akonadi::Collection collection =
            mCheckProxy->data(child, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

        NoteShared::ShowFolderNotesAttribute *attr = collection.attribute<NoteShared::ShowFolderNotesAttribute>();
        if (attr) {
            mCheckProxy->setData(child, Qt::Checked, Qt::CheckStateRole);
        }
        updateStatus(child);
    }
}

void KNoteCollectionConfigWidget::forceStatus(const QModelIndex &parent, bool status)
{
    const int nbCol = mCheckProxy->rowCount(parent);
    for (int i = 0; i < nbCol; ++i) {
        const QModelIndex child = mCheckProxy->index(i, 0, parent);
        mCheckProxy->setData(child, status ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
        forceStatus(child, status);
    }
}

void KNoteCollectionConfigWidget::slotCollectionsInserted(const QModelIndex &, int, int)
{
    mFolderView->expandAll();
}

void KNoteCollectionConfigWidget::save()
{
    updateCollectionsRecursive(QModelIndex());
    Akonadi::Collection col = mDefaultSaveFolder->collection();
    if (col.isValid()) {
        NoteShared::NoteSharedGlobalConfig::self()->setDefaultFolder(col.id());
        NoteShared::NoteSharedGlobalConfig::self()->save();
    }
}

void KNoteCollectionConfigWidget::updateCollectionsRecursive(const QModelIndex &parent)
{
    const int nbCol = mCheckProxy->rowCount(parent);
    for (int i = 0; i < nbCol; ++i) {
        const QModelIndex child = mCheckProxy->index(i, 0, parent);

        Akonadi::Collection collection =
            mCheckProxy->data(child, Akonadi::EntityTreeModel::CollectionRole).value<Akonadi::Collection>();

        NoteShared::ShowFolderNotesAttribute *attr = collection.attribute<NoteShared::ShowFolderNotesAttribute>();
        Akonadi::CollectionModifyJob *modifyJob = 0;
        const bool selected = (mCheckProxy->data(child, Qt::CheckStateRole).toInt() != 0);
        if (selected && !attr) {
            attr = collection.attribute<NoteShared::ShowFolderNotesAttribute>(Akonadi::Collection::AddIfMissing);
            modifyJob = new Akonadi::CollectionModifyJob(collection);
            modifyJob->setProperty("AttributeAdded", true);
        } else if (!selected && attr) {
            collection.removeAttribute<NoteShared::ShowFolderNotesAttribute>();
            modifyJob = new Akonadi::CollectionModifyJob(collection);
            modifyJob->setProperty("AttributeAdded", false);
        }

        if (modifyJob) {
            connect(modifyJob, &Akonadi::CollectionModifyJob::finished, this, &KNoteCollectionConfigWidget::slotModifyJobDone);
        }
        updateCollectionsRecursive(child);
    }
}

void KNoteCollectionConfigWidget::slotModifyJobDone(KJob *job)
{
    Akonadi::CollectionModifyJob *modifyJob = qobject_cast<Akonadi::CollectionModifyJob *>(job);
    if (modifyJob && job->error()) {
        if (job->property("AttributeAdded").toBool()) {
            qCWarning(KNOTES_LOG) << "Failed to append ShowFolderNotesAttribute to collection"
                                  << modifyJob->collection().id() << ":"
                                  << job->errorString();
        } else {
            qCWarning(KNOTES_LOG) << "Failed to remove ShowFolderNotesAttribute from collection"
                                  << modifyJob->collection().id() << ":"
                                  << job->errorString();
        }
    }
}

