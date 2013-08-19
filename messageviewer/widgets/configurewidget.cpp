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
#include "utils/util.h"
#include "settings/globalsettings.h"
#include "viewer/nodehelper.h"

#include "header/customheadersettingwidget.h"

#include "messagecore/settings/globalsettings.h"

#include <KConfigDialogManager>
#include <KDialog>

using namespace MessageViewer;

ConfigureWidget::ConfigureWidget( QWidget *parent )
    : QWidget( parent )
{
    mSettingsUi = new Ui_Settings;
    mSettingsUi->setupUi( this );
    layout()->setContentsMargins( 0, 0, 0, 0 );

    QStringList encodings = NodeHelper::supportedEncodings( false );
    mSettingsUi->fallbackCharacterEncoding->addItems( encodings );
    encodings.prepend( i18n( "Auto" ) );
    mSettingsUi->overrideCharacterEncoding->addItems( encodings );
    mSettingsUi->overrideCharacterEncoding->setCurrentIndex( 0 );

    mSettingsUi->fallbackCharacterEncoding->setWhatsThis(
                MessageCore::GlobalSettings::self()->fallbackCharacterEncodingItem()->whatsThis() );
    mSettingsUi->overrideCharacterEncoding->setWhatsThis(
                MessageCore::GlobalSettings::self()->overrideCharacterEncodingItem()->whatsThis() );
    mSettingsUi->kcfg_ShowEmoticons->setWhatsThis(
                GlobalSettings::self()->showEmoticonsItem()->whatsThis() );
    mSettingsUi->kcfg_ShrinkQuotes->setWhatsThis(
                GlobalSettings::self()->shrinkQuotesItem()->whatsThis() );
    mSettingsUi->kcfg_ShowExpandQuotesMark->setWhatsThis(
                GlobalSettings::self()->showExpandQuotesMarkItem()->whatsThis() );

    connect( mSettingsUi->fallbackCharacterEncoding, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(settingsChanged()) );
    connect( mSettingsUi->overrideCharacterEncoding, SIGNAL(currentIndexChanged(int)),
             this, SIGNAL(settingsChanged()) );

    connect( mSettingsUi->configureCustomHeadersButton, SIGNAL(clicked()),
             this, SLOT(showCustomHeadersDialog()) );
}

ConfigureWidget::~ConfigureWidget()
{
    delete mSettingsUi;
    mSettingsUi = 0;
}

void ConfigureWidget::readConfig()
{
    readCurrentFallbackCodec();
    readCurrentOverrideCodec();
    mSettingsUi->kcfg_CollapseQuoteLevelSpin->setEnabled(
                GlobalSettings::self()->showExpandQuotesMark() );
}

void ConfigureWidget::writeConfig()
{
    MessageCore::GlobalSettings::self()->setFallbackCharacterEncoding(
                NodeHelper::encodingForName( mSettingsUi->fallbackCharacterEncoding->currentText() ) );
    MessageCore::GlobalSettings::self()->setOverrideCharacterEncoding(
                mSettingsUi->overrideCharacterEncoding->currentIndex() == 0 ?
                    QString() :
                    NodeHelper::encodingForName( mSettingsUi->overrideCharacterEncoding->currentText() ) );

    KMime::setFallbackCharEncoding( NodeHelper::encodingForName( mSettingsUi->fallbackCharacterEncoding->currentText() ) );

}

void ConfigureWidget::readCurrentFallbackCodec()
{
    const QStringList encodings = NodeHelper::supportedEncodings( false );
    QStringList::ConstIterator it( encodings.constBegin() );
    const QStringList::ConstIterator end( encodings.constEnd() );
    const QString currentEncoding = MessageCore::GlobalSettings::self()->fallbackCharacterEncoding();
    uint i = 0;
    int indexOfLatin9 = 0;
    bool found = false;
    for( ; it != end; ++it ) {
        const QString encoding = NodeHelper::encodingForName( *it );
        if ( encoding == QLatin1String("ISO-8859-15") )
            indexOfLatin9 = i;
        if( encoding == currentEncoding ) {
            mSettingsUi->fallbackCharacterEncoding->setCurrentIndex( i );
            found = true;
            break;
        }
        ++i;
    }
    if ( !found ) // nothing matched, use latin9
        mSettingsUi->fallbackCharacterEncoding->setCurrentIndex( indexOfLatin9 );
}

void ConfigureWidget::readCurrentOverrideCodec()
{
    const QString &currentOverrideEncoding = MessageCore::GlobalSettings::self()->overrideCharacterEncoding();
    if ( currentOverrideEncoding.isEmpty() ) {
        mSettingsUi->overrideCharacterEncoding->setCurrentIndex( 0 );
        return;
    }
    QStringList encodings = NodeHelper::supportedEncodings( false );
    encodings.prepend( i18n( "Auto" ) );
    QStringList::ConstIterator it( encodings.constBegin() );
    const QStringList::ConstIterator end( encodings.constEnd() );
    int i = 0;
    for( ; it != end; ++it ) {
        if( NodeHelper::encodingForName(*it) == currentOverrideEncoding ) {
            mSettingsUi->overrideCharacterEncoding->setCurrentIndex( i );
            break;
        }
        ++i;
    }
    if ( i == encodings.size() ) {
        // the current value of overrideCharacterEncoding is an unknown encoding => reset to Auto
        kWarning() << "Unknown override character encoding" << currentOverrideEncoding
                   << ". Resetting to Auto.";
        mSettingsUi->overrideCharacterEncoding->setCurrentIndex( 0 );
        MessageCore::GlobalSettings::self()->setOverrideCharacterEncoding( QString() );
    }
}

void ConfigureWidget::showCustomHeadersDialog()
{
    KDialog dialog( this );
    dialog.setButtons( KDialog::Default | KDialog::Ok | KDialog::Cancel );
    dialog.resize(500,250);
    CustomHeaderSettingWidget *widget = new CustomHeaderSettingWidget();
    connect( &dialog, SIGNAL(defaultClicked()), widget, SLOT(resetToDefault()) );
    widget->readConfig();
    dialog.setMainWidget( widget );
    if ( dialog.exec() == QDialog::Accepted ) {
        widget->writeConfig();
        settingsChanged();
    }
}

#include "configurewidget.moc"
