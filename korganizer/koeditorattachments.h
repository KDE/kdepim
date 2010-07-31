/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef KOEDITORATTACHMENTS_H
#define KOEDITORATTACHMENTS_H

#include <tqwidget.h>
#include <kurl.h>

namespace KCal {
class Incidence;
class Attachment;
}

class QIconViewItem;
class AttachmentIconView;
class QMimeSource;
class QPushButton;
class QPopupMenu;
class KAction;

class KOEditorAttachments : public QWidget
{
    Q_OBJECT
  public:
    KOEditorAttachments( int spacing = 8, TQWidget *parent = 0,
                         const char *name = 0 );
    ~KOEditorAttachments();

    void addAttachment( const KURL &uri,
                        const TQString &mimeType = TQString::null, bool asUri = true );
    void addAttachment( KCal::Attachment *attachment );

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readIncidence( KCal::Incidence * );
    /** Write event settings to event object */
    void writeIncidence( KCal::Incidence * );

    bool hasAttachments();

  protected slots:
    void showAttachment( TQIconViewItem *item );
    void slotAdd();
    void slotAddData();
    void slotEdit();
    void slotRemove();
    void slotShow();
    void dragEnterEvent( TQDragEnterEvent *event );
    void dropEvent( TQDropEvent *event );
    void slotCopy();
    void slotCut();
    void slotPaste();
    void selectionChanged();
    void contextMenu( TQIconViewItem* item, const TQPoint &pos );
  signals:
    void openURL( const KURL &url );

  private:
    friend class AttachmentIconView;
    void handlePasteOrDrop( TQMimeSource* source );

    AttachmentIconView *mAttachments;
    TQPushButton *mRemoveBtn;
    TQPopupMenu *mContextMenu, *mAddMenu;
    KAction *mOpenAction, *mCopyAction, *mCutAction;
};

#endif
