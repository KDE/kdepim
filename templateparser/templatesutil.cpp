/* -*- mode: C++; c-file-style: "gnu" -*-
  This file is part of KMail, the KDE mail client.
  Copyright (c) 2011 Montel Laurent <montel@kde.org>

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

#include "templatesutil.h"

#include <KConfigGroup>
#include <KSharedConfig>

using namespace TemplateParser;

void TemplateParser::Util::deleteTemplate( const QString &id )
{
  KSharedConfig::Ptr config = KSharedConfig::openConfig( "templatesconfigurationrc", KConfig::NoGlobals );
  const QString key = QString::fromLatin1( "Templates #%1" ).arg( id );
  if ( config->hasGroup( key ) ) {
    KConfigGroup group = config->group( key );
    group.deleteGroup();
    group.sync();
  }
}
