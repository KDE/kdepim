/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

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
*/

#include "configwidget.h"

#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

#include <klineedit.h>
#include <klocale.h>

#include "configadvancedoptionwidget.h"
#include "configauthenticationwidget.h"
#include "configlocalizationwidget.h"
#include "configconnectionwidget.h"

ConfigWidget::ConfigWidget( const QSync::PluginConfig &config, QWidget *parent )
  : QWidget( parent ),
    mConfig( config ),
    mAdvancedOption( 0 ),
    mAuthentication( 0 ),
    mLocalization( 0 ),
    mConnection( 0 )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QHBoxLayout *nameLayout = new QHBoxLayout;
  layout->addLayout( nameLayout );

  nameLayout->addWidget( new QLabel( i18n( "Name:" ), this ) );
  mInstanceName = new KLineEdit( this );
  nameLayout->addWidget( mInstanceName );

  if ( !config.advancedOptions().isEmpty() ) {
    mAdvancedOption = new ConfigAdvancedOptionWidget( config.advancedOptions(), this );
    layout->addWidget( mAdvancedOption );
  }

  if ( config.authentication().isValid() ) {
    mAuthentication = new ConfigAuthenticationWidget( config.authentication(), this );
    layout->addWidget( mAuthentication );
  }

  if ( config.localization().isValid() ) {
    mLocalization = new ConfigLocalizationWidget( config.localization(), this );
    layout->addWidget( mLocalization );
  }

  if ( config.connection().isValid() ) {
    mConnection = new ConfigConnectionWidget( config.connection(), this );
    layout->addWidget( mConnection );
  }

  layout->addStretch();
}

void ConfigWidget::setInstanceName( const QString &name )
{
  mInstanceName->setText( name );
}

QString ConfigWidget::instanceName() const
{
  return mInstanceName->text();
}

void ConfigWidget::load()
{
  if ( mAdvancedOption )
    mAdvancedOption->load();

  if ( mAuthentication )
    mAuthentication->load();

  if ( mLocalization )
    mLocalization->load();

  if ( mConnection )
    mConnection->load();
}

void ConfigWidget::save()
{
  if ( mAdvancedOption )
    mAdvancedOption->save();

  if ( mAuthentication )
    mAuthentication->save();

  if ( mLocalization )
    mLocalization->save();

  if ( mConnection )
    mConnection->save();
}
