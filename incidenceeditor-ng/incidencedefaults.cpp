/*
    Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
    Copyright (C) 2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

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

#include "incidencedefaults.h"

#include <KDateTime>
#include <KDebug>

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KLocalizedString>
#include <KPIMUtils/Email>

using namespace KCalCore;
using namespace IncidenceEditorsNG;

namespace IncidenceEditorsNG {

struct IncidenceDefaultsPrivate
{
  /// Members
  QStringList mEmails;
  QString     mGroupWareDomain;

  /// Methods
  Person::Ptr organizerAsPerson() const;
  Attendee::Ptr organizerAsAttendee( const Person::ConstPtr &organizer ) const;

  void todoDefaults( const Todo::Ptr &todo ) const;
  void eventDefaults( const Event::Ptr &event ) const;
};

}

Person::Ptr IncidenceDefaultsPrivate::organizerAsPerson() const
{
  const QString invalidEmail = "invalid@email.address";

  Person::Ptr organizer( new Person );
  organizer->setName( i18n( "no (valid) identities found" ) );
  organizer->setEmail( invalidEmail );

  if ( mEmails.isEmpty() ) {
    // Don't bother any longer, either someone forget to call setFullEmails, or
    // the user has no identities configured.
    return organizer;
  }

  if ( !mGroupWareDomain.isEmpty() ) {
    // Check if we have an identity with an email that ends with the groupware
    // domain.
    foreach( const QString &fullEmail, mEmails ) {
      QString name;
      QString email;
      bool success = KPIMUtils::extractEmailAddressAndName( fullEmail, email, name );
      if ( success && email.endsWith( mGroupWareDomain ) ) {
        organizer->setName( name );
        organizer->setEmail( email );
        break;
      }
    }
  }

  if ( organizer->email() == invalidEmail ) {
    // Either, no groupware was used, or we didn't find a groupware email address.
    // Now try to
    foreach( const QString &fullEmail, mEmails ) {
      QString name;
      QString email;
      const bool success = KPIMUtils::extractEmailAddressAndName( fullEmail, email, name );
      if ( success ) {
        organizer->setName( name );
        organizer->setEmail( email );
        break;
      }
    }
  }

  return organizer;
}

Attendee::Ptr IncidenceDefaultsPrivate::organizerAsAttendee( const Person::ConstPtr &organizer ) const
{
  Attendee::Ptr organizerAsAttendee( new Attendee( "", "" ) );
  // Really, the appropriate values (even the fall back values) should come from
  // organizer. (See organizerAsPerson for more details).
  organizerAsAttendee->setName( organizer->name() );
  organizerAsAttendee->setEmail( organizer->email() );
  // NOTE: Don't set the status to None, this value is not supported by the attendee
  //       editor atm.
  organizerAsAttendee->setStatus( Attendee::Accepted );
  organizerAsAttendee->setRole( Attendee::ReqParticipant );
  return organizerAsAttendee;
}

void IncidenceDefaultsPrivate::eventDefaults( const Event::Ptr &event ) const
{
  const KDateTime currentDT = KDateTime::currentLocalDateTime();
  event->setDtStart( currentDT );
  event->setDtEnd( currentDT.addSecs( 3600 ) ); // Default event time: 1 hour
  event->setTransparency( Event::Opaque );
}

void IncidenceDefaultsPrivate::todoDefaults( const Todo::Ptr &todo ) const
{
  Q_UNUSED( todo );
}

/// IncidenceDefaults

IncidenceDefaults::IncidenceDefaults()
  : d_ptr( new IncidenceDefaultsPrivate )
{ }

IncidenceDefaults::~IncidenceDefaults()
{
  delete d_ptr;
}

void IncidenceDefaults::setFullEmails( const QStringList &fullEmails )
{
  Q_D( IncidenceDefaults );
  d->mEmails = fullEmails;
}

void IncidenceDefaults::setGroupWareDomain( const QString &domain )
{
  Q_D( IncidenceDefaults );
  d->mGroupWareDomain = domain;
}

void IncidenceDefaults::setDefaults( const Incidence::Ptr &incidence ) const
{
  Q_D( const IncidenceDefaults );

  // First some general defaults
  incidence->setSummary( QString(), false );
  incidence->setLocation( QString(), false );
  incidence->setCategories( QStringList() );
  incidence->setSecrecy( Incidence::SecrecyPublic );
  incidence->setStatus( Incidence::StatusNone );
  incidence->setCustomStatus( QString() );
  incidence->setResources( QStringList() );
  incidence->setPriority( 0 );

  incidence->clearAlarms();
  incidence->clearAttachments();
  incidence->clearAttendees();
  incidence->clearComments();
  incidence->clearContacts();
  incidence->clearRecurrence();
  incidence->clearTempFiles();

  const Person::Ptr organizerAsPerson = d->organizerAsPerson();
  incidence->setOrganizer( organizerAsPerson );
  incidence->addAttendee( d->organizerAsAttendee( organizerAsPerson ) );

  switch ( incidence->type() ) {
  case Incidence::TypeEvent:
    d->eventDefaults( incidence.dynamicCast<Event>() );
    break;
  case Incidence::TypeTodo:
    d->todoDefaults( incidence.dynamicCast<Todo>() );
    break;
  default:
    kDebug() << "Unsupported incidence type, keeping current values. Type: " << incidence->type();
  }
}
