/*
    This file is part of KAddressbook.
    Copyright (c) 2000 Hans Dijkema <kmailcvt@hum.org>
    Copyright (c) 2003 Helge Deller <deller@kde.org>

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

#include <qfile.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kurl.h>

#include "xxportmanager.h"

#include "pab_xxport.h"

K_EXPORT_KADDRESSBOOK_XXFILTER( libkaddrbk_pab_xxport, PABXXPort )

PABXXPort::PABXXPort( KABC::AddressBook *ab, QWidget *parent, const char *name )
  : KAB::XXPort( ab, parent, name )
{
  createImportAction( i18n("Import MS Exchange Personal Address Book (.PAB)") );
}

KABC::AddresseeList PABXXPort::importContacts( const QString& ) const
{
  KABC::AddresseeList addrList;

  QString fileName = KFileDialog::getOpenFileName( QDir::homeDirPath(), 
      		"*.[pP][aA][bB]|" + i18n("MS Exchange Personal Address Book Files (*.pab)"), 0 );
  if ( fileName.isEmpty() )
    return addrList;
  if ( !QFile::exists( fileName ) ) {
    KMessageBox::sorry( parentWidget(), i18n( "<qt>Could not find a MS Exchange Personal Address Book <b>%1</b>.</qt>" ).arg( fileName ) );
    return addrList;
  }

  // pab PAB(QFile::encodeName(file),this,info);
  // info->setFrom(file);
  // PAB.convert();

  return addrList;
}

#include "pab_xxport.moc"

// vim: ts=2 sw=2 et
