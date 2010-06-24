/*
    Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEATTACHMENTEDITOR_H
#define INCIDENCEATTACHMENTEDITOR_H

#include "incidenceeditor-ng.h"

class KAction;
class KMenu;
class QListWidgetItem;
class QMimeData;

namespace Ui {
class EventOrTodoDesktop;
}

namespace IncidenceEditorsNG {

class AttachmentIconView;

class IncidenceAttachment : public IncidenceEditor
{
  Q_OBJECT
  public:
    IncidenceAttachment( Ui::EventOrTodoDesktop *ui = 0 );

    virtual void load( KCal::Incidence::ConstPtr incidence );
    virtual void save( KCal::Incidence::Ptr incidence );
    virtual bool isDirty() const;
    
    int attachmentCount() const;

  signals:
    void attachmentCountChanged( int newCount );

  private slots:
    void addAttachment();
    void copyToClipboard(); /// Copies selected items to clip board
    void cutToClipboard();  /// Copies selected items to clipboard and removes them from the list
    void editSelectedAttachments();
    void openURL( const KUrl &url );
    void pasteFromClipboard();
    void removeSelectedAttachments();
    void saveAttachment( QListWidgetItem *item );
    void saveSelectedAttachments();
    void showAttachment( QListWidgetItem *item );
    void showContextMenu( const QPoint &pos );
    void showSelectedAttachments();
    void slotItemRenamed ( QListWidgetItem * item );
    void slotSelectionChanged();

  private:
    //     void addAttachment( KCal::Attachment *attachment );
    void addDataAttachment( const QByteArray &data,
                            const QString &mimeType = QString(),
                            const QString &label = QString() );
    void addUriAttachment( const QString &uri,
                           const QString &mimeType = QString(),
                           const QString &label = QString(),
                           bool inLine = false );
    void handlePasteOrDrop( const QMimeData *mimeData );
    void setupActions();
    void setupAttachmentIconView();
    
  private:
    AttachmentIconView *mAttachmentView;
    Ui::EventOrTodoDesktop *mUi;

    KMenu *mPopupMenu;
    KAction *mOpenAction;
    KAction *mSaveAsAction;
    KAction *mCopyAction;
    KAction *mCutAction;
    KAction *mDeleteAction;
    KAction *mEditAction;
};

}

#endif // INCIDENCEATTACHMENTEDITOR_H
