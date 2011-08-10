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

#include "mailscheduler.h"
#include "nepomukcalendar.h"
#include "calendar.h"
#include "calendaradaptor.h"
#include "kcalprefs.h"
#include "identitymanager.h"
#include "mailclient.h"
#include "utils.h"

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

MailScheduler::MailScheduler( CalendarSupport::Calendar *calendar )
  //: Scheduler( calendar )
  : mCalendar( calendar ), mFormat( new KCalCore::ICalFormat() )
{
  if ( mCalendar ) {
    mFormat->setTimeSpec( calendar->timeSpec() );
  } else {
    mFormat->setTimeSpec( KSystemTimeZones::local() );
  }
}

MailScheduler::MailScheduler( const NepomukCalendar::Ptr &nepomukCalendar )
  : mCalendar( 0 ), mNepomukCalendar( nepomukCalendar ), mFormat( new KCalCore::ICalFormat() )
{
  if ( nepomukCalendar ) {
    mFormat->setTimeSpec( nepomukCalendar->timeSpec() );
  } else {
    mFormat->setTimeSpec( KSystemTimeZones::local() );
  }

}

MailScheduler::~MailScheduler()
{
  delete mFormat;
}

bool MailScheduler::publish( const KCalCore::IncidenceBase::Ptr &incidence,
                             const QString &recipients )
{
  QString from = KCalPrefs::instance()->email();
  bool bccMe = KCalPrefs::instance()->mBcc;
  QString messageText = mFormat->createScheduleMessage( incidence, KCalCore::iTIPPublish );

  MailClient mailer;
  return mailer.mailTo(
    incidence,
    CalendarSupport::identityManager()->identityForAddress( from ),
    from, bccMe, recipients, messageText, KCalPrefs::instance()->mailTransport() );
}

bool MailScheduler::performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                        KCalCore::iTIPMethod method,
                                        const QString &recipients )
{
  QString from = KCalPrefs::instance()->email();
  bool bccMe = KCalPrefs::instance()->mBcc;
  QString messageText = mFormat->createScheduleMessage( incidence, method );

  MailClient mailer;
  return mailer.mailTo(
    incidence,
    CalendarSupport::identityManager()->identityForAddress( from ),
    from, bccMe, recipients, messageText, KCalPrefs::instance()->mailTransport() );
}

bool MailScheduler::performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                        KCalCore::iTIPMethod method )
{
  QString from = KCalPrefs::instance()->email();
  bool bccMe = KCalPrefs::instance()->mBcc;
  QString messageText = mFormat->createScheduleMessage( incidence, method );

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
#if 0
/*
QList<ScheduleMessage*> MailScheduler::retrieveTransactions()
{
  QString incomingDirName = KStandardDirs::locateLocal( "data",
                                                        QLatin1String( "korganizer/income" ) );
  kDebug() << "dir:" << incomingDirName;

  QList<ScheduleMessage*> messageList;

  QDir incomingDir( incomingDirName );
  QStringList incoming = incomingDir.entryList( QDir::Files );
  QStringList::ConstIterator it;
  for ( it = incoming.constBegin(); it != incoming.constEnd(); ++it ) {
    kDebug() << "-- File:" << (*it);

    QFile f( incomingDirName + QLatin1Char( '/' ) + (*it) );
    bool inserted = false;
    QMap<KCalCore::IncidenceBase *, QString>::Iterator iter;
    for ( iter = mEventMap.begin(); iter != mEventMap.end(); ++iter ) {
      if ( iter.value() == incomingDirName + QLatin1Char( '/' ) + (*it) ) {
        inserted = true;
      }
    }
    if ( !inserted ) {
      if ( !f.open( QIODevice::ReadOnly ) ) {
        kDebug() << "Can't open file'" << (*it) << "'";
      } else {
        QTextStream t( &f );
        t.setCodec( "ISO 8859-1" );
        QString messageString = t.readAll();
        messageString.remove( QRegExp( QLatin1String( "\n[ \t]" ) ) );
        messageString = QString::fromUtf8( messageString.toLatin1() );

        KCalCore::Calendar::Ptr calendar = calendarr();
        ScheduleMessage *mess = mFormat->parseScheduleMessage( calendar, messageString );

        if ( mess ) {
          kDebug() << "got message '" << (*it) << "'";
          messageList.append( mess );
          mEventMap[ mess->event() ] = incomingDirName + QLatin1Char( '/' ) + (*it);
        } else {
          QString errorMessage;
          if ( mFormat->exception() ) {
            errorMessage = mFormat->exception()->message();
          }
          kDebug() << "Error parsing message:" << errorMessage;
        }
        f.close();
      }
    }
  }
  return messageList;
}

bool MailScheduler::deleteTransaction( const KCalCore::IncidenceBase::Ptr &incidence )
{
  bool status;
  QFile f( mEventMap[incidence] );
  mEventMap.remove( incidence );
  if ( !f.exists() ) {
    status = false;
  } else {
    status = f.remove();
  }
  return status;
}
*/
#endif

QString MailScheduler::freeBusyDir() const
{
  return KStandardDirs::locateLocal( "data", QLatin1String( "korganizer/freebusy" ) );
}

bool MailScheduler::acceptTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                       KCalCore::iTIPMethod method,
                                       KCalCore::ScheduleMessage::Status status,
                                       const QString &email )
{
  class SchedulerAdaptor : public KCalUtils::Scheduler
  {
    public:
      SchedulerAdaptor( MailScheduler *scheduler,
                        const KCalCore::Calendar::Ptr &calendar )
        : KCalUtils::Scheduler( calendar ), m_scheduler( scheduler )
      {
      }

      virtual ~SchedulerAdaptor()
      {
      }

      virtual bool publish( const KCalCore::IncidenceBase::Ptr &incidence,
                            const QString &recipients )
      {
        return m_scheduler->publish( incidence, recipients );
      }

      virtual bool performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                       KCalCore::iTIPMethod method )
      {
        return m_scheduler->performTransaction( incidence, method );
      }

      virtual bool performTransaction( const KCalCore::IncidenceBase::Ptr &incidence,
                                       KCalCore::iTIPMethod method,
                                       const QString &recipients )
      {
        return m_scheduler->performTransaction( incidence, method, recipients );
      }

      virtual bool acceptCounterProposal( const KCalCore::Incidence::Ptr &incidence )
      {
        return m_scheduler->acceptCounterProposal( incidence );
      }

      virtual QList<KCalCore::ScheduleMessage*> retrieveTransactions() {
#if 0
        return m_scheduler->retrieveTransactions();
#else
        return QList<KCalCore::ScheduleMessage*>();
#endif
      }
      virtual QString freeBusyDir() {
        return m_scheduler->freeBusyDir();
      }
    private:
      MailScheduler *m_scheduler;
  };

  SchedulerAdaptor scheduleradaptor( this, calendar() );
  return scheduleradaptor.acceptTransaction( incidence, method, status, email );
}

//AKONADI_PORT review following code
bool MailScheduler::acceptCounterProposal( const KCalCore::Incidence::Ptr &incidence )
{
  if ( !incidence ) {
    return false;
  }

  Akonadi::Item exInc = (mCalendar ? mCalendar->itemForIncidenceUid( incidence->uid() )
                                   : mNepomukCalendar->itemForIncidenceUid( incidence->uid() ));
  if ( ! exInc.isValid() && mCalendar ) {
    exInc = mCalendar->incidenceFromSchedulingID( incidence->uid() );
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

KCalCore::Calendar::Ptr MailScheduler::calendar() const
{
  if ( mCalendar ) {
    return KCalCore::Calendar::Ptr( new CalendarAdaptor( mCalendar, 0 ) );
  } else if ( mNepomukCalendar ) {
    return mNepomukCalendar;
  } else {
    return KCalCore::Calendar::Ptr();
  }
}
