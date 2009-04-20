/*
    This file is part of KContactManager.
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

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

#include "xxportfactory.h"

#include "csv/csv_xxport.h"
#include "ldif/ldif_xxport.h"
#include "vcard/vcard_xxport.h"
#include "opera/opera_xxport.h"
#include "eudora/eudora_xxport.h"
#include "kde2/kde2_xxport.h"

XXPort* XXPortFactory::createXXPort( const QString &identifier, QWidget *parentWidget ) const
{
  if ( identifier == "vcard21" || identifier == "vcard30" )
    return new VCardXXPort( parentWidget );
  else if ( identifier == "csv" )
    return new CsvXXPort( parentWidget );
  else if ( identifier == "ldif" )
    return new LDIFXXPort( parentWidget );
  else if ( identifier == "opera" )
    return new OperaXXPort( parentWidget );
  else if ( identifier == "eudora" )
    return new EudoraXXPort( parentWidget );
  else if ( identifier == "kde2" )
    return new KDE2XXPort( parentWidget );
  else
    return 0;
}
