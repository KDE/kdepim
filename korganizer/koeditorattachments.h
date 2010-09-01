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
#include <tqmap.h>
#include <kdialogbase.h>
#include <kmimetype.h>
#include <kurl.h>
#include <kiconview.h>

#include <set>

class AttachmentListItem;
class AttachmentIconView;

namespace KCal {
class Incidence;
class Attachment;
}

class TQCheckBox;
class TQIconViewItem;
class TQLabel;
class TQMimeSource;
class TQPushButton;
class TQPopupMenu;

class KAction;
class KLineEdit;
class KURLRequester;
class KTempDir;

class AttachmentEditDialog : public KDialogBase
{
  Q_OBJECT
  public:
    AttachmentEditDialog( AttachmentListItem *item, TQWidget *parent=0 );

    void accept();

  protected slots:
    void urlSelected( const TQString &url );
    void urlChanged( const TQString & url );
    virtual void slotApply();

  private:
    friend class KOEditorAttachments;
    KMimeType::Ptr mMimeType;
    AttachmentListItem *mItem;
    TQLabel *mTypeLabel, *mIcon;
    TQCheckBox *mInline;
    KLineEdit *mLabelEdit;
    KURLRequester *mURLRequester;
};

class KOEditorAttachments : public QWidget
{
    Q_OBJECT
  public:
    KOEditorAttachments( int spacing = 8, TQWidget *parent = 0,
                         const char *name = 0 );
    ~KOEditorAttachments();

    void addUriAttachment( const TQString &uri,
                           const TQString &mimeType = TQString(),
                           const TQString &label = TQString(),
                           bool inLine = false );
    void addAttachment( KCal::Attachment *attachment );
    void addDataAttachment( const TQByteArray &data,
                            const TQString &mimeType = TQString(),
                            const TQString &label = TQString() );

    /** Set widgets to default values */
    void setDefaults();
    /** Read event object and setup widgets accordingly */
    void readIncidence( KCal::Incidence * );
    /** Write event settings to event object */
    void writeIncidence( KCal::Incidence * );

    bool hasAttachments();

  protected slots:
    void showAttachment( TQIconViewItem *item );
    void saveAttachment( TQIconViewItem *item );
    void slotAdd();
    void slotAddData();
    void slotEdit();
    void slotRemove();
    void slotShow();
    void slotSaveAs();
    void dragEnterEvent( TQDragEnterEvent *event );
    void dragMoveEvent( TQDragMoveEvent *event );
    void dropEvent( TQDropEvent *event );
    void slotCopy();
    void slotCut();
    void slotPaste();
    void selectionChanged();
    void contextMenu( TQIconViewItem* item, const TQPoint &pos );

  signals:
    void openURL( const KURL &url );

  protected:
    enum {
      DRAG_COPY = 0,
      DRAG_LINK = 1,
      DRAG_CANCEL = 2
    };

  private:
    friend class AttachmentIconView;
    void handlePasteOrDrop( TQMimeSource* source );
    TQString randomString( int length ) const;
    AttachmentIconView *mAttachments;
    TQPushButton *mRemoveBtn;
    TQPopupMenu *mContextMenu, *mAddMenu;
    KAction *mOpenAction;
    KAction *mSaveAsAction;
    KAction *mCopyAction;
    KAction *mCutAction;
    KAction *mDeleteAction;
    KAction *mEditAction;
};


class AttachmentIconView : public KIconView
{
  Q_OBJECT

  friend class KOEditorAttachments;
  public:
    AttachmentIconView( KOEditorAttachments* parent=0 );
    KURL tempFileForAttachment( KCal::Attachment *attachment );
    TQDragObject *mimeData();
    ~AttachmentIconView();

  protected:
    TQDragObject * dragObject();

    void dragMoveEvent( TQDragMoveEvent *event );
    void contentsDragMoveEvent( TQDragMoveEvent *event );
    void contentsDragEnterEvent( TQDragEnterEvent *event );
    void dragEnterEvent( TQDragEnterEvent *event );

  protected slots:

    void handleDrop( TQDropEvent *event, const TQValueList<TQIconDragItem> & list );

  private:
    std::set<KTempDir*> mTempDirs;
    TQMap<KCal::Attachment *, KURL> mTempFiles;
    KOEditorAttachments* mParent;
};

#endif
