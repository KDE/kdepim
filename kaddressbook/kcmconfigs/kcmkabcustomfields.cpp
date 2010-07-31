/*
    This file is part of KAddressbook.
    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include "kcmkabcustomfields.h"

#include "kabprefs.h"

#include <kstandarddirs.h>

#include <tqregexp.h>

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kabcustomfields( TQWidget *parent, const char * ) {
    return new KCMKabCustomFields( parent, "kcmkabcustomfields" );
  }
}

KCMKabCustomFields::KCMKabCustomFields( TQWidget *parent, const char *name )
  : KCMDesignerFields( parent, name )
{
}

TQString KCMKabCustomFields::localUiDir()
{
  return kabLocalDir() + "contacteditorpages/";
}

TQString KCMKabCustomFields::uiPath()
{
  return "kaddressbook/contacteditorpages/";
}

void KCMKabCustomFields::writeActivePages( const TQStringList &activePages )
{
  KABPrefs::instance()->setAdvancedCustomFields( activePages );
  KABPrefs::instance()->writeConfig();
}

TQStringList KCMKabCustomFields::readActivePages()
{
  return KABPrefs::instance()->advancedCustomFields();
}

TQString KCMKabCustomFields::applicationName()
{
  return "KADDRESSBOOK";
}

TQString KCMKabCustomFields::kabLocalDir()
{
  TQStringList kabdirs = locateLocal("data", "kaddressbook/");
  return kabdirs.grep( TQRegExp( "^"+KGlobal::dirs()->localkdedir() ) ).first();
}

