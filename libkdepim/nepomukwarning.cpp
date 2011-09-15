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

#include <Nepomuk/ResourceManager>

#include <KAction>
#include <KLocalizedString>
#include <KStandardDirs>

#include <QProcess>

KPIM::NepomukWarning::NepomukWarning(QWidget* parent): KMessageWidget(parent)
{
  setMessageType( Warning );
  setCloseButtonVisible( true );
  setWordWrap( true );
  setText( i18n( "You do not have the semantic desktop system enabled. Several features in here depend on this and will thus not work correctly." ) );
  setVisible( !Nepomuk::ResourceManager::instance()->initialized() );
  connect( Nepomuk::ResourceManager::instance(), SIGNAL(nepomukSystemStarted()), SLOT(animatedHide()) );
  connect( Nepomuk::ResourceManager::instance(), SIGNAL(nepomukSystemStopped()), SLOT(animatedShow()) );

  KAction *action = new KAction( KIcon( "configure" ), i18n( "&Configure" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(configure()) );
  addAction( action );
}

void KPIM::NepomukWarning::configure()
{
  QProcess::startDetached( KStandardDirs::findExe(QLatin1String("kcmshell4")), QStringList(QLatin1String("kcm_nepomuk")) );
}
