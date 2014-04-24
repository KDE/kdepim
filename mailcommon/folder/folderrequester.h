/*
 * Copyright (c) 2004 Carsten Burghardt <burghardt@kde.org>
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

#ifndef MAILCOMMON_FOLDERREQUESTER_H
#define MAILCOMMON_FOLDERREQUESTER_H

#include "mailcommon_export.h"

#include <Collection>

#include <KLineEdit>

#include <QWidget>

class QKeyEvent;

class KJob;

namespace MailCommon {

/**
 * A widget that contains a KLineEdit which shows the current folder
 * and a button that fires a FolderSelectionDialog
 * The dialog is set to disable readonly folders by default
 * Search folders are excluded
 *
 * @todo This should be cleaned up and go into libakonadi. This includes:
 * - s/Folder/Collection/g
 * - Use Akonadi::CollectionDialog instead of MailCommon::FolderSelectionDialog
 *  - merge that into CollectionDialog
 *  - or allow to replace the built-in dialog by your own
 * - Allow to pass in an existing ETM, to remove the Kernel dependency
 */
class MAILCOMMON_EXPORT FolderRequester: public QWidget
{
  Q_OBJECT

  public:
    /**
     * Constructor
     * @param parent the parent widget
     */
    explicit FolderRequester( QWidget *parent = 0 );
    virtual ~FolderRequester();

    /**
     * Returns the selected collection.
     */
    Akonadi::Collection collection() const;

    /**
     * Presets the folder to the collection @p collection.
     * Disable fetchcollection when not necessary @p fetchCollection
     */
    void setCollection(const Akonadi::Collection &collection , bool fetchCollection = true );

    /**
     * Returns @c true if there's a valid collection set on this widget.
     */
    bool hasCollection() const;

    /**
     * Sets if readonly folders should be disabled.
     * Be aware that if you disable this the user can also select the
     * 'Local Folders' folder which has no valid folder associated
     */
    void setMustBeReadWrite( bool readwrite );

    void setShowOutbox( bool show );

    void setNotAllowToCreateNewFolder( bool notCreateNewFolder );

  protected slots:
    /**
     * Opens the folder dialog.
     */
    void slotOpenDialog();

    /**
     * Updates the information we have about the current folder.
     */
    void slotCollectionsReceived( KJob * );

  signals:
    /**
     * Emitted when the folder changed.
     */
    void folderChanged( const Akonadi::Collection & );

  protected:
    /** Capture space key to open the dialog */
    void keyPressEvent( QKeyEvent *e );
    void setCollectionFullPath( const Akonadi::Collection &col );

  protected:
    Akonadi::Collection mCollection;
    KLineEdit *mEdit;
    bool mMustBeReadWrite;
    bool mShowOutbox;
    bool mNotCreateNewFolder;
};

}

#endif
