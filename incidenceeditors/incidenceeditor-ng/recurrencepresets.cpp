#include "recurrencepresets.h"

#include <boost/shared_ptr.hpp>

#include <QtCore/QDebug>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include <KCal/Recurrence>
#include <KLocalizedString>

using namespace KCal;

typedef boost::shared_ptr<KCal::Recurrence> Ptr;

namespace IncidenceEditorsNG {
namespace RecurrencePresets {

// Don't use a map, because order matters
static QStringList sPresetNames;
static QList<Ptr>  sPresets = QList<Ptr>();

void initPresets()
{
  Q_ASSERT( sPresetNames.isEmpty() );
  Q_ASSERT( sPresets.isEmpty() );

  // TODO: Read this from a  calendar file to make it more configurable
  Ptr recurrence( new Recurrence() );
  recurrence->setDaily( 1 );
  sPresetNames.append( i18nc( "@item:inlistbox", "Daily" ) );
  sPresets.append( recurrence );

  recurrence = Ptr( new Recurrence() );
  recurrence->setWeekly( 1 /* Every week */, 1 /* On monday */ );
  sPresetNames.append( i18nc( "@item:inlistbox", "Weekly" ) );
  sPresets.append( recurrence );

  QBitArray days( 7 );
  days.setBit( 0 ); // Monday
  days.setBit( 1 );
  days.setBit( 2 );
  days.setBit( 3 );
  days.setBit( 4 ); // Friday
  recurrence = Ptr( new Recurrence() );
  recurrence->setWeekly( 1 /* Every week */, days, 1 /* Weekstart is Monday */ );
  sPresetNames.append( i18nc( "@item:inlistbox", "Every workday" ) );
  sPresets.append( recurrence );

  recurrence = Ptr( new Recurrence() );
  recurrence->setWeekly( 2 /* Bi-weekly */, QBitArray( 7, false ), 1 /* Weekstart is Monday */ );
  sPresetNames.append( i18nc( "@item:inlistbox", "Bi-weekly" ) );
  sPresets.append( recurrence );

  recurrence = Ptr( new Recurrence() );
  recurrence->setMonthly( 1 );
  sPresetNames.append( i18nc( "@item:inlistbox", "Monthly" ) );
  sPresets.append( recurrence );

  recurrence = Ptr( new Recurrence() );
  recurrence->setYearly( 1 );
  sPresetNames.append( i18nc( "@item:inlistbox", "Yearly" ) );
  sPresets.append( recurrence );

  Q_ASSERT( sPresetNames.size() == sPresets.size() );
}

QStringList availablePresets()
{
  if ( sPresetNames.isEmpty() )
    initPresets();

  return sPresetNames;
}

Recurrence *preset( const QString &name, const KDateTime &start )
{
  if ( sPresets.isEmpty() )
    initPresets();

  Q_ASSERT( sPresetNames.count( name ) == 1 ); // The name should exists and only once

  Recurrence *rec = new Recurrence( *sPresets.at( sPresetNames.indexOf( name ) ) );
  rec->setStartDateTime( start );

  // We need to do some trickery for the weekly and bi-weekly cases.
  if ( rec->recurrenceType() == Recurrence::rWeekly ) {
    QBitArray days = rec->days();
    if ( rec->days().count( true ) == 0 ) // The workday case has 5 days set to true
      days.setBit( start.date().dayOfWeek() - 1 );

    rec->setWeekly( rec->frequency(), days );
  } else if ( rec->recurrenceType() == Recurrence::rMonthlyDay ) {
    rec->addMonthlyDate( start.date().day() );
  } else if ( rec->recurrenceType() == Recurrence::rYearlyMonth ) {
    qDebug() << "YEARLY DAY!!!";
    rec->addYearlyDate( start.date().day() );
    rec->addYearlyMonth( start.date().month() );
  }

  return rec;
}


} // RecurrencePresets
} // IncidenceEditorsNG
