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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kcmkabcustomfields.h"

#include "kabprefs.h"

#include <kstandarddirs.h>

#include <qregexp.h>

extern "C"
{
  KCModule *create_kabcustomfields( QWidget *parent, const char * ) {
    return new KCMKabCustomFields( parent, "kcmkabcustomfields" );
  }
}

KCMKabCustomFields::KCMKabCustomFields( QWidget *parent, const char *name )
  : KCMDesignerFields( parent, name )
{
}

QString KCMKabCustomFields::localUiDir()
{
  return kabLocalDir() + "contacteditorpages/";
}

QString KCMKabCustomFields::uiPath()
{
  return "kaddressbook/contacteditorpages/";
}

void KCMKabCustomFields::writeActivePages( const QStringList &activePages )
{
  KABPrefs::instance()->setAdvancedCustomFields( activePages );
  KABPrefs::instance()->writeConfig();
}

QStringList KCMKabCustomFields::readActivePages()
{
  return KABPrefs::instance()->advancedCustomFields();
}

QString KCMKabCustomFields::applicationName()
{
  return "KADDRESSBOOK";
}

QString KCMKabCustomFields::kabLocalDir()
{
  QStringList kabdirs = locateLocal("data", "kaddressbook/");
  return kabdirs.grep( QRegExp( "^"+KGlobal::dirs()->localkdedir() ) ).first();
}

#include "kcmkabcustomfields.moc"
