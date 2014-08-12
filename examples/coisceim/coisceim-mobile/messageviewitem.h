/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef MESSAGEVIEWER_MESSAGEVIEWITEM_H
#define MESSAGEVIEWER_MESSAGEVIEWITEM_H

#include <mobile/lib/declarativeakonadiitem.h>
#include <KUrl>
class QSortFilterProxyModel;
class KUrl;

namespace Akonadi {
class Item;
}

namespace MessageViewer {

class Viewer;

class MessageViewItem : public DeclarativeAkonadiItem
{
  Q_OBJECT
  Q_PROPERTY( QString splashMessage READ splashMessage WRITE setSplashMessage )
  Q_PROPERTY( QObject* attachmentModel READ attachmentModel CONSTANT )
  Q_PROPERTY( QString messagePath READ messagePath WRITE setMessagePath )

  public:
    explicit MessageViewItem( QDeclarativeItem *parent = 0 );
    ~MessageViewItem();

    QString splashMessage() const;
    void setSplashMessage( const QString &message );

    QString messagePath() const;
    void setMessagePath( const QString &messagePath );

    QObject* attachmentModel() const;

    Viewer *viewer();

    virtual qint64 itemId() const;
    virtual void setItemId( qint64 id );
    virtual void setItem( const Akonadi::Item &item );

    virtual void scrollDown( int dist );
    virtual void scrollUp( int dist );

  public slots:
    void saveAllAttachments();
    void copyAllToClipboard();

  signals:
    /** Emitted for urls not handled by MessageViewer::Viewer. */
    void urlClicked( const Akonadi::Item &item, const KUrl &url );
    void mailRemoved();

  private:
    Viewer *m_viewer;
    QSortFilterProxyModel *m_attachmentProxy;
};

}

#endif /* MESSAGEVIEWER_MESSAGEVIEWITEM_H */
