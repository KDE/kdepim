/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>
    Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "configdialog.h"
#include "settings.h"

#include <gpgme++/context.h>

#include <KConfigDialogManager>
#include <KConfigGroup>
#include <kwindowsystem.h>

ConfigDialog::ConfigDialog( WId windowId, QWidget * parent ) : KDialog( parent )
{
  ui.setupUi( mainWidget() );
  setButtons( Ok | Cancel );

  if ( windowId )
    KWindowSystem::setMainWindow( this, windowId );

  connect( this, SIGNAL(okClicked()), SLOT(save()) );

  ui.encryptedIndex->setEnabled( GpgME::hasFeature( GpgME::G13VFSFeature ) );

  m_manager = new KConfigDialogManager( this, Settings::self() );
  m_manager->updateWidgets();

  KConfig config( "akonadi_nepomuk_feederrc" );
  KConfigGroup cfgGrp( &config, "akonadi_nepomuk_email_feeder" );
  ui.enableIndexing->setChecked( cfgGrp.readEntry( "Enabled", true ) );
}


void ConfigDialog::save()
{
  m_manager->updateSettings();
  if ( Settings::self()->indexEncryptedContent() == Settings::EncryptedIndex && !GpgME::hasFeature( GpgME::G13VFSFeature ) )
    Settings::self()->setIndexEncryptedContent( Settings::NoIndexing );
  Settings::self()->writeConfig();

  KConfig config( "akonadi_nepomuk_feederrc" );
  KConfigGroup cfgGrp( &config, "akonadi_nepomuk_email_feeder" );
  cfgGrp.writeEntry( "Enabled", ui.enableIndexing->isChecked() );
  cfgGrp.sync();
}

#include "configdialog.moc"
