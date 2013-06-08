/*
  Copyright 2011 Volker Krause <vkrause@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "nepomukwarning.h"

#include <Nepomuk2/ResourceManager>

#include <KAction>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KLocalizedString>
#include <KStandardDirs>
#include <KService>

#include <QProcess>

KPIM::NepomukWarning::NepomukWarning( const char *neverShowAgainKey, QWidget *parent )
  : KMessageWidget( parent ),
    m_neverShowAgainKey( QLatin1String( neverShowAgainKey ) )
{
  const bool neverShowAgain = missingNepomukWarning( neverShowAgainKey );
  if ( !neverShowAgain )
  {
    setMessageType( Warning );
    setCloseButtonVisible( true );
    setWordWrap( true );
    setText( i18n( "You do not have the semantic desktop system enabled. "
                   "Many important features of this software depend on the "
                   "semantic desktop system and will not work correctly without it." ) );

    connect( Nepomuk2::ResourceManager::instance(), SIGNAL(nepomukSystemStarted()),
             SLOT(animatedHide()) );
    connect( Nepomuk2::ResourceManager::instance(), SIGNAL(nepomukSystemStopped()),
             SLOT(animatedShow()) );

    setVisible( !Nepomuk2::ResourceManager::instance()->initialized() );

    KAction *action = this->findChild<KAction *>(); // should give us the close action...
    if ( action ) {
      connect( action, SIGNAL(triggered(bool)), SLOT(explicitlyClosed()) );
    }

    action = new KAction( KIcon( "configure" ), i18n( "&Configure" ), this );
    connect( action, SIGNAL(triggered(bool)), SLOT(configure()) );
    addAction( action );
  }
  else
    setVisible(false);
}

bool KPIM::NepomukWarning::missingNepomukWarning( const char *neverShowAgainKey )
{
  const KConfigGroup cfgGroup( KGlobal::config(), KPIM::NepomukWarning::nepomukWarningGroupName() );
  const bool neverShowAgain = cfgGroup.readEntry( neverShowAgainKey, false );
  return neverShowAgain;
}

QString KPIM::NepomukWarning::nepomukWarningGroupName()
{
  return QLatin1String( "Missing Nepomuk Warning" );
}

void KPIM::NepomukWarning::configure()
{
  if ( KService::serviceByStorageId( "kcm_nepomuk.desktop" ) ) {
    QProcess::startDetached( KStandardDirs::findExe( QLatin1String( "kcmshell4" ) ),
                             QStringList( QLatin1String( "kcm_nepomuk" ) ) );
  } else {
    KAction *action = qobject_cast<KAction *>( sender() );
    action->setEnabled( false );
    setText( i18n( "The module to configure the semantic desktop system (Nepomuk) "
                   "was not found on your system. Please make sure Nepomuk was "
                   "properly installed." ) );
  }

}

void KPIM::NepomukWarning::setMissingFeatures( const QStringList &features )
{
  if ( !features.isEmpty() ) {
    setText( i18n( "You do not have the semantic desktop system enabled. "
                   "The following features will not work correctly:<ul><li>%1</li></ul>",
                   features.join( QLatin1String( "</li><li>" ) ) ) );
  }
}

void KPIM::NepomukWarning::explicitlyClosed()
{
  KConfigGroup cfgGroup( KGlobal::config(), QLatin1String( "Missing Nepomuk Warning" ) );
  cfgGroup.writeEntry( m_neverShowAgainKey, true );
}

#include "nepomukwarning.moc"
