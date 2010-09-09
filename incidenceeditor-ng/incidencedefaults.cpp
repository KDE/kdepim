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

#include <KABC/Addressee>
#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalCore/Journal>
#include <KLocalizedString>
#include <KPIMUtils/Email>

using namespace KCalCore;
using namespace IncidenceEditorNG;

namespace IncidenceEditorNG {

struct IncidenceDefaultsPrivate
{
  /// Members
  Attachment::List       mAttachments;
  QVector<Attendee::Ptr> mAttendees;
  QStringList            mEmails;
  QString                mGroupWareDomain;
  Incidence::Ptr         mRelatedIncidence;
  KDateTime              mStartDt;
  KDateTime              mEndDt;

  /// Methods
  Person::Ptr organizerAsPerson() const;
  Attendee::Ptr organizerAsAttendee( const Person::Ptr &organizer ) const;

  void todoDefaults( const Todo::Ptr &todo ) const;
  void eventDefaults( const Event::Ptr &event ) const;
  void journalDefaults( const Journal::Ptr &journal ) const;
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

Attendee::Ptr IncidenceDefaultsPrivate::organizerAsAttendee( const Person::Ptr &organizer ) const
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
  KDateTime startDT = KDateTime::currentLocalDateTime();
  if ( mStartDt.isValid() )
    startDT = mStartDt;

  KDateTime endDT = startDT.addSecs( 3600 ); // Default event time: 1 hour
  if ( mEndDt.isValid() )
    endDT = mEndDt;

  event->setDtStart( startDT );
  event->setDtEnd( mEndDt );
  event->setTransparency( Event::Opaque );
}

void IncidenceDefaultsPrivate::journalDefaults( const Journal::Ptr &journal ) const
{
  KDateTime startDT = KDateTime::currentLocalDateTime();
  if ( mStartDt.isValid() )
    startDT = mStartDt;

  KDateTime endDT = startDT.addSecs( 3600 ); // Default event time: 1 hour

  journal->setDtStart( startDT );
}

void IncidenceDefaultsPrivate::todoDefaults( const Todo::Ptr &todo ) const
{
  Todo::Ptr relatedTodo = mRelatedIncidence.dynamicCast<Todo>();
  if ( relatedTodo )
    todo->setCategories( relatedTodo->categories() );

  if ( mEndDt.isValid() )
    todo->setDtDue( mEndDt );
  else if ( relatedTodo && relatedTodo->hasDueDate() )
    todo->setDtDue( relatedTodo->dtDue() );
  else
    todo->setDtDue( KDateTime::currentLocalDateTime().addDays( 1 ) );

  if ( mStartDt.isValid() )
    todo->setDtStart( mStartDt );
  else if ( !mEndDt.isValid() || ( KDateTime::currentLocalDateTime() < mEndDt ) )
    todo->setDtStart( KDateTime::currentLocalDateTime() );
  else
    todo->setDtStart( mEndDt.addDays( -1 ) );

  todo->setCompleted( false );
  todo->setPercentComplete( 0 );
  todo->setPriority( 5 );
}

/// IncidenceDefaults

IncidenceDefaults::IncidenceDefaults()
  : d_ptr( new IncidenceDefaultsPrivate )
{ }

IncidenceDefaults::IncidenceDefaults( const IncidenceDefaults& other )
  : d_ptr( new IncidenceDefaultsPrivate )
{
  *d_ptr = *other.d_ptr;
}

IncidenceDefaults::~IncidenceDefaults()
{
  delete d_ptr;
}

IncidenceDefaults &IncidenceDefaults::operator=( const IncidenceDefaults& other )
{
  if ( &other != this )
    *d_ptr = *other.d_ptr;

  return *this;
}


void IncidenceDefaults::setAttachments( const QStringList &attachments,
                                        const QStringList &attachmentMimetypes,
                                        bool inlineAttachment )
{
  Q_D( IncidenceDefaults );
  d->mAttachments.clear();

  QStringList::ConstIterator it;
  int i = 0;
  for ( it = attachments.constBegin(); it != attachments.constEnd(); ++it, ++i ) {
    if ( !(*it).isEmpty() ) {
      QString mimeType;
      if ( attachmentMimetypes.count() > i )
        mimeType = attachmentMimetypes[ i ];

      Attachment::Ptr attachment( new Attachment( *it, mimeType ) );
      attachment->setShowInline( inlineAttachment );
      d->mAttachments << attachment;
    }
  }
}

void IncidenceDefaults::setAttendees( const QStringList &attendees )
{
  Q_D( IncidenceDefaults );
  d->mAttendees.clear();
  QStringList::ConstIterator it;
  for ( it = attendees.begin(); it != attendees.end(); ++it ) {
    QString name, email;
    KABC::Addressee::parseEmailAddress( *it, name, email );
    d->mAttendees << Attendee::Ptr( new Attendee( name, email, true, Attendee::NeedsAction ) );
  }
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

void IncidenceDefaults::setRelatedIncidence( const Incidence::Ptr &incidence )
{
  Q_D( IncidenceDefaults );
  d->mRelatedIncidence = incidence;
}

void IncidenceDefaults::setStartDateTime( const KDateTime &startDT )
{
  Q_D( IncidenceDefaults );
  d->mStartDt = startDT;
}

void IncidenceDefaults::setEndDateTime( const KDateTime &endDT )
{
  Q_D( IncidenceDefaults );
  d->mEndDt = endDT;
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
  incidence->setAllDay( false );
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
  foreach ( const Attendee::Ptr &attendee, d->mAttendees )
    incidence->addAttendee( attendee );

  foreach ( const Attachment::Ptr &attachment, d->mAttachments )
    incidence->addAttachment( attachment );

  switch ( incidence->type() ) {
  case Incidence::TypeEvent:
    d->eventDefaults( incidence.dynamicCast<Event>() );
    break;
  case Incidence::TypeTodo:
    d->todoDefaults( incidence.dynamicCast<Todo>() );
    break;
  case Incidence::TypeJournal:
    d->journalDefaults( incidence.dynamicCast<Journal>() );
    break;
  default:
    kDebug() << "Unsupported incidence type, keeping current values. Type: "
             << incidence->type();
  }
}
