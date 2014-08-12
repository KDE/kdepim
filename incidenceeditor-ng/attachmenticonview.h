/*
  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#ifndef INCIDENCEEDITOR_ATTACHMENTICONVIEW_H
#define INCIDENCEEDITOR_ATTACHMENTICONVIEW_H

#include "incidenceeditors_ng_export.h"

#include <KCalCore/Attachment>

#include <KMimeType>
#include <KUrl>

#include <QListWidget>

namespace IncidenceEditorNG {

class INCIDENCEEDITORS_NG_EXPORT AttachmentIconView : public QListWidget
{
  friend class EditorAttachments;
  public:
    explicit AttachmentIconView( QWidget *parent = 0 );

    QMimeData *mimeData() const;
    KUrl tempFileForAttachment( const KCalCore::Attachment::Ptr &attachment ) const;

  protected:
    QMimeData * mimeData( const QList<QListWidgetItem*> items ) const;
    void startDrag( Qt::DropActions supportedActions );
    void keyPressEvent( QKeyEvent *event );

  private:
    mutable QHash<KCalCore::Attachment::Ptr, KUrl> mTempFiles;
};

class INCIDENCEEDITORS_NG_EXPORT AttachmentIconItem : public QListWidgetItem
{
  public:
    AttachmentIconItem( const KCalCore::Attachment::Ptr &att, QListWidget *parent );
    ~AttachmentIconItem();

    KCalCore::Attachment::Ptr attachment() const;
    const QString uri() const;
    const QString savedUri() const;
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
    KCalCore::Attachment::Ptr mAttachment;
    QString mSaveUri;
};

}

#endif // INCIDENCEEDITOR_ATTACHMENTICONVIEW_H
