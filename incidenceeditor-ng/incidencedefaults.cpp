/*
  Copyright (C) 2010 Bertjan Broeksema <broeksema@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

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

#include <config-enterprise.h>

#include "incidencedefaults.h"
#include "alarmpresets.h"

#include <calendarsupport/kcalprefs.h>
#include <akonadi/calendar/calendarsettings.h>

#include <KABC/Addressee>

#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <KCalCore/Journal>
#include <KCalCore/Alarm>

#include <KPIMUtils/Email>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <KDebug>
#include <KLocalizedString>

#include <QFile>

using namespace CalendarSupport;
using namespace IncidenceEditorNG;
using namespace KCalCore;

namespace IncidenceEditorNG {

enum {
  UNSPECIFED_PRIORITY = 0
};

class IncidenceDefaultsPrivate
{
public:
  /// Members
  KCalCore::Attachment::List       mAttachments;
  QVector<KCalCore::Attendee::Ptr> mAttendees;
  QStringList                      mEmails;
  QString                          mGroupWareDomain;
  KCalCore::Incidence::Ptr         mRelatedIncidence;
  KDateTime                        mStartDt;
  KDateTime                        mEndDt;
  bool                             mCleanupTemporaryFiles;

  /// Methods
  KCalCore::Person::Ptr organizerAsPerson() const;
  KCalCore::Attendee::Ptr organizerAsAttendee( const KCalCore::Person::Ptr &organizer ) const;

  void todoDefaults( const KCalCore::Todo::Ptr &todo ) const;
  void eventDefaults( const KCalCore::Event::Ptr &event ) const;
  void journalDefaults( const KCalCore::Journal::Ptr &journal ) const;
};

}

KCalCore::Person::Ptr IncidenceDefaultsPrivate::organizerAsPerson() const
{
  const QString invalidEmail = IncidenceDefaults::invalidEmailAddress();

  KCalCore::Person::Ptr organizer( new KCalCore::Person );
  organizer->setName( i18nc( "@label", "no (valid) identities found" ) );
  organizer->setEmail( invalidEmail );

  if ( mEmails.isEmpty() ) {
    // Don't bother any longer, either someone forget to call setFullEmails, or
    // the user has no identities configured.
    return organizer;
  }

  if ( !mGroupWareDomain.isEmpty() ) {
    // Check if we have an identity with an email that ends with the groupware
    // domain.
    foreach ( const QString &fullEmail, mEmails ) {
      QString name;
      QString email;
      const bool success = KPIMUtils::extractEmailAddressAndName( fullEmail, email, name );
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
    foreach ( const QString &fullEmail, mEmails ) {
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

KCalCore::Attendee::Ptr IncidenceDefaultsPrivate::organizerAsAttendee(
  const KCalCore::Person::Ptr &organizer ) const
{
  KCalCore::Attendee::Ptr organizerAsAttendee( new KCalCore::Attendee( "", "" ) );
  // Really, the appropriate values (even the fall back values) should come from
  // organizer. (See organizerAsPerson for more details).
  organizerAsAttendee->setName( organizer->name() );
  organizerAsAttendee->setEmail( organizer->email() );
  // NOTE: Don't set the status to None, this value is not supported by the attendee
  //       editor atm.
  organizerAsAttendee->setStatus( KCalCore::Attendee::Accepted );
  organizerAsAttendee->setRole( KCalCore::Attendee::ReqParticipant );
  return organizerAsAttendee;
}

void IncidenceDefaultsPrivate::eventDefaults( const KCalCore::Event::Ptr &event ) const
{
  KDateTime startDT;
  if ( mStartDt.isValid() ) {
    startDT = mStartDt;
  } else {
    startDT = KDateTime::currentLocalDateTime();

    if ( KCalPrefs::instance()->startTime().isValid() ) {
      startDT.setTime( KCalPrefs::instance()->startTime().time() );
    }
  }

  const QTime defaultDurationTime = KCalPrefs::instance()->defaultDuration().time();
  const int defaultDuration = defaultDurationTime.hour() * 3600 +
                              defaultDurationTime.minute() * 60;

  const KDateTime endDT = mEndDt.isValid() ? mEndDt : startDT.addSecs( defaultDuration );

  event->setDtStart( startDT );
  event->setDtEnd( endDT );
  event->setTransparency( KCalCore::Event::Opaque );

  if ( KCalPrefs::instance()->defaultEventReminders() ) {
    event->addAlarm( AlarmPresets::defaultAlarm( AlarmPresets::BeforeStart ) );
  }
}

void IncidenceDefaultsPrivate::journalDefaults( const KCalCore::Journal::Ptr &journal ) const
{
  const KDateTime startDT = mStartDt.isValid() ? mStartDt : KDateTime::currentLocalDateTime();
  journal->setDtStart( startDT );
  journal->setAllDay( true );
}

void IncidenceDefaultsPrivate::todoDefaults( const KCalCore::Todo::Ptr &todo ) const
{
  KCalCore::Todo::Ptr relatedTodo = mRelatedIncidence.dynamicCast<KCalCore::Todo>();
  if ( relatedTodo ) {
    todo->setCategories( relatedTodo->categories() );
  }

  if ( mEndDt.isValid() ) {
    todo->setDtDue( mEndDt, true/** first */ );
  } else if ( relatedTodo && relatedTodo->hasDueDate() ) {
    todo->setDtDue( relatedTodo->dtDue( true ), true/** first */ );
    todo->setAllDay( relatedTodo->allDay() );
  } else if ( relatedTodo ) {
    todo->setDtDue( KDateTime() );
  } else {
    todo->setDtDue( KDateTime::currentLocalDateTime().addDays( 1 ), true/** first */ );
  }

  if ( mStartDt.isValid() ) {
    todo->setDtStart( mStartDt );
  } else if ( relatedTodo && !relatedTodo->hasStartDate() ) {
    todo->setDtStart( KDateTime() );
  } else if ( relatedTodo && relatedTodo->hasStartDate() &&
              relatedTodo->dtStart() <= todo->dtDue() ) {
    todo->setDtStart( relatedTodo->dtStart() );
    todo->setAllDay( relatedTodo->allDay() );
  } else if ( !mEndDt.isValid() || ( KDateTime::currentLocalDateTime() < mEndDt ) ) {
    todo->setDtStart( KDateTime::currentLocalDateTime() );
  } else {
    todo->setDtStart( mEndDt.addDays( -1 ) );
  }

  todo->setCompleted( false );
  todo->setPercentComplete( 0 );

  // I had a bunch of to-dos and couldn't distinguish between those that had priority '5'
  // because I wanted, and those that had priority '5' because it was set by default
  // and I forgot to unset it.
  // So don't be smart and try to guess a good default priority for the user, just use unspecified.
  todo->setPriority( UNSPECIFED_PRIORITY );

  if ( KCalPrefs::instance()->defaultTodoReminders() ) {
    todo->addAlarm( AlarmPresets::defaultAlarm( AlarmPresets::BeforeEnd ) );
  }
}

/// IncidenceDefaults

IncidenceDefaults::IncidenceDefaults( bool cleanupAttachmentTemporaryFiles )
  : d_ptr( new IncidenceDefaultsPrivate )
{
  d_ptr->mCleanupTemporaryFiles = cleanupAttachmentTemporaryFiles;
}

IncidenceDefaults::IncidenceDefaults( const IncidenceDefaults &other )
  : d_ptr( new IncidenceDefaultsPrivate )
{
  *d_ptr = *other.d_ptr;
}

IncidenceDefaults::~IncidenceDefaults()
{
  delete d_ptr;
}

IncidenceDefaults &IncidenceDefaults::operator=( const IncidenceDefaults &other )
{
  if ( &other != this ) {
    *d_ptr = *other.d_ptr;
  }
  return *this;
}

void IncidenceDefaults::setAttachments( const QStringList &attachments,
                                        const QStringList &attachmentMimetypes,
                                        const QStringList &attachmentLabels,
                                        bool inlineAttachment )
{
  Q_D( IncidenceDefaults );
  d->mAttachments.clear();

  QStringList::ConstIterator it;
  int i = 0;
  for ( it = attachments.constBegin(); it != attachments.constEnd(); ++it, ++i ) {
    if ( !(*it).isEmpty() ) {
      QString mimeType;
      if ( attachmentMimetypes.count() > i ) {
        mimeType = attachmentMimetypes[ i ];
      }

      KCalCore::Attachment::Ptr attachment;
      if ( inlineAttachment ) {
        QString tmpFile;
        if ( KIO::NetAccess::download( *it, tmpFile, 0 ) ) {
          QFile f( tmpFile );
          if ( f.open( QIODevice::ReadOnly ) ) {
            const QByteArray data = f.readAll();
            f.close();

            attachment =
              KCalCore::Attachment::Ptr( new KCalCore::Attachment( data.toBase64(), mimeType ) );

            if ( i < attachmentLabels.count() ) {
              attachment->setLabel( attachmentLabels[ i ] );
            }
          } else {
            kError() << "Error opening " << *it;
          }
        } else {
          kError() << "Error downloading uri " << *it
                   << KIO::NetAccess::lastErrorString(); //krazy:exclude=kdebug
        }
        // TODO, this method needs better error reporting.
        KIO::NetAccess::removeTempFile( tmpFile );

        if ( d_ptr->mCleanupTemporaryFiles ) {
          QFile file( *it );
          file.remove();
        }
      } else {
        attachment = KCalCore::Attachment::Ptr( new KCalCore::Attachment( *it, mimeType ) );
        if ( i < attachmentLabels.count() ) {
          attachment->setLabel( attachmentLabels[ i ] );
        }
      }

      if ( attachment ) {
        if ( attachment->label().isEmpty() ) {
          if ( attachment->isUri() ) {
            attachment->setLabel( attachment->uri() );
          } else {
            attachment->setLabel(
              i18nc( "@label attachment contains binary data", "[Binary data]" ) );
          }
        }
        d->mAttachments << attachment;
        attachment->setShowInline( inlineAttachment );
      }
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
    d->mAttendees << KCalCore::Attendee::Ptr(
      new KCalCore::Attendee( name, email, true, KCalCore::Attendee::NeedsAction ) );
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

void IncidenceDefaults::setRelatedIncidence( const KCalCore::Incidence::Ptr &incidence )
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

void IncidenceDefaults::setDefaults( const KCalCore::Incidence::Ptr &incidence ) const
{
  Q_D( const IncidenceDefaults );

  // First some general defaults
  incidence->setSummary( QString(), false );
  incidence->setLocation( QString(), false );
  incidence->setCategories( QStringList() );
  incidence->setSecrecy( KCalCore::Incidence::SecrecyPublic );
  incidence->setStatus( KCalCore::Incidence::StatusNone );
  incidence->setAllDay( false );
  incidence->setCustomStatus( QString() );
  incidence->setResources( QStringList() );
  incidence->setPriority( 0 );

  if ( d->mRelatedIncidence ) {
    incidence->setRelatedTo( d->mRelatedIncidence->uid() );
  }

  incidence->clearAlarms();
  incidence->clearAttachments();
  incidence->clearAttendees();
  incidence->clearComments();
  incidence->clearContacts();
  incidence->clearRecurrence();
  incidence->clearTempFiles();

  const KCalCore::Person::Ptr organizerAsPerson = d->organizerAsPerson();
#ifdef KDEPIM_ENTERPRISE_BUILD
  incidence->addAttendee( d->organizerAsAttendee( organizerAsPerson ) );
#endif
  foreach ( const KCalCore::Attendee::Ptr &attendee, d->mAttendees ) {
    incidence->addAttendee( attendee );
  }
  // Ical standard: No attendees -> must not have an organizer!
  if ( incidence->attendeeCount() ) {
    incidence->setOrganizer( organizerAsPerson );
  }

  foreach ( const KCalCore::Attachment::Ptr &attachment, d->mAttachments ) {
    incidence->addAttachment( attachment );
  }

  switch ( incidence->type() ) {
  case KCalCore::Incidence::TypeEvent:
    d->eventDefaults( incidence.dynamicCast<KCalCore::Event>() );
    break;
  case KCalCore::Incidence::TypeTodo:
    d->todoDefaults( incidence.dynamicCast<KCalCore::Todo>() );
    break;
  case KCalCore::Incidence::TypeJournal:
    d->journalDefaults( incidence.dynamicCast<KCalCore::Journal>() );
    break;
  default:
    kDebug() << "Unsupported incidence type, keeping current values. Type: "
             << static_cast<int>( incidence->type() );
  }
}

/** static */
IncidenceDefaults IncidenceDefaults::minimalIncidenceDefaults( bool cleanupAttachmentTempFiles )
{
  IncidenceDefaults defaults( cleanupAttachmentTempFiles );

  // Set the full emails manually here, to avoid that we get dependencies on
  // KCalPrefs all over the place.
  defaults.setFullEmails( CalendarSupport::KCalPrefs::instance()->fullEmails() );

  // NOTE: At some point this should be generalized. That is, we now use the
  //       freebusy url as a hack, but this assumes that the user has only one
  //       groupware account. Which doesn't have to be the case necessarily.
  //       This method should somehow depend on the calendar selected to which
  //       the incidence is added.
  if ( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication() ) {
    defaults.setGroupWareDomain(
      KUrl( Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl() ).host() );
  }
  return defaults;
}

/** static */
QString IncidenceDefaults::invalidEmailAddress()
{
  static const QString invalidEmail( i18nc( "@label invalid email address marker",
                                            "invalid@email.address" ) );
  return invalidEmail;
}
