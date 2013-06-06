/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#ifndef ATTACHMENTEDITOR_H
#define ATTACHMENTEDITOR_H

#include <QtCore/QObject>

namespace MessageComposer {
class AttachmentModel;
class AttachmentControllerBase;
}

class KActionCollection;
class QAbstractItemModel;
class QAction;
class QItemSelectionModel;

/**
 * @short The C++ part of the attachment editor for mobile apps.
 *
 * This class encapsulates the logic of the attachment viewing/editing
 * and the UI is provided by AttachmentEditor.qml.
 */
class AttachmentEditor : public QObject
{
  Q_OBJECT

  public:
    /**
     * Creates a new attachment editor.
     *
     * @param actionCollection The action collection to register the manipulation
     *                         actions (e.g. add, delete, sign, encrypt) at.
     * @param model The attachment model.
     * @param controller The attachment controller to use for composing the message.
     * @param parent The parent object.
     */
    AttachmentEditor( KActionCollection *actionCollection, MessageComposer::AttachmentModel *model,
                      MessageComposer::AttachmentControllerBase *controller, QObject *parent = 0 );

  public Q_SLOTS:
    /**
     * Sets the row of the attachment the user has selected in the UI.
     */
    void setRowSelected( int row );

  private Q_SLOTS:
    void selectionChanged();
    void signAttachment( bool value );
    void encryptAttachment( bool value );

  private:
    MessageComposer::AttachmentModel *mModel;
    QItemSelectionModel *mSelectionModel;

    MessageComposer::AttachmentControllerBase *mAttachmentController;
    QAction *mAddAction;
    QAction *mDeleteAction;
    QAction *mSignAction;
    QAction *mEncryptAction;
};

#endif
