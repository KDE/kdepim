/*
    This file is part of kdepim.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <qfile.h>
#include <qfiledialog.h>
#include <qlayout.h>
#include <qpopupmenu.h>

#include <klistview.h>
#include <kstaticdeleter.h>

#include <stdlib.h>

#include "debugdialog.h"

DebugDialog* DebugDialog::mSelf = 0;
static KStaticDeleter<DebugDialog> debugDialogDeleter;

DebugDialog::DebugDialog()
 : KDialogBase( Plain, WStyle_DialogBorder | WStyle_StaysOnTop, 0,
                "Debug Dialog", false, "DebugDialog",
                User1 | User2 | Ok, Ok, true )
{
  QWidget *page = plainPage();
  QVBoxLayout *layout = new QVBoxLayout( page, marginHint(), spacingHint() );

  mView = new KListView( page );
  mView->addColumn( "" );
  mView->setFullWidth( true );
  mView->setRootIsDecorated( true );

  layout->addWidget( mView );

  setButtonText( User1, "Save As..." );
  setButtonText( User2, "Clear" );

  connect( mView, SIGNAL( contextMenuRequested( QListViewItem*, const QPoint&, int ) ),
           this, SLOT( contextMenuRequested( QListViewItem*, const QPoint&, int ) ) );

  clear();
}

void DebugDialog::init()
{
  if ( !mSelf ) {
    if ( getenv( "EGROUPWARE_DEBUG" ) != 0 ) {
      debugDialogDeleter.setObject( mSelf, new DebugDialog );
    }
  }

  if ( mSelf ) {
    mSelf->show();
    mSelf->raise();
  }
}

DebugDialog::~DebugDialog()
{
  mSelf = 0;
}

void DebugDialog::addMessage( const QString &msg, Type type )
{
  if ( mSelf )
    mSelf->addText( msg, type );
}

void DebugDialog::clear()
{
  mView->clear();
  new QListViewItem( mView, "Communication Protocol" );

  mMessages.clear();
}

void DebugDialog::save()
{
  QString fileName = QFileDialog::getSaveFileName();
  if ( fileName.isEmpty() )
    return;

  QFile file( fileName );
  if ( !file.open( IO_WriteOnly ) ) {
    qWarning( "Couldn't open file %s", file.name().latin1() );
    return;
  }

  file.writeBlock( mMessages.join( "\n\n" ).utf8() );
  file.close();
}

void DebugDialog::slotUser1()
{
  save();
}

void DebugDialog::slotUser2()
{
  clear();
}

void DebugDialog::addText( const QString &text, Type type )
{
  mMessages.append( text );

  addNode( text, type );
}

void DebugDialog::addNode( const QString &text, Type type )
{
  QDomDocument document( "XMLMessage" );

  document.setContent( text );

  QDomElement documentElement = document.documentElement(); 

  QListViewItem *item = new QListViewItem( mView->firstChild(), type == Input ? "Input" : "Output" );

  addSubNode( documentElement, item );

  mView->setOpen( item, true );
}

void DebugDialog::addSubNode( const QDomElement &parentElement, QListViewItem *parentItem )
{
  QDomNode node = parentElement.firstChild();
  while ( !node.isNull() ) {
    QDomElement element = node.toElement();
    if ( !element.isNull() ) {
      QString name = element.tagName();

      bool hasChildNodes = false;
      QDomNodeList nodes = element.childNodes();
      for ( uint i = 0; i < nodes.count(); ++i )
        if ( !nodes.item( i ).isText() )
          hasChildNodes = true;

      if ( !hasChildNodes )
        name += (element.text().isEmpty() ? "" : "( " + element.text() + " )");
      QListViewItem *item = new QListViewItem( parentItem, name );
      addSubNode( element, item );
    }

    node = node.nextSibling();
  }
}

void DebugDialog::contextMenuRequested( QListViewItem *item, const QPoint &point, int )
{
  QPopupMenu menu( this );
  menu.insertItem( "Expand Item", 1 );
  menu.insertItem( "Collaps Item", 2 );
  switch ( menu.exec( point ) ) {
    case 1:
      {
        QListViewItemIterator it( item );
        while ( it.current() ) {
          mView->setOpen( it.current(), true );
          ++it;
          if ( it.current() == item->nextSibling() )
            break;
        }
      }
      break;
    case 2:
      {
        QListViewItemIterator it( item );
        while ( it.current() ) {
          mView->setOpen( it.current(), false );
          ++it;
          if ( it.current() == item->nextSibling() )
            break;
        }
      }
      break;
    default:
      break;
  }
}

#include "debugdialog.moc"
