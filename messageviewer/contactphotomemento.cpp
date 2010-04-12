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
#include "contactphotomemento.h"

#include <Akonadi/Contact/ContactSearchJob>

using namespace MessageViewer;

ContactPhotoMemento::ContactPhotoMemento( const QString &emailAddress )
  : QObject( 0 ), mFinished( false )
{
  Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob();
  searchJob->setQuery( Akonadi::ContactSearchJob::Email, emailAddress );
  connect( searchJob, SIGNAL( result( KJob* ) ),
           this, SLOT( slotSearchJobFinished( KJob* ) ) );
}

void ContactPhotoMemento::slotSearchJobFinished( KJob *job )
{
  mFinished = true;
  Akonadi::ContactSearchJob *searchJob = static_cast<Akonadi::ContactSearchJob*>( job );
  if ( searchJob->error() ) {
    kWarning() << "Unable to fetch photo for contact:" << searchJob->errorText();
    return;
  }

  if ( searchJob->contacts().size() == 1 ) {

    KABC::Addressee addressee = searchJob->contacts().first();
    mPhoto = addressee.photo();
    emit update( Viewer::Delayed );

  } else if ( searchJob->contacts().size() > 1 ) {
    // TODO: Figure out something here...
  }
}

bool ContactPhotoMemento::finished() const
{
  return mFinished;
}

KABC::Picture ContactPhotoMemento::photo() const
{
  Q_ASSERT( mFinished );
  return mPhoto;
}

void ContactPhotoMemento::detach()
{
  disconnect( this, SIGNAL(update(Viewer::UpdateMode)), 0, 0 );
}



#include "contactphotomemento.moc"
