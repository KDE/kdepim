/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  KMail is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  KMail is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "favoritecollectionwidget.h"
#include "messagecore/globalsettings.h"
#include "mailkernel.h"

#include <QDebug>
#include <kxmlguiclient.h>
#include <kglobalsettings.h>

using namespace MailCommon;

FavoriteCollectionWidget::FavoriteCollectionWidget( KXMLGUIClient *xmlGuiClient, QWidget *parent  )
  : Akonadi::EntityListView( xmlGuiClient, parent )
{
  connect( KGlobalSettings::self(), SIGNAL(kdisplayFontChanged()), this,  SLOT(slotGeneralFontChanged()));
  readConfig();
}

FavoriteCollectionWidget::~FavoriteCollectionWidget()
{
}

void FavoriteCollectionWidget::slotGeneralFontChanged()
{
  // Custom/System font support
  if (MessageCore::GlobalSettings::self()->useDefaultFonts() ) {
    setFont( KGlobalSettings::generalFont() );
  }
}

void FavoriteCollectionWidget::readConfig()
{
  // Custom/System font support
  if (!MessageCore::GlobalSettings::self()->useDefaultFonts() ) {
    KConfigGroup fontConfig( KernelIf->config(), "Fonts" );
    setFont( fontConfig.readEntry("folder-font", KGlobalSettings::generalFont() ) );
  } else {
    setFont( KGlobalSettings::generalFont() );
  }
}

#include "favoritecollectionwidget.moc"
