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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <kabc/geo.h>
#include <kdialog.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "geowidget.h"

GeoWidget::GeoWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 3 );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  mMapWidget = new GeoMapWidget( this );
  topLayout->addMultiCellWidget( mMapWidget, 0, 1, 0, 0 );

  QLabel *label = new QLabel( i18n( "Latitude:" ), this );
  topLayout->addWidget( label, 0, 1 );

  mLatitudeBox = new KDoubleSpinBox( -90, 90, 1, 0, 6, this );
  topLayout->addWidget( mLatitudeBox, 0, 2 );
  label->setBuddy( mLatitudeBox );

  label = new QLabel( i18n( "Longitude:" ), this );
  topLayout->addWidget( label, 1, 1 );

  mLongitudeBox = new KDoubleSpinBox( -180, 180, 1, 0, 6, this );
  topLayout->addWidget( mLongitudeBox, 1, 2 );
  label->setBuddy( mLongitudeBox );

  connect( mLatitudeBox, SIGNAL( valueChanged( double ) ),
           SIGNAL( changed() ) );
  connect( mLatitudeBox, SIGNAL( valueChanged( double ) ),
           SLOT( updateGeoMap() ) );
  connect( mLongitudeBox, SIGNAL( valueChanged( double ) ),
           SIGNAL( changed() ) );
  connect( mLongitudeBox, SIGNAL( valueChanged( double ) ),
           SLOT( updateGeoMap() ) );
}

GeoWidget::~GeoWidget()
{
}

void GeoWidget::setGeo( const KABC::Geo &geo )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mLatitudeBox->setValue( geo.latitude() );
  mLongitudeBox->setValue( geo.longitude() );

  blockSignals( blocked );

  updateGeoMap();
}

KABC::Geo GeoWidget::geo()
{
  KABC::Geo geo;
  
  geo.setLatitude( mLatitudeBox->value() );
  geo.setLongitude( mLongitudeBox->value() );

  return geo;
}

void GeoWidget::updateGeoMap()
{
  mMapWidget->setLatitude( mLatitudeBox->value() );
  mMapWidget->setLongitude( mLongitudeBox->value() );
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
  update();
}

double GeoMapWidget::latitude()
{
  return mLatitude;
}

void GeoMapWidget::setLongitude( double longitude )
{
  mLongitude = longitude;
  update();
}

double GeoMapWidget::longitude()
{
  return mLongitude;
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
