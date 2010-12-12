/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Tobias Koenig <tokoe@kdab.com>

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

#include "markmessagereadhandler.h"

#include "globalsettings.h"

#include <akonadi/itemmodifyjob.h>
#include <akonadi/kmime/messageflags.h>

#include <QtCore/QQueue>
#include <QtCore/QTimer>

using namespace MessageViewer;

class MarkMessageReadHandler::Private
{
  public:
    Private( MarkMessageReadHandler *qq )
      : q( qq )
    {
    }

    void handleMessages();

    MarkMessageReadHandler *q;
    QQueue<Akonadi::Item> mItemQueue;
    QTimer mTimer;
};

void MarkMessageReadHandler::Private::handleMessages()
{
  while ( !mItemQueue.isEmpty() ) {
    Akonadi::Item item = mItemQueue.dequeue();

    // mark as read
    item.setFlag( Akonadi::MessageFlags::Seen );

    Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob( item, q );
    modifyJob->setIgnorePayload( true );
  }
}


MarkMessageReadHandler::MarkMessageReadHandler( QObject *parent )
  : QObject( parent ), d( new Private( this ) )
{
  d->mTimer.setSingleShot( true );
  connect( &d->mTimer, SIGNAL( timeout() ), this, SLOT( handleMessages() ) );
}

MarkMessageReadHandler::~MarkMessageReadHandler()
{
  delete d;
}

void MarkMessageReadHandler::setItem( const Akonadi::Item &item )
{
  if ( item.hasFlag( Akonadi::MessageFlags::Seen ) )
    return;

  d->mTimer.stop();

  d->mItemQueue.enqueue( item );

  if ( MessageViewer::GlobalSettings::self()->delayedMarkAsRead() ) {
    if ( MessageViewer::GlobalSettings::self()->delayedMarkTime() != 0 )
      d->mTimer.start( MessageViewer::GlobalSettings::self()->delayedMarkTime() * 1000 );
    else
      d->handleMessages();
  }
}

#include "markmessagereadhandler.moc"
