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

#include <QDeclarativeItem>

namespace MessageViewer {
class Viewer;

class MessageViewItem : public QDeclarativeItem
{
  Q_OBJECT
  Q_PROPERTY( int messageItemId READ messageItemId WRITE setMessageItemId )
  Q_PROPERTY( QString splashMessage READ splashMessage WRITE setSplashMessage )

  public:
    explicit MessageViewItem( QDeclarativeItem *parent = 0 );
    ~MessageViewItem();

    qint64 messageItemId() const;
    void setMessageItemId( qint64 id );

    QString splashMessage() const;
    void setSplashMessage( const QString &message );

  protected:
    void geometryChanged(const QRectF& newGeometry, const QRectF& oldGeometry);
    bool eventFilter( QObject *obj, QEvent *event );

  private:
    Viewer *m_viewer;
    QGraphicsProxyWidget *m_proxy;

    /// Handle mouse events
    bool m_mousePressed;
};

}

#endif
