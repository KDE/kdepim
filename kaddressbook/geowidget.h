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

#ifndef GEOWIDGET_H
#define GEOWIDGET_H

namespace KABC {
class Geo;
}

class GeoMapWidget;
class KDoubleSpinBox;

#include <qwidget.h>

class GeoWidget : public QWidget
{
  Q_OBJECT

  public:
    GeoWidget( QWidget *parent, const char *name = 0 );
    ~GeoWidget();

    /**
      Sets the geo object.
     */
    void setGeo( const KABC::Geo &geo );

    /**
      Returns a geo object.
     */
    KABC::Geo geo();

  signals:
    void changed();

  private slots:
    void updateGeoMap();

  private:
    GeoMapWidget *mMapWidget;
    KDoubleSpinBox *mLatitudeBox;
    KDoubleSpinBox *mLongitudeBox;
};

class GeoMapWidget : public QWidget
{
  public:
    GeoMapWidget( QWidget *parent, const char *name = 0 );
    ~GeoMapWidget();

    void setLatitude( double latitude );
    double latitude();

    void setLongitude( double longitude );
    double longitude();
    
  protected:
    virtual void paintEvent( QPaintEvent* );

  private:
    double mLatitude;
    double mLongitude;
};

#endif
