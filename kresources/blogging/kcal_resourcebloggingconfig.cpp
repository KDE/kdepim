/*
    This file is part of kdepim.

    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "kcal_resourcebloggingconfig.h"

#include <kmessagebox.h>
#include <klocale.h>

using namespace KCal;

ResourceBloggingConfig::ResourceBloggingConfig( QWidget *parent, const char *name ) : ResourceGroupwareBaseConfig( parent, name )
{
}

void ResourceBloggingConfig::saveSettings( KRES::Resource *resource )
{
  if ( resource && !resource->readOnly() ) {
    KMessageBox::information( this, i18n("Currently, the blogging resource is only read-only. You will not be able to add journals to this resource or upload any changes to the server."), i18n("Read-Only"), "AutoSetReadOnly");
    resource->setReadOnly( true );
  }
  ResourceGroupwareBaseConfig::saveSettings( resource );
}

#include "kcal_resourcebloggingconfig.moc"
