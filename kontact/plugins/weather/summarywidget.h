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

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include "summary.h"

#include <dcopobject.h>

#include <tqmap.h>
#include <tqpixmap.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include <tqtimer.h>
#include <tqwidget.h>

class KProcess;

class QGridLayout;
class QLabel;
class QVBoxLayout;

class WeatherData
{
  public:
    void setIcon( const TQPixmap &icon ) { mIcon = icon; }
    TQPixmap icon() const { return mIcon; }

    void setName( const TQString &name ) { mName = name; }
    TQString name() const { return mName; }

    void setCover( const TQStringList& cover ) { mCover = cover; }
    TQStringList cover() const { return mCover; }

    void setDate( const TQString &date ) { mDate = date; }
    TQString date() const { return mDate; }

    void setTemperature( const TQString &temperature ) { mTemperature = temperature; }
    TQString temperature() const { return mTemperature; }

    void setWindSpeed( const TQString &windSpeed ) { mWindSpeed = windSpeed; }
    TQString windSpeed() const { return mWindSpeed; }

    void setRelativeHumidity( const TQString &relativeHumidity ) { mRelativeHumidity = relativeHumidity; }
    TQString relativeHumidity() const { return mRelativeHumidity; }

    void setStationID( const TQString &station ) { mStationID = station;}
    TQString stationID() { return mStationID; }

    bool operator< ( const WeatherData &data )
    {
      return ( TQString::localeAwareCompare( mName, data.mName ) < 0 );
    }

  private:
    TQPixmap mIcon;
    TQString mName;
    TQStringList mCover;
    TQString mDate;
    TQString mTemperature;
    TQString mWindSpeed;
    TQString mRelativeHumidity;
    TQString mStationID;
};

class SummaryWidget : public Kontact::Summary, public DCOPObject
{
    Q_OBJECT
    K_DCOP
  public:
    SummaryWidget( TQWidget *parent, const char *name = 0 );

    TQStringList configModules() const;

    void updateSummary( bool force = false );

  k_dcop:
    virtual void refresh( TQString );
    virtual void stationRemoved( TQString );

  protected:
    virtual bool eventFilter( TQObject *obj, TQEvent *e );

  private slots:
    void updateView();
    void timeout();
    void showReport( const TQString& );
    void reportFinished( KProcess* );

  private:
    TQStringList mStations;
    TQMap<TQString, WeatherData> mWeatherMap;
    TQTimer mTimer;

    TQPtrList<TQLabel> mLabels;
    TQPtrList<TQGridLayout> mLayouts;
    TQVBoxLayout *mLayout;

    KProcess* mProc;
};

#endif
