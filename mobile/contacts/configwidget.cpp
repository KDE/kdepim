/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Tobias Koenig <tobias.koenig@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "configwidget.h"

#include "settings.h"
#include "stylesheetloader.h"
#include "ui_configwidget.h"

#include <kcmoduleproxy.h>
#include <kconfigdialogmanager.h>
#include <KLocalizedString>

ConfigWidget::ConfigWidget( QWidget *parent )
  : QWidget( parent )
{
  Ui_ConfigWidget ui;
  ui.setupUi( this );

  mLdapConfigWidget = new KCModuleProxy( QLatin1String( "kcmldap" ) );

  ui.ldapServerSettingsLayout->addWidget( mLdapConfigWidget, 1, 1 );

  mMapServiceBox = ui.kcfg_MapService;

  mManager = new KConfigDialogManager( this, Settings::self() );

  mMapServiceBox->addItem( i18n( "None" ), QString() );
  mMapServiceBox->addItem( i18n( "OpenStreetMap" ), QLatin1String( "http://open.mapquestapi.com/nominatim/v1/search.php?q=%s,+%z+%l,+%c" ) );
  mMapServiceBox->addItem( i18n( "Google Maps" ), QLatin1String( "http://maps.google.com/maps?q=%n,%l,%s" ) );
}

void ConfigWidget::load()
{
  KConfig config( QLatin1String("akonadi_contactrc") );
  const KConfigGroup group( &config, "Show Address Settings" );
  const QString addressUrl = group.readEntry( "AddressUrl", QString::fromLatin1( "http://open.mapquestapi.com/nominatim/v1/search.php?q=%s,+%z+%l,+%c" ) );

  Settings::self()->setMapService( mMapServiceBox->findData( addressUrl ) );
  mManager->updateWidgets();

  mLdapConfigWidget->load();
}

void ConfigWidget::save()
{
  mManager->updateSettings();

  const QString addressUrl = mMapServiceBox->itemData( Settings::self()->mapService() ).toString();

  KConfig config( QLatin1String("akonadi_contactrc") );
  KConfigGroup group( &config, "Show Address Settings" );
  group.writeEntry( "AddressUrl", addressUrl );
  config.sync();

  mLdapConfigWidget->save();
}

DeclarativeConfigWidget::DeclarativeConfigWidget( QGraphicsItem *parent )
  : QGraphicsProxyWidget( parent ), mConfigWidget( new ConfigWidget )
{
  QPalette palette = mConfigWidget->palette();
  palette.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );
  mConfigWidget->setPalette( palette );
  StyleSheetLoader::applyStyle( mConfigWidget );

  setWidget( mConfigWidget );
  setFocusPolicy( Qt::StrongFocus );
}

DeclarativeConfigWidget::~DeclarativeConfigWidget()
{
}

void DeclarativeConfigWidget::load()
{
  mConfigWidget->load();
}

void DeclarativeConfigWidget::save()
{
  mConfigWidget->save();
}

