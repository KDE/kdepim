/*
  Copyright (c) 2007 Bruno Virlet <bruno@virlet.org>

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
#include "timescaleconfigdialog.h"
#include "prefs.h"

#include <KSystemTimeZone>
#include <KLocalizedString>
#include <KIcon>

using namespace EventViews;

class TimeScaleConfigDialog::Private
{
  public:
    Private( TimeScaleConfigDialog *parent, const PrefsPtr &preferences )
      : q( parent ), mPreferences( preferences )
    {
    }

  public:
    TimeScaleConfigDialog *const q;
    PrefsPtr mPreferences;
};

//TODO: move to KCalCore::Stringify
static QString tzUTCOffsetStr( const KTimeZone &tz )
{
  int utcOffsetHrs = tz.currentOffset() / 3600;  // in hours
  int utcOffsetMins = ( tz.currentOffset() % 3600 ) / 60;  // in minutes
  QString utcStr;
  if ( utcOffsetMins > 0 ) {
    utcStr = utcOffsetHrs >= 0 ?
               QString::fromLatin1( "+%1:%2" ).arg( utcOffsetHrs ).arg( utcOffsetMins ) :
               QString::fromLatin1( "%1:%2" ).arg( utcOffsetHrs ).arg( utcOffsetMins );

  } else {
    utcStr = utcOffsetHrs >= 0 ?
               QString::fromLatin1( "+%1" ).arg( utcOffsetHrs ) :
               QString::fromLatin1( "%1" ).arg( utcOffsetHrs );
  }
  return utcStr;
}

//TODO: move to KCalCore::Stringify
static QString tzWithUTC( KTimeZones::ZoneMap::ConstIterator it )
{
  return
    QString::fromLatin1( "%1 (UTC%2)" ).
      arg( i18n( it.key().toUtf8() ) ).
      arg( tzUTCOffsetStr( it.value() ) );
}

//TODO: move to KCalCore::Stringify
static QString tzWithOutUTC( const QString &tz )
{
  return tz.split( QLatin1Char(' ') ).first();
}

TimeScaleConfigDialog::TimeScaleConfigDialog( const PrefsPtr &preferences, QWidget *parent )
  : KDialog( parent ), d( new Private( this, preferences ) )
{
  setCaption( i18n( "Timezone" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  setModal( true );
  showButtonSeparator( false );

  QWidget *mainwidget = new QWidget( this );
  setupUi( mainwidget );
  setMainWidget( mainwidget );

  QStringList shownTimeZones( d->mPreferences->timeSpec().timeZone().name() );
  shownTimeZones += d->mPreferences->timeScaleTimezones();
  shownTimeZones.removeDuplicates();

  QStringList availList, selList;
  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for ( KTimeZones::ZoneMap::ConstIterator it = timezones.begin();  it != timezones.end();  ++it ) {
    // do not list timezones already shown
    if ( !shownTimeZones.contains( it.key() ) ) {
      availList.append( tzWithUTC( it ) );
    } else {
      selList.append( tzWithUTC( it ) );
    }
  }
  availList.sort();
  zoneCombo->addItems( availList );
  zoneCombo->setCurrentIndex( 0 );

  addButton->setIcon( KIcon( QLatin1String("list-add") ) );
  removeButton->setIcon( KIcon( QLatin1String("list-remove") ) );
  upButton->setIcon( KIcon( QLatin1String("go-up") ) );
  downButton->setIcon( KIcon( QLatin1String("go-down") ) );

  connect( addButton, SIGNAL(clicked()), SLOT(add()) );
  connect( removeButton, SIGNAL(clicked()), SLOT(remove()) );
  connect( upButton, SIGNAL(clicked()), SLOT(up()) );
  connect( downButton, SIGNAL(clicked()), SLOT(down()) );

  connect( this, SIGNAL(okClicked()), SLOT(okClicked()) );
  connect( this, SIGNAL(cancelClicked()), SLOT(reject()) );

  listWidget->addItems( selList );
}

TimeScaleConfigDialog::~TimeScaleConfigDialog()
{
  delete d;
}

void TimeScaleConfigDialog::okClicked()
{
  d->mPreferences->setTimeScaleTimezones( zones() );
  d->mPreferences->writeConfig();
  accept();
}

void TimeScaleConfigDialog::add()
{
  // Do not add duplicates
  for ( int i=0; i < listWidget->count(); ++i ) {
    if ( listWidget->item( i )->text() == zoneCombo->currentText() ) {
      return;
    }
  }

  listWidget->addItem( zoneCombo->currentText() );
  zoneCombo->removeItem( zoneCombo->currentIndex() );
}

void TimeScaleConfigDialog::remove()
{
  zoneCombo->insertItem( 0, listWidget->currentItem()->text() );
  delete listWidget->takeItem( listWidget->currentRow() );
}

void TimeScaleConfigDialog::up()
{
  int row = listWidget->currentRow();
  QListWidgetItem *item = listWidget->takeItem( row );
  listWidget->insertItem( qMax( row - 1, 0 ), item );
  listWidget->setCurrentRow( qMax( row - 1, 0 ) );
}

void TimeScaleConfigDialog::down()
{
  int row = listWidget->currentRow();
  QListWidgetItem *item = listWidget->takeItem( row );
  listWidget->insertItem( qMin( row + 1, listWidget->count() ), item );
  listWidget->setCurrentRow( qMin( row + 1, listWidget->count() - 1 ) );
}

QStringList TimeScaleConfigDialog::zones()
{
  QStringList list;
  for ( int i=0; i < listWidget->count(); ++i ) {
    list << tzWithOutUTC( listWidget->item( i )->text() );
  }
  return list;
}

