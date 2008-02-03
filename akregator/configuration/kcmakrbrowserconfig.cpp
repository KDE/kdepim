/*
    This file is part of Akregator.
    Copyright (c) 2008 Frank Osterfeld <osterfeld@kde.org>

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

#include "kcmakrbrowserconfig.h"
#include "akregatorconfig.h"

#include "ui_settings_browser.h"

#include <KAboutData>
#include <KConfigDialogManager>
#include <KGenericFactory>
#include <KLocalizedString>
#include <kdemacros.h>

#include <QVBoxLayout>

using namespace Akregator;

K_PLUGIN_FACTORY(KCMAkregatorBrowserConfigFactory, registerPlugin<KCMAkregatorBrowserConfig>();)
K_EXPORT_PLUGIN(KCMAkregatorBrowserConfigFactory( "kcmakrbrowserconfig" ))

KCMAkregatorBrowserConfig::KCMAkregatorBrowserConfig( QWidget* parent, const QVariantList& args )
    : KCModule( KCMAkregatorBrowserConfigFactory::componentData(), parent, args ), m_widget( new QWidget )
{  
    Ui::SettingsBrowser ui;
    ui.setupUi( m_widget );

    QVBoxLayout* layout = new QVBoxLayout( this );
    layout->addWidget( m_widget );
    
    connect( ui.kcfg_ExternalBrowserUseCustomCommand, SIGNAL( toggled( bool ) ), 
             ui.kcfg_ExternalBrowserCustomCommand, SLOT( setEnabled( bool ) ) );

    KAboutData *about = new KAboutData( I18N_NOOP( "kcmakrbrowserconfig" ), 0,
                                        ki18n( "Configure Feed Reader Browser" ),
                                        0, KLocalizedString(), KAboutData::License_GPL,
                                        ki18n( "(c), 2004 - 2008 Frank Osterfeld" ) );

    about->addAuthor( ki18n( "Frank Osterfeld" ), KLocalizedString(), "osterfeld@kde.org" );
    setAboutData( about );

    addConfig( Settings::self(), m_widget );
}

#include "kcmakrbrowserconfig.moc"
