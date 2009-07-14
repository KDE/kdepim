/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "configurewidget.h"
#include "ui_settings.h"
#include "util.h"
#include "globalsettings.h"

namespace MailViewer {

ConfigureWidget::ConfigureWidget( QWidget *parent )
 : QWidget(parent)
{
  mSettingsUi = new Ui_Settings;
  mSettingsUi->setupUi( this );
  QStringList encodings = Util::supportedEncodings( false );
  mSettingsUi->kcfg_FallbackCharacterEncoding->addItems( encodings );
  QString fallbackCharsetWhatsThis =
    i18n( GlobalSettings::self()->fallbackCharacterEncodingItem()->whatsThis().toUtf8() );
  mSettingsUi->kcfg_FallbackCharacterEncoding->setWhatsThis( fallbackCharsetWhatsThis );

  encodings.prepend( i18n( "Auto" ) );
  mSettingsUi->kcfg_OverrideCharacterEncoding->addItems( encodings );
  mSettingsUi->kcfg_OverrideCharacterEncoding->setCurrentIndex(0);

  QString overrideCharsetWhatsThis =
    i18n( GlobalSettings::self()->overrideCharacterEncodingItem()->whatsThis().toUtf8() );
  mSettingsUi->kcfg_OverrideCharacterEncoding->setWhatsThis( overrideCharsetWhatsThis );

  readCurrentFallbackCodec();
  readCurrentOverrideCodec();
}


ConfigureWidget::~ConfigureWidget()
{
 delete mSettingsUi;
 mSettingsUi = 0;
}

void ConfigureWidget::readCurrentFallbackCodec()
{
  QStringList encodings = Util::supportedEncodings( false );
  QStringList::ConstIterator it( encodings.begin() );
  QStringList::ConstIterator end( encodings.end() );
  QString currentEncoding = GlobalSettings::self()->fallbackCharacterEncoding();
  uint i = 0;
  int indexOfLatin9 = 0;
  bool found = false;
  for( ; it != end; ++it)
  {
    const QString encoding = Util::encodingForName(*it);
    if ( encoding == "ISO-8859-15" )
        indexOfLatin9 = i;
    if( encoding == currentEncoding )
    {
      mSettingsUi->kcfg_FallbackCharacterEncoding->setCurrentIndex( i );
      found = true;
      break;
    }
    i++;
  }
  if ( !found ) // nothing matched, use latin9
    mSettingsUi->kcfg_FallbackCharacterEncoding->setCurrentIndex( indexOfLatin9 );
}

void ConfigureWidget::readCurrentOverrideCodec()
{
  const QString &currentOverrideEncoding = GlobalSettings::self()->overrideCharacterEncoding();
  if ( currentOverrideEncoding.isEmpty() ) {
    mSettingsUi->kcfg_OverrideCharacterEncoding->setCurrentIndex( 0 );
    return;
  }
  QStringList encodings = Util::supportedEncodings( false );
  encodings.prepend( i18n( "Auto" ) );
  QStringList::ConstIterator it( encodings.constBegin() );
  QStringList::ConstIterator end( encodings.constEnd() );
  int i = 0;
  for( ; it != end; ++it)
  {
    if( Util::encodingForName(*it) == currentOverrideEncoding )
    {
      mSettingsUi->kcfg_OverrideCharacterEncoding->setCurrentIndex( i );
      break;
    }
    i++;
  }
  if ( i == encodings.size() ) {
    // the current value of overrideCharacterEncoding is an unknown encoding => reset to Auto
    kWarning() <<"Unknown override character encoding \"" << currentOverrideEncoding
                   << "\". Resetting to Auto.";
    mSettingsUi->kcfg_OverrideCharacterEncoding->setCurrentIndex( 0 );
    GlobalSettings::self()->setOverrideCharacterEncoding( QString() );
  }
}


void ConfigureWidget::slotSettingsChanged()
{
  GlobalSettings::self()->setOverrideCharacterEncoding( Util::encodingForName( mSettingsUi->kcfg_OverrideCharacterEncoding->currentText() ) );
  GlobalSettings::self()->setFallbackCharacterEncoding( Util::encodingForName( mSettingsUi->kcfg_FallbackCharacterEncoding->currentText() ) );
  emit settingsChanged();
}


}

#include "configurewidget.moc"
