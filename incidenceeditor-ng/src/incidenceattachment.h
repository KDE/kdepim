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

#ifndef INCIDENCEEDITOR_INCIDENCEATTACHMENT_H
#define INCIDENCEEDITOR_INCIDENCEATTACHMENT_H

#include "incidenceeditor-ng.h"
class QUrl;
class KJob;
namespace Ui
{
class EventOrTodoDesktop;
}

class QMenu;

class QListWidgetItem;
class QMimeData;
class QAction;
namespace IncidenceEditorNG
{

class AttachmentIconView;

class IncidenceAttachment : public IncidenceEditor
{
    Q_OBJECT
public:
    using IncidenceEditorNG::IncidenceEditor::save; // So we don't trigger -Woverloaded-virtual
    using IncidenceEditorNG::IncidenceEditor::load; // So we don't trigger -Woverloaded-virtual

    explicit IncidenceAttachment(Ui::EventOrTodoDesktop *ui);

    ~IncidenceAttachment();

    void load(const KCalCore::Incidence::Ptr &incidence) Q_DECL_OVERRIDE;
    void save(const KCalCore::Incidence::Ptr &incidence) Q_DECL_OVERRIDE;
    bool isDirty() const Q_DECL_OVERRIDE;

    int attachmentCount() const;

Q_SIGNALS:
    void attachmentCountChanged(int newCount);

private Q_SLOTS:
    void addAttachment();
    void copyToClipboard(); /// Copies selected items to clip board
    void cutToClipboard();  /// Copies selected items to clipboard and removes them from the list
    void editSelectedAttachments();
    void openURL(const QUrl &url);
    void pasteFromClipboard();
    void removeSelectedAttachments();
    void saveAttachment(QListWidgetItem *item);
    void saveSelectedAttachments();
    void showAttachment(QListWidgetItem *item);
    void showContextMenu(const QPoint &pos);
    void showSelectedAttachments();
    void slotItemRenamed(QListWidgetItem *item);
    void slotSelectionChanged();
    void downloadComplete(KJob *);

private:
    //     void addAttachment( KCalCore::Attachment *attachment );
    void addDataAttachment(const QByteArray &data,
                           const QString &mimeType = QString(),
                           const QString &label = QString());
    void addUriAttachment(const QString &uri,
                          const QString &mimeType = QString(),
                          const QString &label = QString(),
                          bool inLine = false);
    void handlePasteOrDrop(const QMimeData *mimeData);
    void setupActions();
    void setupAttachmentIconView();

private:
    AttachmentIconView *mAttachmentView;
    Ui::EventOrTodoDesktop *mUi;

    QMenu *mPopupMenu;
    QAction *mOpenAction;
    QAction *mSaveAsAction;
#ifndef QT_NO_CLIPBOARD
    QAction *mCopyAction;
    QAction *mCutAction;
#endif
    QAction *mDeleteAction;
    QAction *mEditAction;
};

}

#endif
