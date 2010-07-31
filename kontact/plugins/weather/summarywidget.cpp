/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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
#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtooltip.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprocess.h>
#include <kurllabel.h>

#include "summarywidget.h"

SummaryWidget::SummaryWidget( TQWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( "WeatherSummaryWidget" ), mProc( 0 )
{
  mLayout = new TQVBoxLayout( this, 3, 3 );
  mLayout->setAlignment( Qt::AlignTop );

  TQPixmap icon = KGlobal::iconLoader()->loadIcon( "kweather", KIcon::Desktop, KIcon::SizeMedium );
  TQWidget *header = createHeader( this, icon, i18n( "Weather Service" ) );
  mLayout->addWidget( header );

  TQString error;
  TQCString appID;
  bool serviceAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "KWeatherService" ) ) {
    if ( KApplication::startServiceByDesktopName( "kweatherservice", TQStringList(), &error, &appID ) ) {
      TQLabel *label = new TQLabel( i18n( "No weather dcop service available;\nyou need KWeather to use this plugin." ), this );
      mLayout->addWidget( label, Qt::AlignHCenter | AlignVCenter );
      serviceAvailable = false;
    }
  }

  if ( serviceAvailable ) {
    connectDCOPSignal( 0, 0, "fileUpdate(TQString)", "refresh(TQString)", false );
    connectDCOPSignal( 0, 0, "stationRemoved(TQString)", "stationRemoved(TQString)", false );

    DCOPRef dcopCall( "KWeatherService", "WeatherService" );
    DCOPReply reply = dcopCall.call( "listStations()", true );
    if ( reply.isValid() ) {
      mStations = reply;

      connect( &mTimer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( timeout() ) );
      mTimer.start( 0 );
    } else {
      kdDebug(5602) << "ERROR: dcop reply not valid..." << endl;
    }
  }
}


void SummaryWidget::updateView()
{
  mLayouts.setAutoDelete( true );
  mLayouts.clear();
  mLayouts.setAutoDelete( false );

  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  if ( mStations.count() == 0 ) {
    kdDebug(5602) << "No weather stations defined..." << endl;
    return;
  }


  TQValueList<WeatherData> dataList = mWeatherMap.values();
  qHeapSort( dataList );

  TQValueList<WeatherData>::Iterator it;
  for ( it = dataList.begin(); it != dataList.end(); ++it ) {
    TQString cover;
    for ( uint i = 0; i < (*it).cover().count(); ++i )
      cover += TQString( "- %1\n" ).arg( (*it).cover()[ i ] );

    TQImage img;
    img = (*it).icon();

    TQGridLayout *layout = new TQGridLayout( mLayout, 3, 3, 3 );
    mLayouts.append( layout );

    KURLLabel* urlLabel = new KURLLabel( this );
    urlLabel->installEventFilter( this );
    urlLabel->setURL( (*it).stationID() );
    urlLabel->setPixmap( img.smoothScale( 32, 32 ) );
    urlLabel->setMaximumSize( urlLabel->sizeHint() );
    urlLabel->setAlignment( AlignTop );
    layout->addMultiCellWidget( urlLabel, 0, 1, 0, 0 );
    mLabels.append( urlLabel );
    connect ( urlLabel, TQT_SIGNAL( leftClickedURL( const TQString& ) ),
              this, TQT_SLOT( showReport( const TQString& ) ) );

    TQLabel* label = new TQLabel( this );
    label->setText( TQString( "%1 (%2)" ).arg( (*it).name() ).arg( (*it).temperature() ) );
    TQFont font = label->font();
    font.setBold( true );
    label->setFont( font );
    label->setAlignment( AlignLeft );
    layout->addMultiCellWidget( label, 0, 0, 1, 2 );
    mLabels.append( label );

    TQString labelText;
    labelText = TQString( "<b>%1:</b> %2<br>"
                         "<b>%3:</b> %4<br>"
                         "<b>%5:</b> %6" )
                         .arg( i18n( "Last updated on" ) )
                         .arg( (*it).date() )
                         .arg( i18n( "Wind Speed" ) )
                         .arg( (*it).windSpeed() )
                         .arg( i18n( "Rel. Humidity" ) )
                         .arg( (*it).relativeHumidity() );

    TQToolTip::add( label, labelText.replace( " ", "&nbsp;" ) );

    label = new TQLabel( cover, this );
    label->setAlignment( AlignLeft );
    layout->addMultiCellWidget( label, 1, 1, 1, 2 );
    mLabels.append( label );
  }

  for ( TQLabel *label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void SummaryWidget::timeout()
{
  mTimer.stop();

  DCOPRef dcopCall( "KWeatherService", "WeatherService" );
  dcopCall.send( "updateAll()" );

  mTimer.start( 15 * 60000 );
}

void SummaryWidget::refresh( TQString station )
{
  DCOPRef dcopCall( "KWeatherService", "WeatherService" );

  mWeatherMap[ station ].setIcon( dcopCall.call( "currentIcon(TQString)", station, true ) );
  mWeatherMap[ station ].setName( dcopCall.call( "stationName(TQString)", station, true ) );
  mWeatherMap[ station ].setCover( dcopCall.call( "cover(TQString)", station, true ) );
  mWeatherMap[ station ].setDate( dcopCall.call( "date(TQString)", station, true ) );
  mWeatherMap[ station ].setTemperature( dcopCall.call( "temperature(TQString)", station, true ) );
  mWeatherMap[ station ].setWindSpeed( dcopCall.call( "wind(TQString)", station, true ) );
  mWeatherMap[ station ].setRelativeHumidity( dcopCall.call( "relativeHumidity(TQString)", station, true ) );
  mWeatherMap[ station ].setStationID(station);

  updateView();
}

void SummaryWidget::stationRemoved( TQString station )
{
  mWeatherMap.remove( station );
  updateView();
}

bool SummaryWidget::eventFilter( TQObject *obj, TQEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    if ( e->type() == TQEvent::Enter )
      emit message(
        i18n( "View Weather Report for Station" ) );
    if ( e->type() == TQEvent::Leave )
      emit message( TQString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

TQStringList SummaryWidget::configModules() const
{
  return TQStringList( "kcmweatherservice.desktop" );
}

void SummaryWidget::updateSummary( bool )
{
  timeout();
}

void SummaryWidget::showReport( const TQString &stationID )
{
  mProc = new KProcess;
  TQApplication::connect( mProc, TQT_SIGNAL( processExited( KProcess* ) ),
                         this, TQT_SLOT( reportFinished( KProcess* ) ) );
  *mProc << "kweatherreport";
  *mProc << stationID;

  if ( !mProc->start() ) {
    delete mProc;
    mProc = 0;
  }
}

void SummaryWidget::reportFinished( KProcess* )
{
  mProc->deleteLater();
  mProc = 0;
}

#include "summarywidget.moc"
