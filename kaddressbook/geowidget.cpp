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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kabc/geo.h>
#include <kaccelmanager.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <qcheckbox.h>
#include <qfile.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qstring.h>

#include "geowidget.h"

GeoWidget::GeoWidget( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name ), mReadOnly( false )
{
  QLabel *label = 0;

  QGridLayout *topLayout = new QGridLayout( this, 4, 3 );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  label = new QLabel( this );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "package_network",
                    KIcon::Desktop, KIcon::SizeMedium ) );
  label->setAlignment( Qt::AlignTop );
  topLayout->addMultiCellWidget( label, 0, 3, 0, 0 );

  mGeoIsValid = new QCheckBox( i18n( "Use geo data" ), this );
  topLayout->addMultiCellWidget( mGeoIsValid, 0, 0, 1, 2 );

  label = new QLabel( i18n( "Latitude:" ), this );
  topLayout->addWidget( label, 1, 1 );

  mLatitudeBox = new KDoubleSpinBox( -90, 90, 1, 0, 6, this );
  mLatitudeBox->setEnabled( false );
  mLatitudeBox->setSuffix( "°" );
  topLayout->addWidget( mLatitudeBox, 1, 2 );
  label->setBuddy( mLatitudeBox );

  label = new QLabel( i18n( "Longitude:" ), this );
  topLayout->addWidget( label, 2, 1 );

  mLongitudeBox = new KDoubleSpinBox( -180, 180, 1, 0, 6, this );
  mLongitudeBox->setEnabled( false );
  mLongitudeBox->setSuffix( "°" );
  topLayout->addWidget( mLongitudeBox, 2, 2 );
  label->setBuddy( mLongitudeBox );

  mExtendedButton = new QPushButton( i18n( "Edit Geo Data..." ), this );
  mExtendedButton->setEnabled( false );
  topLayout->addMultiCellWidget( mExtendedButton, 3, 3, 1, 2 );

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



GeoDialog::GeoDialog( QWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Geo Data Input" ), Ok | Cancel, Ok,
                 parent, name, true, true ),
    mUpdateSexagesimalInput( true )
{
  QFrame *page = plainPage();

  QGridLayout *topLayout = new QGridLayout( page, 2, 2, marginHint(),
                                            spacingHint() );
  topLayout->setRowStretch( 1, 1 );

  mMapWidget = new GeoMapWidget( page );
  topLayout->addMultiCellWidget( mMapWidget, 0, 1, 0, 0 );

  mCityCombo = new KComboBox( page );
  topLayout->addWidget( mCityCombo, 0, 1 );

  QGroupBox *sexagesimalGroup = new QGroupBox( 0, Vertical, i18n( "Sexagesimal" ), page );
  QGridLayout *sexagesimalLayout = new QGridLayout( sexagesimalGroup->layout(),
                                                    2, 5, spacingHint() );

  QLabel *label = new QLabel( i18n( "Latitude:" ), sexagesimalGroup );
  sexagesimalLayout->addWidget( label, 0, 0 );

  mLatDegrees = new QSpinBox( 0, 90, 1, sexagesimalGroup );
  mLatDegrees->setSuffix( "°" );
  mLatDegrees->setWrapping( false );
  label->setBuddy( mLatDegrees );
  sexagesimalLayout->addWidget( mLatDegrees, 0, 1 );

  mLatMinutes = new QSpinBox( 0, 59, 1, sexagesimalGroup );
  mLatMinutes->setSuffix( "'" );
  sexagesimalLayout->addWidget( mLatMinutes, 0, 2 );

  mLatSeconds = new QSpinBox( 0, 59, 1, sexagesimalGroup );
  mLatSeconds->setSuffix( "\"" );
  sexagesimalLayout->addWidget( mLatSeconds, 0, 3 );

  mLatDirection = new KComboBox( sexagesimalGroup );
  mLatDirection->insertItem( i18n( "North" ) );
  mLatDirection->insertItem( i18n( "South" ) );
  sexagesimalLayout->addWidget( mLatDirection, 0, 4 );

  label = new QLabel( i18n( "Longitude:" ), sexagesimalGroup );
  sexagesimalLayout->addWidget( label, 1, 0 );

  mLongDegrees = new QSpinBox( 0, 180, 1, sexagesimalGroup );
  mLongDegrees->setSuffix( "°" );
  label->setBuddy( mLongDegrees );
  sexagesimalLayout->addWidget( mLongDegrees, 1, 1 );

  mLongMinutes = new QSpinBox( 0, 59, 1, sexagesimalGroup );
  mLongMinutes->setSuffix( "'" );
  sexagesimalLayout->addWidget( mLongMinutes, 1, 2 );

  mLongSeconds = new QSpinBox( 0, 59, 1, sexagesimalGroup );
  mLongSeconds->setSuffix( "\"" );
  sexagesimalLayout->addWidget( mLongSeconds, 1, 3 );

  mLongDirection = new KComboBox( sexagesimalGroup );
  mLongDirection->insertItem( i18n( "East" ) );
  mLongDirection->insertItem( i18n( "West" ) );
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

  mLatitude *= ( mLatDirection->currentItem() == 1 ? -1 : 1 );

  mLongitude = (double)( mLongDegrees->value() + (double)mLongMinutes->value() /
                         60 + (double)mLongSeconds->value() / 3600 );

  mLongitude *= ( mLongDirection->currentItem() == 1 ? -1 : 1 );

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
  if ( mCityCombo->currentItem() != 0 ) {
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

    mLatDirection->setCurrentItem( mLatitude < 0 ? 1 : 0 );

    degrees = (int)( longitude * 1 );
    minutes = (int)( ( longitude - degrees ) * 60 );
    seconds = (int)( (double)( longitude - (double)degrees - ( (double)minutes / 60 ) ) * 3600 );

    mLongDegrees->setValue( degrees );
    mLongMinutes->setValue( minutes );
    mLongSeconds->setValue( seconds );
    mLongDirection->setCurrentItem( mLongitude < 0 ? 1 : 0 );
  }
  mUpdateSexagesimalInput = true;

  int pos = nearestCity( mLongitude, mLatitude );
  if ( pos != -1 )
    mCityCombo->setCurrentItem( pos + 1 );
  else
    mCityCombo->setCurrentItem( 0 );

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

  QFile file( locate( "data", "kaddressbook/zone.tab" ) );

  if ( file.open( IO_ReadOnly ) ) {
    QTextStream s( &file );

    QString line, country;
    QRegExp coord( "[+-]\\d+[+-]\\d+" );
    QRegExp name( "[^\\s]+/[^\\s]+" );
    int pos;

    while ( !s.eof() ) {
      line = s.readLine().stripWhiteSpace();
      if ( line.isEmpty() || line[ 0 ] == '#' )
        continue;

      country = line.left( 2 );
      QString c, n;
      pos = coord.search( line, 0 );
      if ( pos >= 0 )
        c = line.mid( pos, coord.matchedLength() );

      pos = name.search(line, pos);
      if ( pos > 0 ) {
        n = line.mid( pos, name.matchedLength() ).stripWhiteSpace();
        n.replace( '_', " " );
      }

      if ( !c.isEmpty() && !n.isEmpty() ) {
        pos = c.find( "+", 1 );
        if ( pos < 0 )
          pos = c.find( "-", 1 );
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
    mCityCombo->insertStringList( items );

    file.close();
  }
}

double GeoDialog::calculateCoordinate( const QString &coordinate )
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

int GeoDialog::nearestCity( double x, double y )
{
  QMap<QString, GeoData>::Iterator it;
  int pos = 0;
  for ( it = mGeoDataMap.begin(); it != mGeoDataMap.end(); ++it, ++pos ) {
    double dist = ( (*it).longitude - x ) * ( (*it).longitude - x ) +
                  ( (*it).latitude - y ) * ( (*it).latitude - y );
    if ( dist < 1.5 )
      return pos;
  }

  return -1;
}


GeoMapWidget::GeoMapWidget( QWidget *parent, const char *name )
  : QWidget( parent, name ), mLatitude( 0 ), mLongitude( 0 )
{
  setBackgroundMode( NoBackground );

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
  p.begin( &pm, this );

  p.setPen( QColor( 255, 0, 0 ) );
  p.setBrush( QColor( 255, 0, 0 ) );

  QPixmap world( locate( "data", "kaddressbook/pics/world.jpg" ) );
  p.drawPixmap( 0, 0, world );

  double latMid = h / 2;
  double longMid = w / 2;

  double latOffset = ( mLatitude * latMid ) / 90;
  double longOffset = ( mLongitude * longMid ) / 180;

  int x = (int)(longMid + longOffset);
  int y = (int)(latMid - latOffset);
  p.drawEllipse( x, y, 4, 4 );

  p.end();
  bitBlt( this, 0, 0, &pm );
}

#include "geowidget.moc"
