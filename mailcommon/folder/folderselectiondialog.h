/* -*- mode: C++; c-file-style: "gnu" -*-

  Copyright (c) 2009 Montel Laurent <montel@kde.org>

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

#ifndef MAILCOMMON_FOLDERSELECTIONDIALOG_H
#define MAILCOMMON_FOLDERSELECTIONDIALOG_H

#include "mailcommon_export.h"

#include <Collection>

#include <KDialog>

#include <QAbstractItemView>

class KJob;

namespace MailCommon {

/**
 * A dialog that lets the user select a folder.
 * TODO: Move most of this to Akonadi::CollectionDialog
 */
class MAILCOMMON_EXPORT FolderSelectionDialog : public KDialog
{
    Q_OBJECT

public:
    enum SelectionFolderOption {
        None = 0,
        EnableCheck = 1,
        ShowUnreadCount = 2,
        HideVirtualFolder = 4,
        NotAllowToCreateNewFolder = 8,
        HideOutboxFolder = 16,
        NotUseGlobalSettings = 64
    };
    Q_DECLARE_FLAGS( SelectionFolderOptions, SelectionFolderOption )

    FolderSelectionDialog( QWidget *parent, FolderSelectionDialog::SelectionFolderOptions options );
    ~FolderSelectionDialog();

    void setSelectionMode( QAbstractItemView::SelectionMode mode );
    QAbstractItemView::SelectionMode selectionMode() const;

    Akonadi::Collection selectedCollection() const;
    void setSelectedCollection( const Akonadi::Collection &collection );

    Akonadi::Collection::List selectedCollections() const;

private slots:
    void slotSelectionChanged();
    void slotAddChildFolder();
    void collectionCreationResult( KJob * );
    void rowsInserted( const QModelIndex &col, int, int );
    void slotDoubleClick(const QModelIndex&);
    void slotFolderTreeWidgetContextMenuRequested(const QPoint&);

protected:
    void focusTreeView();
    void readConfig();
    void writeConfig();
    bool canCreateCollection( Akonadi::Collection &parentCol );

    /*reimp*/
    void hideEvent( QHideEvent * );

    /*reimp*/
    void showEvent( QShowEvent * );

private:
    class FolderSelectionDialogPrivate;
    FolderSelectionDialogPrivate *const d;
};

}

#endif
