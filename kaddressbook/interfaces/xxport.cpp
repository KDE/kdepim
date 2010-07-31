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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqmap.h>
#include <tqsignalmapper.h>

#include <kaction.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include "xxport.h"

using namespace KAB;

class XXPort::XXPortPrivate
{
  public:
    TQSignalMapper *mExportMapper;
    TQSignalMapper *mImportMapper;
    KApplication *mKApp;
};

XXPort::XXPort( KABC::AddressBook *ab, TQWidget *parent,
                            const char *name )
  : TQObject( parent, name ), mAddressBook( ab ), mParentWidget( parent ),
    d( new XXPortPrivate )
{
  setInstance( new KInstance( "kaddressbook" ) );

  d->mExportMapper = new TQSignalMapper( this );
  d->mImportMapper = new TQSignalMapper( this );

  connect( d->mExportMapper, TQT_SIGNAL( mapped( const TQString& ) ),
           TQT_SLOT( slotExportActivated( const TQString& ) ) );
  connect( d->mImportMapper, TQT_SIGNAL( mapped( const TQString& ) ),
           TQT_SLOT( slotImportActivated( const TQString& ) ) );
}

XXPort::~XXPort()
{
  delete d;
  d = 0;
}

bool XXPort::exportContacts( const KABC::AddresseeList&, const TQString& )
{
  // do nothing
  return false;
}

KABC::AddresseeList XXPort::importContacts( const TQString& ) const
{
  // do nothing
  return KABC::AddresseeList();
}

void XXPort::createImportAction( const TQString &label, const TQString &data )
{
  TQString id = "file_import_" + identifier() + ( data.isEmpty() ? TQString( "" ) : "_" + data );
  KAction *action = new KAction( label, 0, d->mImportMapper, TQT_SLOT( map() ), actionCollection(), id.latin1() );

  d->mImportMapper->setMapping( action, ( data.isEmpty() ? TQString( "<empty>" ) : data ) );

  setXMLFile( identifier() + "_xxportui.rc" );
}

void XXPort::createExportAction( const TQString &label, const TQString &data )
{
  TQString id = "file_export_" + identifier() + ( data.isEmpty() ? TQString( "" ) : "_" + data );
  KAction *action = new KAction( label, 0, d->mExportMapper, TQT_SLOT( map() ), actionCollection(), id.latin1() );

  d->mExportMapper->setMapping( action, ( data.isEmpty() ? TQString( "<empty>" ) : data ) );

  setXMLFile( identifier() + "_xxportui.rc" );
}

KABC::AddressBook *XXPort::addressBook() const
{
  return mAddressBook;
}

TQWidget *XXPort::parentWidget() const
{
  return mParentWidget;
}

void XXPort::setKApplication( KApplication *app )
{
  d->mKApp = app;
}

void XXPort::processEvents() const
{
  if ( d->mKApp )
    d->mKApp->processEvents();
}

void XXPort::slotExportActivated( const TQString &data )
{
  emit exportActivated( identifier(), ( data == "<empty>" ? TQString::null : data ) );
}

void XXPort::slotImportActivated( const TQString &data )
{
  emit importActivated( identifier(), ( data == "<empty>" ? TQString::null : data ) );
}

#include "xxport.moc"
