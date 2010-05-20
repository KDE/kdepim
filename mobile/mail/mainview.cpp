/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
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

#include "mainview.h"
#include "composerview.h"
#include "messagelistproxy.h"
#include "global.h"

#include <KDE/KDebug>
#include <kselectionproxymodel.h>
#include <klocalizedstring.h>

#include <KMime/Message>
#include <akonadi/kmime/messageparts.h>
#include <kpimidentities/identitymanager.h>

#include <KActionCollection>
#include <KAction>

MainView::MainView(QWidget* parent) :
  KDeclarativeMainView( QLatin1String( "kmail-mobile" ), new MessageListProxy, parent )
{
  addMimeType( KMime::Message::mimeType() );
  setListPayloadPart( Akonadi::MessagePart::Header );
  setWindowTitle( i18n( "KMail Mobile" ) );
}

void MainView::startComposer()
{
  ComposerView *composer = new ComposerView;
  composer->show();
}

void MainView::reply( quint64 id )
{
  reply( id, MessageComposer::ReplySmart );
}

void MainView::replyToAll(quint64 id)
{
  reply( id, MessageComposer::ReplyAll );
}

void MainView::reply(quint64 id, MessageComposer::ReplyStrategy replyStrategy)
{
  const Akonadi::Item item = itemFromId( id );
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), id );
  factory.setIdentityManager( Global::identityManager() );
  factory.setReplyStrategy( replyStrategy );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createReply().msg );
  composer->show();
}

void MainView::forwardInline(quint64 id)
{
  const Akonadi::Item item = itemFromId( id );
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), id );
  factory.setIdentityManager( Global::identityManager() );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createForward() );
  composer->show();
}

#include "mainview.moc"

