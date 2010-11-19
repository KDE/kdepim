/*
  Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "mailscheduler2.h"
#include "nepomukcalendar.h"
#include "calendar.h"
#include "calendaradaptor.h"
#include "kcalprefs.h"
#include "identitymanager.h"
#include "mailclient.h"
#include "utils.h"
#include "incidencechanger2.h"

#include <Akonadi/Item>
#include <Akonadi/Collection>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemModifyJob>

#include <KCalCore/ICalFormat>
#include <KCalCore/Incidence>
#include <KCalCore/IncidenceBase>
#include <KCalCore/ScheduleMessage>

#include <KCalUtils/Scheduler>

#include <KStandardDirs>
#include <KSystemTimeZone>

#include <QDir>

using namespace CalendarSupport;

class MailScheduler2::Private {
  public:
    Private()
    {
      mFormat = new KCalCore::ICalFormat();
    }

   ~Private()
    {
      delete mFormat;
    }

    KCalCore::ICalFormat *mFormat;
};


MailScheduler2::MailScheduler2( IncidenceChanger2 *changer,
                                const NepomukCalendar::Ptr &calendar )
  : Scheduler( calendar, changer ), d( new Private )
{
  if ( this->calendar() ) {
    d->mFormat->setTimeSpec( calendar->timeSpec() );
  } else {
    d->mFormat->setTimeSpec( KSystemTimeZones::local() );
  }
}

MailScheduler2::~MailScheduler2()
{
}

CallId MailScheduler2::publish( const KCalCore::IncidenceBase::Ptr &incidence,
                                const QString &recipients )
{
  const QString from = KCalPrefs::instance()->email();
  const bool bccMe = KCalPrefs::instance()->mBcc;
  const QString messageText = d->mFormat->createScheduleMessage( incidence, KCalCore::iTIPPublish );

  MailClient mailer;
  return mailer.mailTo(
    incidence,
    CalendarSupport::identityManager()->identityForAddress( from ),
    from, bccMe, recipients, messageText, KCalPrefs::instance()->mailTransport() );
}

CallId MailScheduler2::performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                           KCalCore::iTIPMethod method,
                                           const QString &recipients )
{
  const QString from = KCalPrefs::instance()->email();
  const bool bccMe = KCalPrefs::instance()->mBcc;
  const QString messageText = d->mFormat->createScheduleMessage( incidence, method );

  MailClient mailer;
  return mailer.mailTo(
    incidence,
    CalendarSupport::identityManager()->identityForAddress( from ),
    from, bccMe, recipients, messageText, KCalPrefs::instance()->mailTransport() );
}

CallId MailScheduler2::performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                           KCalCore::iTIPMethod method )
{
  const QString from = KCalPrefs::instance()->email();
  const bool bccMe = KCalPrefs::instance()->mBcc;
  const QString messageText = d->mFormat->createScheduleMessage( incidence, method );

  MailClient mailer;
  bool status;
  if ( method == KCalCore::iTIPRequest ||
       method == KCalCore::iTIPCancel ||
       method == KCalCore::iTIPAdd ||
       method == KCalCore::iTIPDeclineCounter ) {
    status = mailer.mailAttendees(
      incidence,
      CalendarSupport::identityManager()->identityForAddress( from ),
      bccMe, messageText, KCalPrefs::instance()->mailTransport() );
  } else {
    QString subject;
    KCalCore::Incidence::Ptr inc = incidence.dynamicCast<KCalCore::Incidence>() ;
    if ( inc && method == KCalCore::iTIPCounter ) {
      subject = i18n( "Counter proposal: %1", inc->summary() );
    }
    status = mailer.mailOrganizer(
      incidence,
      CalendarSupport::identityManager()->identityForAddress( from ),
      from, bccMe, messageText, subject, KCalPrefs::instance()->mailTransport() );
  }
  return status;
}

QString MailScheduler2::freeBusyDir() const
{
  return KStandardDirs::locateLocal( "data", QLatin1String( "korganizer/freebusy" ) );
}

//AKONADI_PORT review following code
CallId MailScheduler2::acceptCounterProposal( const KCalCore::Incidence::Ptr &incidence )
{
  if ( !incidence ) {
    return false;
  }

  Akonadi::Item exInc = calendar()->itemForIncidenceUid( incidence->uid() );
  if ( !exInc.isValid() ) {
    KCalCore::Incidence::Ptr exIncidence = calendar()->incidenceFromSchedulingID( incidence->uid() );
    if ( exIncidence ) {
      exInc = calendar()->itemForIncidenceUid( exIncidence->uid() );
    }
    //exInc = exIncItem.isValid() && exIncItem.hasPayload<KCalCore::Incidence::Ptr>() ?
    //        exIncItem.payload<KCalCore::Incidence::Ptr>() : KCalCore::Incidence::Ptr();
  }

  incidence->setRevision( incidence->revision() + 1 );
  if ( exInc.isValid() && exInc.hasPayload<KCalCore::Incidence::Ptr>() ) {
    KCalCore::Incidence::Ptr exIncPtr = exInc.payload<KCalCore::Incidence::Ptr>();
    incidence->setRevision( qMax( incidence->revision(), exIncPtr->revision() + 1 ) );
    // some stuff we don't want to change, just to be safe
    incidence->setSchedulingID( exIncPtr->schedulingID() );
    incidence->setUid( exIncPtr->uid() );

    Q_ASSERT( exIncPtr && incidence );

    KCalCore::IncidenceBase::Ptr i1 = exIncPtr;
    KCalCore::IncidenceBase::Ptr i2 = incidence;

    if ( i1->type() == i2->type() ) {
      *i1 = *i2;
    }

    exIncPtr->updated();
    new Akonadi::ItemModifyJob( exInc );
    //FIXME: Add error handling
  } else {
    int dialogCode = 0;
    const QString incidenceMimeType = CalendarSupport::subMimeTypeForIncidence( incidence );
    QStringList mimeTypes( incidenceMimeType );
    Akonadi::Collection collection = CalendarSupport::selectCollection( 0, dialogCode, mimeTypes );

    Akonadi::Item item;
    item.setPayload( KCalCore::Incidence::Ptr( incidence->clone() ) );
    item.setMimeType( incidenceMimeType );

    new Akonadi::ItemCreateJob( item, collection );
    //FIXME: Add error handling
  }

  return true;
}
