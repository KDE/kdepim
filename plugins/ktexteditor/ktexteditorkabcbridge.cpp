/* This file is part of the KDE libraries
  Copyright (C) 2005 Joseph Wenninger <jowenn@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include <kabc/addressee.h>
#include <kcmultidialog.h>
#include <kmessagebox.h>
#include <KIdentityManagement/kidentitymanagement/identity.h>
#include <KIdentityManagement/kidentitymanagement/identitymanager.h>
#include <KLocalizedString>

using namespace KIdentityManagement;

// extern "C" is needed here because this function must be located using
// QLibrary resolve() that only knows how to resolve C functions
// This function is used by kate/kwrite
extern "C" Q_DECL_EXPORT QString ktexteditorkabcbridge( const QString &placeHolder, QWidget *widget, bool *ok )
{
  IdentityManager manager( true, widget );
  Identity defaultIdentity = manager.defaultIdentity();

  if ( defaultIdentity.fullName().isEmpty() && defaultIdentity.primaryEmailAddress().isEmpty() ) {
    const int result = KMessageBox::questionYesNo( widget,
                                                   i18n( "The template needs information about you, but it looks as if you have not yet provided that information. Do you want to provide it now?" ),
                                                   i18n( "Missing personal information" ) );

    if ( result == KMessageBox::No ) {
      *ok = false;
      return QString();
    }

    KCMultiDialog dlg( widget );
    dlg.addModule( QLatin1String("kcm_useraccount.desktop") );
    if ( !dlg.exec() ) {
      *ok = false;
      return QString();
    }
  }

  defaultIdentity = manager.defaultIdentity();

  if ( ok )
    *ok = true;

  KABC::Addressee contact;
  contact.setNameFromString( defaultIdentity.fullName() );

  if ( placeHolder == QLatin1String("firstname") )
    return contact.givenName();
  else if ( placeHolder == QLatin1String("lastname") )
    return contact.familyName();
  else if ( placeHolder == QLatin1String("fullname") )
    return contact.assembledName();
  else if ( placeHolder == QLatin1String("email") )
    return defaultIdentity.primaryEmailAddress();
  else
    return QString();
}
