/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kabc/geo.h>
#include <kacceleratormanager.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <QCheckBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QRegExp>
#include <QString>
#include <QGridLayout>

#include "geowidget.h"

GeoWidget::GeoWidget( KABC::AddressBook *ab, QWidget *parent )
  : KAB::ContactEditorWidget( ab, parent ), mReadOnly( false )
{
  QLabel *label = 0;

  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  label = new QLabel( this );
  label->setPixmap( KIconLoader::global()->loadIcon( "applications-internet",
                    KIconLoader::Desktop, KIconLoader::SizeMedium ) );
  label->setAlignment( Qt::AlignTop );
  topLayout->addWidget( label, 0, 0, 4, 1);

  mGeoIsValid = new QCheckBox( i18n( "Use geo data" ), this );
  topLayout->addWidget( mGeoIsValid, 0, 1, 1, 2 );

  label = new QLabel( i18n( "Latitude:" ), this );
  topLayout->addWidget( label, 1, 1 );

  mLatitudeBox = new QDoubleSpinBox( this );
  mLatitudeBox->setMinimum( -90 );
  mLatitudeBox->setMaximum( 90 );
  mLatitudeBox->setSingleStep( 1 );
  mLatitudeBox->setValue( 0 );
  mLatitudeBox->setDecimals( 6 );
  mLatitudeBox->setEnabled( false );
  mLatitudeBox->setSuffix( QString::fromUtf8( "°" ) );
  topLayout->addWidget( mLatitudeBox, 1, 2 );
  label->setBuddy( mLatitudeBox );

  label = new QLabel( i18n( "Longitude:" ), this );
  topLayout->addWidget( label, 2, 1 );

  mLongitudeBox = new QDoubleSpinBox( this );
  mLongitudeBox->setMinimum( -180 );
  mLongitudeBox->setMaximum( 180 );
  mLongitudeBox->setSingleStep( 1 );
  mLongitudeBox->setValue( 0 );
  mLongitudeBox->setDecimals( 6 );
  mLongitudeBox->setEnabled( false );
  mLongitudeBox->setSuffix( QString::fromUtf8( "°" ) );
  topLayout->addWidget( mLongitudeBox, 2, 2 );
  label->setBuddy( mLongitudeBox );

  mExtendedButton = new QPushButton( i18n( "Edit Geo Data..." ), this );
  mExtendedButton->setEnabled( false );
  topLayout->addWidget( mExtendedButton, 3, 1, 1, 2 );

  connect( mLatitudeBox, SIGNAL( valueChanged( double ) ),
           SLOT( setModified() ) );
  connect( mLongitudeBox, SIGNAL( valueChanged( double ) ),
           SLOT( setModified() ) );
  connect( mExtendedButton, SIGNAL( clicked() ),
           SLOT( editGeoData() ) );

  connect( mGeoIsValid, SIGNAL( toggled( bool ) ),
           mLatitudeBox, SLOT( setEnabled( bool ) ) );
  connect( mGeoIsValid, SIGNAL( toggled( bool ) ),
           mLongitudeBox, SLOT( setEnabled( bool ) ) );
  connect( mGeoIsValid, SIGNAL( toggled( bool ) ),
           mExtendedButton, SLOT( setEnabled( bool ) ) );
  connect( mGeoIsValid, SIGNAL( toggled( bool ) ),
           SLOT( setModified() ) );
}

GeoWidget::~GeoWidget()
{
}

void GeoWidget::loadContact( KABC::Addressee *addr )
{
  KABC::Geo geo = addr->geo();

  if ( geo.isValid() ) {
    if ( !mReadOnly )
      mGeoIsValid->setChecked( true );
    mLatitudeBox->setValue( geo.latitude() );
    mLongitudeBox->setValue( geo.longitude() );
  } else
    mGeoIsValid->setChecked( false );
}

void GeoWidget::storeContact( KABC::Addressee *addr )
{
  KABC::Geo geo;

  if ( mGeoIsValid->isChecked() ) {
    geo.setLatitude( mLatitudeBox->value() );
    geo.setLongitude( mLongitudeBox->value() );
  } else {
    geo.setLatitude( 91 );
    geo.setLongitude( 181 );
  }

  addr->setGeo( geo );
}

void GeoWidget::setReadOnly( bool readOnly )
{
  mReadOnly = readOnly;

  mGeoIsValid->setEnabled( !mReadOnly );
}

void GeoWidget::editGeoData()
{
  GeoDialog dlg( this );

  dlg.setLatitude( mLatitudeBox->value() );
  dlg.setLongitude( mLongitudeBox->value() );

  if ( dlg.exec() ) {
    mLatitudeBox->setValue( dlg.latitude() );
    mLongitudeBox->setValue( dlg.longitude() );

    setModified( true );
  }
}



GeoDialog::GeoDialog( QWidget *parent )
  : KDialog( parent ),
    mUpdateSexagesimalInput( true )
{
  setCaption( i18n( "Geo Data Input" ) );
  setButtons( Ok | Cancel );
  setDefaultButton( Ok );
  showButtonSeparator( true );
  setModal( true );

  QFrame *page = new QFrame(this);
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page );
  topLayout->setSpacing( spacingHint() );
  topLayout->setMargin( 0 );
  topLayout->setRowStretch( 1, 1 );

  mMapWidget = new GeoMapWidget( page );
  topLayout->addWidget( mMapWidget, 0, 0, 2, 1);

  mCityCombo = new KComboBox( page );
  topLayout->addWidget( mCityCombo, 0, 1 );

  QGroupBox *sexagesimalGroup = new QGroupBox( i18n( "Sexagesimal" ), page );
  QGridLayout *sexagesimalLayout = new QGridLayout();
  sexagesimalGroup->setLayout( sexagesimalLayout );
  sexagesimalLayout->setSpacing( spacingHint() );

  QLabel *label = new QLabel( i18n( "Latitude:" ), sexagesimalGroup );
  sexagesimalLayout->addWidget( label, 0, 0 );

  mLatDegrees = new QSpinBox( sexagesimalGroup );
  mLatDegrees->setMinimum( 0 );
  mLatDegrees->setMaximum( 90 );
  mLatDegrees->setValue( 1 );
  mLatDegrees->setSuffix( QString::fromUtf8( "°" ) );
  mLatDegrees->setWrapping( false );
  label->setBuddy( mLatDegrees );
  sexagesimalLayout->addWidget( mLatDegrees, 0, 1 );

  mLatMinutes = new QSpinBox( sexagesimalGroup );
  mLatMinutes->setMinimum( 0 );
  mLatMinutes->setMaximum( 59 );
  mLatMinutes->setValue( 1 );

  mLatMinutes->setSuffix( "'" );
  sexagesimalLayout->addWidget( mLatMinutes, 0, 2 );

  mLatSeconds = new QSpinBox( sexagesimalGroup );
  mLatSeconds->setMinimum( 0 );
  mLatSeconds->setMaximum( 59 );
  mLatSeconds->setValue( 1 );
  mLatSeconds->setSuffix( "\"" );
  sexagesimalLayout->addWidget( mLatSeconds, 0, 3 );

  mLatDirection = new KComboBox( sexagesimalGroup );
  mLatDirection->addItem( i18n( "North" ) );
  mLatDirection->addItem( i18n( "South" ) );
  sexagesimalLayout->addWidget( mLatDirection, 0, 4 );

  label = new QLabel( i18n( "Longitude:" ), sexagesimalGroup );
  sexagesimalLayout->addWidget( label, 1, 0 );

  mLongDegrees = new QSpinBox( sexagesimalGroup );
  mLongDegrees->setMinimum( 0 );
  mLongDegrees->setMaximum( 180 );
  mLongDegrees->setValue( 1 );
  mLongDegrees->setSuffix( QString::fromUtf8( "°" ) );
  label->setBuddy( mLongDegrees );
  sexagesimalLayout->addWidget( mLongDegrees, 1, 1 );

  mLongMinutes = new QSpinBox( sexagesimalGroup );
  mLongMinutes->setMinimum( 0 );
  mLongMinutes->setMaximum( 59 );
  mLongMinutes->setValue( 1 );
  mLongMinutes->setSuffix( "'" );
  sexagesimalLayout->addWidget( mLongMinutes, 1, 2 );

  mLongSeconds = new QSpinBox( sexagesimalGroup );
  mLongSeconds->setMinimum( 0 );
  mLongSeconds->setMaximum( 59 );
  mLongSeconds->setValue( 1 );
  mLongSeconds->setSuffix( "\"" );
  sexagesimalLayout->addWidget( mLongSeconds, 1, 3 );

  mLongDirection = new KComboBox( sexagesimalGroup );
  mLongDirection->addItem( i18n( "East" ) );
  mLongDirection->addItem( i18n( "West" ) );
  sexagesimalLayout->addWidget( mLongDirection, 1, 4 );

  topLayout->addWidget( sexagesimalGroup, 1, 1 );

  loadCityList();

  connect( mMapWidget, SIGNAL( changed() ),
           SLOT( geoMapChanged() ) );
  connect( mCityCombo, SIGNAL( activated( int ) ),
           SLOT( cityInputChanged() ) );
  connect( mLatDegrees, SIGNAL( valueChanged( int ) ),
           SLOT( sexagesimalInputChanged() ) );
  connect( mLatMinutes, SIGNAL( valueChanged( int ) ),
           SLOT( sexagesimalInputChanged() ) );
  connect( mLatSeconds, SIGNAL( valueChanged( int ) ),
           SLOT( sexagesimalInputChanged() ) );
  connect( mLatDirection, SIGNAL( activated( int ) ),
           SLOT( sexagesimalInputChanged() ) );
  connect( mLongDegrees, SIGNAL( valueChanged( int ) ),
           SLOT( sexagesimalInputChanged() ) );
  connect( mLongMinutes, SIGNAL( valueChanged( int ) ),
           SLOT( sexagesimalInputChanged() ) );
  connect( mLongSeconds, SIGNAL( valueChanged( int ) ),
           SLOT( sexagesimalInputChanged() ) );
  connect( mLongDirection, SIGNAL( activated( int ) ),
           SLOT( sexagesimalInputChanged() ) );

  KAcceleratorManager::manage( this );
}

GeoDialog::~GeoDialog()
{
}

void GeoDialog::setLatitude( double latitude )
{
  mLatitude = latitude;
  updateInputs();
}

double GeoDialog::latitude() const
{
  return mLatitude;
}

void GeoDialog::setLongitude( double longitude )
{
  mLongitude = longitude;
  updateInputs();
}

double GeoDialog::longitude() const
{
  return mLongitude;
}

void GeoDialog::sexagesimalInputChanged()
{
  mLatitude = (double)( mLatDegrees->value() + (double)mLatMinutes->value() /
                        60 + (double)mLatSeconds->value() / 3600 );

  mLatitude *= ( mLatDirection->currentIndex() == 1 ? -1 : 1 );

  mLongitude = (double)( mLongDegrees->value() + (double)mLongMinutes->value() /
                         60 + (double)mLongSeconds->value() / 3600 );

  mLongitude *= ( mLongDirection->currentIndex() == 1 ? -1 : 1 );

  mUpdateSexagesimalInput = false;

  updateInputs();
}

void GeoDialog::geoMapChanged()
{
  mLatitude = mMapWidget->latitude();
  mLongitude = mMapWidget->longitude();

  updateInputs();
}

void GeoDialog::cityInputChanged()
{
  if ( mCityCombo->currentIndex() != 0 ) {
    GeoData data = mGeoDataMap[ mCityCombo->currentText() ];
    mLatitude = data.latitude;
    mLongitude = data.longitude;
  } else
    mLatitude = mLongitude = 0;

  updateInputs();
}

void GeoDialog::updateInputs()
{
  // hmm, doesn't look nice, but there is no better way AFAIK
  mCityCombo->blockSignals( true );
  mLatDegrees->blockSignals( true );
  mLatMinutes->blockSignals( true );
  mLatSeconds->blockSignals( true );
  mLatDirection->blockSignals( true );
  mLongDegrees->blockSignals( true );
  mLongMinutes->blockSignals( true );
  mLongSeconds->blockSignals( true );
  mLongDirection->blockSignals( true );

  mMapWidget->setLatitude( mLatitude );
  mMapWidget->setLongitude( mLongitude );
  mMapWidget->update();

  if ( mUpdateSexagesimalInput ) {
    int degrees, minutes, seconds;
    double latitude = mLatitude;
    double longitude = mLongitude;

    latitude *= ( mLatitude < 0 ? -1 : 1 );
    longitude *= ( mLongitude < 0 ? -1 : 1 );

    degrees = (int)( latitude * 1 );
    minutes = (int)( ( latitude - degrees ) * 60 );
    seconds = (int)( (double)( (double)latitude - (double)degrees - ( (double)minutes / (double)60 ) ) * (double)3600 );

    mLatDegrees->setValue( degrees );
    mLatMinutes->setValue( minutes );
    mLatSeconds->setValue( seconds );

    mLatDirection->setCurrentIndex( mLatitude < 0 ? 1 : 0 );

    degrees = (int)( longitude * 1 );
    minutes = (int)( ( longitude - degrees ) * 60 );
    seconds = (int)( (double)( longitude - (double)degrees - ( (double)minutes / 60 ) ) * 3600 );

    mLongDegrees->setValue( degrees );
    mLongMinutes->setValue( minutes );
    mLongSeconds->setValue( seconds );
    mLongDirection->setCurrentIndex( mLongitude < 0 ? 1 : 0 );
  }
  mUpdateSexagesimalInput = true;

  int pos = nearestCity( mLongitude, mLatitude );
  if ( pos != -1 )
    mCityCombo->setCurrentIndex( pos + 1 );
  else
    mCityCombo->setCurrentIndex( 0 );

  mCityCombo->blockSignals( false );
  mLatDegrees->blockSignals( false );
  mLatMinutes->blockSignals( false );
  mLatSeconds->blockSignals( false );
  mLatDirection->blockSignals( false );
  mLongDegrees->blockSignals( false );
  mLongMinutes->blockSignals( false );
  mLongSeconds->blockSignals( false );
  mLongDirection->blockSignals( false );
}

void GeoDialog::loadCityList()
{
  mCityCombo->clear();
  mGeoDataMap.clear();

  QFile file( KStandardDirs::locate( "data", "kaddressbook/zone.tab" ) );

  if ( file.open( QIODevice::ReadOnly ) ) {
    QTextStream s( &file );

    QString line, country;
    QRegExp coord( "[+-]\\d+[+-]\\d+" );
    QRegExp name( "[^\\s]+/[^\\s]+" );
    int pos;

    while ( !s.atEnd() ) {
      line = s.readLine().trimmed();
      if ( line.isEmpty() || line[ 0 ] == '#' )
        continue;

      country = line.left( 2 );
      QString c, n;
      pos = coord.indexIn( line, 0 );
      if ( pos >= 0 )
        c = line.mid( pos, coord.matchedLength() );

      pos = name.indexIn(line, pos);
      if ( pos > 0 ) {
        n = line.mid( pos, name.matchedLength() ).trimmed();
        n.replace( '_', " " );
      }

      if ( !c.isEmpty() && !n.isEmpty() ) {
        pos = c.indexOf( "+", 1 );
        if ( pos < 0 )
          pos = c.indexOf( "-", 1 );
        if ( pos > 0 ) {
          GeoData data;
          data.latitude = calculateCoordinate( c.left( pos ) );
          data.longitude = calculateCoordinate( c.mid( pos ) );
          data.country = country;

          mGeoDataMap.insert( n, data );
        }
      }
    }
    QStringList items( mGeoDataMap.keys() );
    items.prepend( i18n( "Undefined" ) );
    mCityCombo->addItems( items );

    file.close();
  }
}

double GeoDialog::calculateCoordinate( const QString &coordinate ) const
{
  int neg;
  int d = 0, m = 0, s = 0;
  QString str = coordinate;

  neg = str.left( 1 ) == "-";
  str.remove( 0, 1 );

  switch ( str.length() ) {
    case 4:
      d = str.left( 2 ).toInt();
      m = str.mid( 2 ).toInt();
      break;
    case 5:
      d = str.left( 3 ).toInt();
      m = str.mid( 3 ).toInt();
      break;
    case 6:
      d = str.left( 2 ).toInt();
      m = str.mid( 2, 2 ).toInt();
      s = str.right( 2 ).toInt();
      break;
    case 7:
      d = str.left( 3 ).toInt();
      m = str.mid( 3, 2 ).toInt();
      s = str.right( 2 ).toInt();
      break;
    default:
      break;
  }

  if ( neg )
    return - ( d + m / 60.0 + s / 3600.0 );
  else
    return d + m / 60.0 + s / 3600.0;
}

int GeoDialog::nearestCity( double x, double y ) const
{
  QMap<QString, GeoData>::ConstIterator it;
  int pos = 0;
  for ( it = mGeoDataMap.begin(); it != mGeoDataMap.end(); ++it, ++pos ) {
    double dist = ( (*it).longitude - x ) * ( (*it).longitude - x ) +
                  ( (*it).latitude - y ) * ( (*it).latitude - y );
    if ( dist < 1.5 )
      return pos;
  }

  return -1;
}


GeoMapWidget::GeoMapWidget( QWidget *parent )
  : QWidget( parent ), mLatitude( 0 ), mLongitude( 0 )
{
  setAttribute(Qt::WA_NoSystemBackground, true);

  setFixedSize( 400, 200 );

  update();
}

GeoMapWidget::~GeoMapWidget()
{
}

void GeoMapWidget::setLatitude( double latitude )
{
  mLatitude = latitude;
}

double GeoMapWidget::latitude()const
{
  return mLatitude;
}

void GeoMapWidget::setLongitude( double longitude )
{
  mLongitude = longitude;
}

double GeoMapWidget::longitude()const
{
  return mLongitude;
}

void GeoMapWidget::mousePressEvent( QMouseEvent *event )
{
  double latMid = height() / 2;
  double longMid = width() / 2;

  double latOffset = latMid - event->y();
  double longOffset = event->x() - longMid;

  mLatitude = ( latOffset * 90 ) / latMid;
  mLongitude = ( longOffset * 180 ) / longMid;

  emit changed();
}

void GeoMapWidget::paintEvent( QPaintEvent* )
{
  uint w = width();
  uint h = height();

  QPixmap pm( w, h );
  QPainter p;
  p.begin( &pm );
  p.initFrom( this );

  p.setPen( QColor( 255, 0, 0 ) );
  p.setBrush( QColor( 255, 0, 0 ) );

  QPixmap world( KStandardDirs::locate( "data", "kaddressbook/pics/world.jpg" ) );
  p.drawPixmap( 0, 0, world );

  double latMid = h / 2;
  double longMid = w / 2;

  double latOffset = ( mLatitude * latMid ) / 90;
  double longOffset = ( mLongitude * longMid ) / 180;

  int x = (int)(longMid + longOffset);
  int y = (int)(latMid - latOffset);
  p.drawEllipse( x, y, 4, 4 );

  p.end();
  QPainter thisPainter(this);
  thisPainter.drawPixmap( 0, 0, pm );
}

#include "geowidget.moc"
