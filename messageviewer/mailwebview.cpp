/* Copyright 2010 Thomas McGuire <mcguire@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "mailwebview.h"

#include <KDebug>

#include <QContextMenuEvent>
#include <QWebFrame>

using namespace MessageViewer;

MailWebView::MailWebView( QWidget *parent )
  : KWebView( parent )
{
}

bool MailWebView::event( QEvent *event )
{
  if ( event->type() != QEvent::ContextMenu )
    return KWebView::event( event );
  else {
    // Don't call KWebView::event() here, it will do silly things like selecting the text
    // under the mouse cursor, which we don't want.

    QContextMenuEvent const *contextMenuEvent = static_cast<QContextMenuEvent*>( event );
    const QWebFrame * const frame = page()->currentFrame();
    const QWebHitTestResult hit = frame->hitTestContent( contextMenuEvent->pos() );
    kDebug() << "Right-clicked URL:" << hit.linkUrl();
    emit popupMenu( hit.linkUrl().toString(), mapToGlobal( contextMenuEvent->pos() ) );
    event->accept();
    return true;
  }
}
