/* Copyright (C) 2012 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "contactabstractmemento.h"


#include <Akonadi/Contact/ContactSearchJob>

using namespace MessageViewer;

ContactAbstractMemento::ContactAbstractMemento( const QString &emailAddress )
  : QObject( 0 ), mFinished( false )
{
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob();
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, emailAddress );
  connect( searchJob, SIGNAL(result(KJob*)),
           this, SLOT(slotSearchJobFinished(KJob*)) );
}

void ContactAbstractMemento::slotSearchJobFinished( KJob *job )
{
  mFinished = true;
  Akonadi::ContactSearchJob *searchJob = static_cast<Akonadi::ContactSearchJob*>( job );
  if ( searchJob->error() ) {
    kWarning() << "Unable to fetch contact:" << searchJob->errorText();
    return;
  }

  if ( searchJob->contacts().size() == 1 ) {

    KABC::Addressee addressee = searchJob->contacts().first();
    processAddress( addressee );
    emit update( Viewer::Delayed );

  } else if ( searchJob->contacts().size() > 1 ) {
    // TODO: Figure out something here...
  }
}

bool ContactAbstractMemento::finished() const
{
  return mFinished;
}

void ContactAbstractMemento::detach()
{
  disconnect( this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0 );
}

void ContactAbstractMemento::processAddress( const KABC::Addressee& addressee )
{
}
   

#include "contactabstractmemento.moc"
