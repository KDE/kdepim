/*
    This file is part of KAddressbook.
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

#include <qmap.h>
#include <qsignalmapper.h>

#include <kaction.h>
#include <kmessagebox.h>
#include <kinstance.h>
#include "xxport.h"

using namespace KAB;

class XXPort::XXPortPrivate
{
  public:
    QSignalMapper *mExportMapper;
    QSignalMapper *mImportMapper;
};

XXPort::XXPort( KABC::AddressBook *ab, QWidget *parent,
                            const char *name )
  : QObject( parent, name ), mAddressBook( ab ), mParentWidget( parent ),
    d( new XXPortPrivate )
{
  setInstance( new KInstance( "kaddressbook" ) );

  d->mExportMapper = new QSignalMapper( this );
  d->mImportMapper = new QSignalMapper( this );

  connect( d->mExportMapper, SIGNAL( mapped( const QString& ) ),
           SLOT( slotExportActivated( const QString& ) ) );
  connect( d->mImportMapper, SIGNAL( mapped( const QString& ) ),
           SLOT( slotImportActivated( const QString& ) ) );
}

XXPort::~XXPort()
{
  delete d;
  d = 0;
}

bool XXPort::exportContacts( const KABC::AddresseeList&, const QString& )
{
  // do nothing
  return false;
}

KABC::AddresseeList XXPort::importContacts( const QString& ) const
{
  // do nothing
  return KABC::AddresseeList();
}

void XXPort::createImportAction( const QString &label, const QString &data )
{
  QString id = "file_import_" + identifier() + ( data.isEmpty() ? QString( "" ) : "_" + data );
  KAction *action = new KAction( label, 0, d->mImportMapper, SLOT( map() ), actionCollection(), id.latin1() );

  d->mImportMapper->setMapping( action, ( data.isEmpty() ? QString( "<empty>" ) : data ) );

  setXMLFile( identifier() + "_xxportui.rc" );
}

void XXPort::createExportAction( const QString &label, const QString &data )
{
  QString id = "file_export_" + identifier() + ( data.isEmpty() ? QString( "" ) : "_" + data );
  KAction *action = new KAction( label, 0, d->mExportMapper, SLOT( map() ), actionCollection(), id.latin1() );

  d->mExportMapper->setMapping( action, ( data.isEmpty() ? QString( "<empty>" ) : data ) );

  setXMLFile( identifier() + "_xxportui.rc" );
}

KABC::AddressBook *XXPort::addressBook() const
{
  return mAddressBook;
}

QWidget *XXPort::parentWidget() const
{
  return mParentWidget;
}

void XXPort::slotExportActivated( const QString &data )
{
  emit exportActivated( identifier(), ( data == "<empty>" ? QString::null : data ) );
}

void XXPort::slotImportActivated( const QString &data )
{
  emit importActivated( identifier(), ( data == "<empty>" ? QString::null : data ) );
}

#include "xxport.moc"
