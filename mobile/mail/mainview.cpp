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

#include <messagecore/messagestatus.h>

#include <KActionCollection>
#include <KAction>
#include <KCmdLineArgs>
#include <Akonadi/ItemModifyJob>
#include <Akonadi/ItemFetchScope>
#include "mailactionmanager.h"
#include <Akonadi/ItemFetchJob>

MainView::MainView(QWidget* parent) :
  KDeclarativeMainView( QLatin1String( "kmail-mobile" ), new MessageListProxy, parent )
{
  static const bool debugTiming = KCmdLineArgs::parsedArgs()->isSet("timeit");

  QTime t;
  if ( debugTiming ) {
    t.start();
    kWarning() << "Start MainView ctor" << &t << " - " << QDateTime::currentDateTime();
  }

  addMimeType( KMime::Message::mimeType() );
  itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope );
  setWindowTitle( i18n( "KMail Mobile" ) );

  MailActionManager *mailActionManager = new MailActionManager(actionCollection(), this);
  mailActionManager->setItemSelectionModel(itemSelectionModel());

  connect(actionCollection()->action("mark_message_important"), SIGNAL(triggered(bool)), SLOT(markImportant(bool)));
  connect(actionCollection()->action("mark_message_action_item"), SIGNAL(triggered(bool)), SLOT(markMailTask(bool)));

  connect(itemSelectionModel()->model(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(dataChanged()));

  if ( debugTiming ) {
    kWarning() << "Finished MainView ctor: " << t.elapsed() << " - "<< &t;
  }
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
  Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
  fetch->fetchScope().fetchFullPayload();
  fetch->setProperty( "replyStrategy", QVariant::fromValue( replyStrategy ) );
  connect( fetch, SIGNAL(result(KJob*)), SLOT(replyFetchResult(KJob*)) );
}

void MainView::replyFetchResult(KJob* job)
{
  Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Akonadi::Item item = fetch->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( Global::identityManager() );
  factory.setReplyStrategy( fetch->property( "replyStrategy" ).value<MessageComposer::ReplyStrategy>() );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createReply().msg );
  composer->show();
}

void MainView::forwardInline(quint64 id)
{
  Akonadi::ItemFetchJob *fetch = new Akonadi::ItemFetchJob( Akonadi::Item( id ), this );
  fetch->fetchScope().fetchFullPayload();
  connect( fetch, SIGNAL(result(KJob*)), SLOT(forwardInlineFetchResult(KJob*)) );
}

void MainView::forwardInlineFetchResult( KJob* job )
{
  Akonadi::ItemFetchJob *fetch = qobject_cast<Akonadi::ItemFetchJob*>( job );
  if ( job->error() || fetch->items().isEmpty() )
    return;

  const Akonadi::Item item = fetch->items().first();
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  MessageComposer::MessageFactory factory( item.payload<KMime::Message::Ptr>(), item.id() );
  factory.setIdentityManager( Global::identityManager() );

  ComposerView *composer = new ComposerView;
  composer->setMessage( factory.createForward() );
  composer->show();
}

void MainView::markImportant(bool checked)
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  KPIM::MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if (checked && status.isImportant())
    return;
  if (checked)
      status.setImportant();
  else
      status.setImportant(false);
  item.setFlags(status.getStatusFlags());

  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
  connect(job, SIGNAL(result(KJob *)), SLOT(modifyDone(KJob *)));
}

void MainView::markMailTask(bool checked)
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  KPIM::MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if (checked && status.isToAct())
    return;
  if (checked)
      status.setToAct();
  else
      status.setToAct(false);
  item.setFlags(status.getStatusFlags());

  Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
  connect(job, SIGNAL(result(KJob *)), SLOT(modifyDone(KJob *)));
}

void MainView::modifyDone(KJob *job)
{
  if (job->error())
  {
    kWarning() << "Modify error: " << job->errorString();
    return;
  }
}

void MainView::dataChanged()
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();
  if (list.size() != 1)
    return;
  const QModelIndex idx = list.first();
  Akonadi::Item item = idx.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  if (!item.hasPayload<KMime::Message::Ptr>())
    return;

  KPIM::MessageStatus status;
  status.setStatusFromFlags(item.flags());

  actionCollection()->action("mark_message_important")->setChecked(status.isImportant());
  actionCollection()->action("mark_message_action_item")->setChecked(status.isToAct());
}

// FIXME: remove and put mark-as-read logic into messageviewer (shared with kmail)
void MainView::setListSelectedRow(int row)
{
  static const int column = 0;
  const QModelIndex idx = itemSelectionModel()->model()->index( row, column );
  itemSelectionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
  Akonadi::Item item = idx.data(Akonadi::EntityTreeModel::ItemRole).value<Akonadi::Item>();
  KPIM::MessageStatus status;
  status.setStatusFromFlags(item.flags());
  if ( status.isUnread() )
  {
    status.setRead();
    item.setFlags(status.getStatusFlags());
    Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(item);
  }
}


#include "mainview.moc"
