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

#ifndef GEOWIDGET_H
#define GEOWIDGET_H

#include <kdialogbase.h>

#include "contacteditorwidget.h"

namespace KABC {
class Geo;
}

class GeoMapWidget;

class KComboBox;
class KDoubleSpinBox;

class TQCheckBox;
class TQLabel;
class TQSpinBox;
class TQPushButton;

typedef struct {
  double latitude;
  double longitude;
  TQString country;
} GeoData;

class GeoWidget : public KAB::ContactEditorWidget
{
  Q_OBJECT

  public:
    GeoWidget( KABC::AddressBook *ab, TQWidget *parent, const char *name = 0 );
    ~GeoWidget();

    void loadContact( KABC::Addressee *addr );
    void storeContact( KABC::Addressee *addr );

    void setReadOnly( bool readOnly );

  private slots:
    void editGeoData();

  private:
    KDoubleSpinBox *mLatitudeBox;
    KDoubleSpinBox *mLongitudeBox;

    TQCheckBox *mGeoIsValid;
    TQPushButton *mExtendedButton;

    bool mReadOnly;
};

class GeoDialog : public KDialogBase
{
  Q_OBJECT

  public:
    GeoDialog( TQWidget *parent, const char *name = 0 );
    ~GeoDialog();

    void setLatitude( double latitude );
    double latitude() const;

    void setLongitude( double longitude );
    double longitude() const;

  private slots:
    void updateInputs();

    void sexagesimalInputChanged();
    void geoMapChanged();
    void cityInputChanged();

  private:
    void loadCityList();
    double calculateCoordinate( const TQString& ) const;
    int nearestCity( double, double ) const;

    GeoMapWidget *mMapWidget;
    KComboBox *mCityCombo;

    TQSpinBox *mLatDegrees;
    TQSpinBox *mLatMinutes;
    TQSpinBox *mLatSeconds;
    KComboBox *mLatDirection;

    TQSpinBox *mLongDegrees;
    TQSpinBox *mLongMinutes;
    TQSpinBox *mLongSeconds;
    KComboBox *mLongDirection;

    double mLatitude;
    double mLongitude;
    TQMap<TQString, GeoData> mGeoDataMap;
    bool mUpdateSexagesimalInput;
};

class GeoMapWidget : public QWidget
{
  Q_OBJECT

  public:
    GeoMapWidget( TQWidget *parent, const char *name = 0 );
    ~GeoMapWidget();

    void setLatitude( double latitude );
    double latitude()const;

    void setLongitude( double longitude );
    double longitude()const;

  signals:
    void changed();

  protected:
    virtual void mousePressEvent( TQMouseEvent* );
    virtual void paintEvent( TQPaintEvent* );

  private:
    double mLatitude;
    double mLongitude;
};

class GeoWidgetFactory : public KAB::ContactEditorWidgetFactory
{
  public:
    KAB::ContactEditorWidget *createWidget( KABC::AddressBook *ab, TQWidget *parent, const char *name )
    {
      return new GeoWidget( ab, parent, name );
    }

    TQString pageIdentifier() const { return "misc"; }
};

#endif
