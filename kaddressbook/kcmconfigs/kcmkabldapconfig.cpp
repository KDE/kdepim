/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#include <qcheckbox.h>
#include <qframe.h>
#include <qlayout.h>

#include <kaboutdata.h>
#include <klocale.h>

#include "ldapoptionswidget.h"

#include "kcmkabldapconfig.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kabldapconfig( QWidget *parent, const char * ) {
    return new KCMKabLdapConfig( parent, "kcmkabldapconfig" );
  }
}

KCMKabLdapConfig::KCMKabLdapConfig( QWidget *parent, const char *name )
  : KCModule( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  mConfigWidget = new LDAPOptionsWidget( this );
  layout->addWidget( mConfigWidget );

  connect( mConfigWidget, SIGNAL( changed( bool ) ), SIGNAL( changed( bool ) ) );

  load();
  
  KAboutData *about = new KAboutData( I18N_NOOP( "kcmkabldapconfig" ),
                                      I18N_NOOP( "KAB LDAP Configure Dialog" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2003 - 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKabLdapConfig::load()
{
  mConfigWidget->restoreSettings();
}

void KCMKabLdapConfig::save()
{
  mConfigWidget->saveSettings();
}

void KCMKabLdapConfig::defaults()
{
  mConfigWidget->defaults();
}

#include "kcmkabldapconfig.moc"
