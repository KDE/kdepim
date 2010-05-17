/*
  This file is part of KOrganizer.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.

  NOTE: May, 2010. Extracted this code from
        kdepim/incidenceeditors/editorattachments.{h,cpp}
*/

#ifndef ATTACHMENTICONVIEW_H
#define ATTACHMENTICONVIEW_H

#include <QtGui/QListWidget>

#include <KDE/KMimeType>

class KUrl;

namespace KCal {
class Attachment;
}

namespace IncidenceEditorsNG {

class AttachmentIconView : public QListWidget
{
  friend class EditorAttachments;
  public:
    AttachmentIconView( QWidget *parent );

    KUrl tempFileForAttachment( KCal::Attachment *attachment ) const;

  protected:
    QMimeData* mimeData( const QList<QListWidgetItem*> items ) const;
    QMimeData* mimeData() const;
    void startDrag( Qt::DropActions supportedActions );
    void keyPressEvent( QKeyEvent* event );

  private:
    mutable QHash<KCal::Attachment *, KUrl> mTempFiles;
};

class AttachmentIconItem : public QListWidgetItem
{
  public:
    AttachmentIconItem( KCal::Attachment *att, QListWidget *parent );
    ~AttachmentIconItem();

    KCal::Attachment *attachment() const;
    const QString uri() const;
    void setUri( const QString &uri );

    using QListWidgetItem::setData;

    void setData( const QByteArray &data );

    const QString mimeType() const;
    void setMimeType( const QString &mime );

    const QString label() const;
    void setLabel( const QString &description );

    bool isBinary() const;

    static QPixmap icon( KMimeType::Ptr mimeType, const QString &uri, bool binary = false );
    QPixmap icon() const;

    void readAttachment();

  private:
    KCal::Attachment *mAttachment;
};

}

#endif // ATTACHMENTICONVIEW_H
