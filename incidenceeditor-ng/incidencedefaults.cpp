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

#include <akonadi/kcal/kcalprefs.h>

using namespace KCalCore;

struct IncidenceDefaultsPrivate
{
  /// Members
  QString mGroupWareDomain;

  /// Methods
  Person::Ptr organizerAsPerson() const;
  Attendee::Ptr organizerAsAttendee() const;

  void todoDefaults( const Todo::Ptr &todo ) const;
  void eventDefaults( const Event::Ptr &event ) const;
};

Person::Ptr IncidenceDefaultsPrivate::organizerAsPerson() const
{
  Person::Ptr organizer( new Person );
  organizer->setName( "NOT IMPLEMENTED" );
  organizer->setEmail( "not@implement.com" );
  return organizer;
}

Attendee::Ptr IncidenceDefaultsPrivate::organizerAsAttendee() const
{
  Attendee::Ptr organizer( new Attendee( "NOT IMPLEMENTED", "not@implement.com" ) );
  // NOTE: Don't set the status to None, this value is not supported by the attendee
  //       editor atm.
  organizer->setStatus( Attendee::Accepted );
  organizer->setRole( Attendee::ReqParticipant );
  return organizer;
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

  incidence->setOrganizer( d->organizerAsPerson() );
  incidence->addAttendee( d->organizerAsAttendee() );

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
