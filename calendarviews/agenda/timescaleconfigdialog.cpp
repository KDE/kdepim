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
#include <QIcon>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

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

enum {
  TimeZoneNameRole = Qt::UserRole
};

typedef QPair<QString, QString> TimeZoneNamePair;

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

TimeScaleConfigDialog::TimeScaleConfigDialog( const PrefsPtr &preferences, QWidget *parent )
  : QDialog( parent ), d( new Private( this, preferences ) )
{
  setWindowTitle( i18n( "Timezone" ) );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
  okButton->setDefault(true);
  okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &TimeScaleConfigDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &TimeScaleConfigDialog::reject);
  okButton->setDefault(true);
  setModal( true );

  QWidget *mainwidget = new QWidget( this );
  setupUi( mainwidget );
  
  mainLayout->addWidget(mainwidget);
  mainLayout->addWidget(buttonBox);

  QStringList shownTimeZones( d->mPreferences->timeSpec().timeZone().name() );
  shownTimeZones += d->mPreferences->timeScaleTimezones();
  shownTimeZones.removeDuplicates();

  QList<TimeZoneNamePair> availList, selList;
  const KTimeZones::ZoneMap timezones = KSystemTimeZones::zones();
  for ( KTimeZones::ZoneMap::ConstIterator it = timezones.begin();  it != timezones.end();  ++it ) {
    // do not list timezones already shown
    if ( !shownTimeZones.contains( it.key() ) ) {
      availList.append(TimeZoneNamePair(tzWithUTC( it ), it.key()));
    } else {
      selList.append(TimeZoneNamePair(tzWithUTC( it ), it.key()));
    }
  }
  qSort(availList.begin(), availList.end());

  Q_FOREACH(const TimeZoneNamePair& item, availList) {
    zoneCombo->addItem( item.first, item.second );
  }
  zoneCombo->setCurrentIndex( 0 );

  addButton->setIcon( QIcon::fromTheme( QLatin1String("list-add") ) );
  removeButton->setIcon( QIcon::fromTheme( QLatin1String("list-remove") ) );
  upButton->setIcon( QIcon::fromTheme( QLatin1String("go-up") ) );
  downButton->setIcon( QIcon::fromTheme( QLatin1String("go-down") ) );

  connect(addButton, &QPushButton::clicked, this, &TimeScaleConfigDialog::add);
  connect(removeButton, &QPushButton::clicked, this, &TimeScaleConfigDialog::remove);
  connect(upButton, &QPushButton::clicked, this, &TimeScaleConfigDialog::up);
  connect(downButton, &QPushButton::clicked, this, &TimeScaleConfigDialog::down);

  connect(okButton, &QPushButton::clicked, this, &TimeScaleConfigDialog::okClicked);
  connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &TimeScaleConfigDialog::reject);

  Q_FOREACH(const TimeZoneNamePair& item, selList) {
    QListWidgetItem* widgetItem = new QListWidgetItem(item.first);
    widgetItem->setData( TimeZoneNameRole, item.second );
    listWidget->addItem( widgetItem );
  }
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
  if (zoneCombo->currentIndex() >= 0) {
    for ( int i=0; i < listWidget->count(); ++i ) {
      if ( listWidget->item( i )->data(TimeZoneNameRole).toString() == zoneCombo->itemData( zoneCombo->currentIndex(), TimeZoneNameRole ).toString() ) {
        return;
      }
    }

    QListWidgetItem* item = new QListWidgetItem( zoneCombo->currentText() );
    item->setData( TimeZoneNameRole, zoneCombo->itemData( zoneCombo->currentIndex(), TimeZoneNameRole ).toString() );
    listWidget->addItem( item );
    zoneCombo->removeItem( zoneCombo->currentIndex() );
  }

}

void TimeScaleConfigDialog::remove()
{
  zoneCombo->insertItem( 0, listWidget->currentItem()->text(), zoneCombo->itemData( zoneCombo->currentIndex(), TimeZoneNameRole ).toString() );
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
    list << listWidget->item( i )->data(TimeZoneNameRole).toString();
  }
  return list;
}

