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

#include <kabc/addresseelist.h>
#include <kaction.h>
#include <kmessagebox.h>

#include "kabcore.h"

#include "xxportobject.h"

class XXPortObject::XXPortObjectPrivate
{
  public:
    KABCore *mCore;

    QSignalMapper *mExportMapper;
    QSignalMapper *mImportMapper;
};

XXPortObject::XXPortObject( KABCore *core, QObject *parent, const char *name )
  : QObject( parent, name ), d( new XXPortObjectPrivate )
{
  d->mCore = core;
  d->mExportMapper = new QSignalMapper( this );
  d->mImportMapper = new QSignalMapper( this );

  connect( d->mExportMapper, SIGNAL( mapped( const QString& ) ),
           SLOT( slotExportActivated( const QString& ) ) );
  connect( d->mImportMapper, SIGNAL( mapped( const QString& ) ),
           SLOT( slotImportActivated( const QString& ) ) );
}

XXPortObject::~XXPortObject()
{
  delete d;
  d = 0;
}

bool XXPortObject::exportContacts( const KABC::AddresseeList&, const QString& )
{
  // do nothing
  return false;
}

KABC::AddresseeList XXPortObject::importContacts( const QString& ) const
{
  // do nothing
  return KABC::AddresseeList();
}

void XXPortObject::createImportAction( const QString &label, const QString &data )
{
  QString id = "file_import_" + identifier() + ( data.isEmpty() ? QString( "" ) : "_" + data );
  KAction *action = new KAction( label, 0, d->mImportMapper, SLOT( map() ), d->mCore->actionCollection(), id );

  d->mImportMapper->setMapping( action, ( data.isEmpty() ? QString( "<empty>" ) : data ) );

  setXMLFile( identifier() + "_xxportui.rc" );
}

void XXPortObject::createExportAction( const QString &label, const QString &data )
{
  QString id = "file_export_" + identifier() + ( data.isEmpty() ? QString( "" ) : "_" + data );
  KAction *action = new KAction( label, 0, d->mExportMapper, SLOT( map() ), d->mCore->actionCollection(), id );

  d->mExportMapper->setMapping( action, ( data.isEmpty() ? QString( "<empty>" ) : data ) );

  setXMLFile( identifier() + "_xxportui.rc" );
}

KABCore *XXPortObject::core() const
{
  return d->mCore;
}

void XXPortObject::slotExportActivated( const QString &data )
{
  emit exportActivated( identifier(), ( data == "<empty>" ? QString::null : data ) );
}

void XXPortObject::slotImportActivated( const QString &data )
{
  emit importActivated( identifier(), ( data == "<empty>" ? QString::null : data ) );
}

#include "xxportobject.moc"
