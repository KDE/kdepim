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

#include <QtGui/QFormLayout>

#include <klineedit.h>
#include <klocale.h>

#include "configlocalizationwidget.h"

ConfigLocalizationWidget::ConfigLocalizationWidget( const QSync::PluginLocalization &localization, QWidget *parent )
  : QWidget( parent ),
    mLocalization( localization ),
    mEncoding( 0 ), mTimeZone( 0 ), mLanguage( 0 )
{
  QFormLayout *layout = new QFormLayout( this );

  if ( localization.isOptionSupported( QSync::PluginLocalization::EncodingOption ) ) {
    mEncoding = new KLineEdit( this );
    layout->addRow( i18n( "Encoding:" ), mEncoding );
  }

  if ( localization.isOptionSupported( QSync::PluginLocalization::TimeZoneOption ) ) {
    mTimeZone = new KLineEdit( this );
    layout->addRow( i18n( "Timezone:" ), mTimeZone );
  }

  if ( localization.isOptionSupported( QSync::PluginLocalization::LanguageOption ) ) {
    mLanguage = new KLineEdit( this );
    layout->addRow( i18n( "Language:" ), mLanguage );
  }
}

void ConfigLocalizationWidget::load()
{
  if ( mEncoding )
    mEncoding->setText( mLocalization.encoding() );

  if ( mTimeZone )
    mTimeZone->setText( mLocalization.timeZone() );

  if ( mLanguage )
    mLanguage->setText( mLocalization.language() );
}

void ConfigLocalizationWidget::save()
{
  if ( mEncoding )
    mLocalization.setEncoding( mEncoding->text() );

  if ( mTimeZone )
    mLocalization.setTimeZone( mTimeZone->text() );

  if ( mLanguage )
    mLocalization.setLanguage( mLanguage->text() );
}
