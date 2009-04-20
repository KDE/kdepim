/*
    This file is part of KContactManager.
    Copyright (c) 2009 Laurent Montel <montel@kde.org>
    Based on code Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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
*/

#include "kde2_xxport.h"

#include <QtCore/QFile>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktemporaryfile.h>
#include <kurl.h>

#include "xxportmanager.h"

KDE2XXPort::KDE2XXPort( QWidget *parent )
  : XXPort( parent )
{
}

KABC::Addressee::List KDE2XXPort::importContacts() const
{
  QString fileName = KStandardDirs::locateLocal( "data", "kabc/std.vcf" );
  if ( !QFile::exists( fileName ) ) {
    KMessageBox::sorry( parentWidget(), i18n( "<qt>Could not find a KDE 2 address book <b>%1</b>.</qt>", fileName ) );
    return KABC::AddresseeList();
  }

  int result = KMessageBox::questionYesNoCancel( parentWidget(),
      i18n( "Override previously imported entries?" ),
      i18n( "Import KDE 2 Address Book" ), KGuiItem(i18n("Import")), KGuiItem(i18n("Do Not Import")) );

  if ( !result ) return KABC::AddresseeList();

  KProcess proc;

  if ( result == KMessageBox::Yes ) {
    proc << "kab2kabc";
    proc << "--override";
  } else if ( result == KMessageBox::No )
    proc << "kab2kabc";
  else {
    kDebug(5720) <<"KAddressBook::importKDE2(): Unknow return value.";
    return KABC::AddresseeList();
  }
  proc.execute();

  return KABC::AddresseeList();
}

bool KDE2XXPort::exportContacts( const KABC::Addressee::List &contacts ) const
{
  //we can't export contact to opera yet
  return false;
}
