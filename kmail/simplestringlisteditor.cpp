/*  -*- mode: C++; c-file-style: "gnu" -*-
    simplestringlisteditor.cpp

    This file is part of KMail, the KDE mail client.
    Copyright (c) 2001 Marc Mutz <mutz@kde.org>

    KMail is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License, version 2, as
    published by the Free Software Foundation.

    KMail is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "simplestringlisteditor.h"

#include <kinputdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kpushbutton.h>

#include <tqlayout.h>


//********************************************************
// SimpleStringListEditor
//********************************************************

// small helper function:
static inline TQListBoxItem * findSelectedItem( TQListBox * lb ) {
  TQListBoxItem * item = 0;
  for ( item = lb->firstItem() ; item && !item->isSelected() ;
	item = item->next() ) ;
  return item;
}

SimpleStringListEditor::SimpleStringListEditor( TQWidget * parent,
						const char * name,
						ButtonCode buttons,
						const TQString & addLabel,
						const TQString & removeLabel,
						const TQString & modifyLabel,
						const TQString & addDialogLabel )
  : TQWidget( parent, name ),
    mAddButton(0), mRemoveButton(0), mModifyButton(0),
    mUpButton(0), mDownButton(0),
    mAddDialogLabel( addDialogLabel.isEmpty() ?
		     i18n("New entry:") : addDialogLabel )
{
  TQHBoxLayout * hlay = new TQHBoxLayout( this, 0, KDialog::spacingHint() );

  mListBox = new TQListBox( this );
  hlay->addWidget( mListBox, 1 );

  if ( buttons == None )
    kdDebug(5006) << "SimpleStringListBox called with no buttons. "
      "Consider using a plain TQListBox instead!" << endl;

  TQVBoxLayout * vlay = new TQVBoxLayout( hlay ); // inherits spacing

  if ( buttons & Add ) {
    if ( addLabel.isEmpty() )
      mAddButton = new TQPushButton( i18n("&Add..."), this );
    else
      mAddButton = new TQPushButton( addLabel, this );
    mAddButton->setAutoDefault( false );
    vlay->addWidget( mAddButton );
    connect( mAddButton, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(slotAdd()) );
  }

  if ( buttons & Remove ) {
    if ( removeLabel.isEmpty() )
      mRemoveButton = new TQPushButton( i18n("&Remove"), this );
    else
      mRemoveButton = new TQPushButton( removeLabel, this );
    mRemoveButton->setAutoDefault( false );
    mRemoveButton->setEnabled( false ); // no selection yet
    vlay->addWidget( mRemoveButton );
    connect( mRemoveButton, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(slotRemove()) );
  }

  if ( buttons & Modify ) {
    if ( modifyLabel.isEmpty() )
      mModifyButton = new TQPushButton( i18n("&Modify..."), this );
    else
      mModifyButton = new TQPushButton( modifyLabel, this );
    mModifyButton->setAutoDefault( false );
    mModifyButton->setEnabled( false ); // no selection yet
    vlay->addWidget( mModifyButton );
    connect( mModifyButton, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(slotModify()) );
    connect( mListBox, TQT_SIGNAL( doubleClicked( TQListBoxItem* ) ),
             this, TQT_SLOT( slotModify() ) );
  }

  if ( buttons & Up ) {
    if ( !(buttons & Down) )
      kdDebug(5006) << "Are you sure you want to use an Up button "
	"without a Down button??" << endl;
    mUpButton = new KPushButton( TQString::null, this );
    mUpButton->setIconSet( BarIconSet( "up", KIcon::SizeSmall ) );
    mUpButton->setAutoDefault( false );
    mUpButton->setEnabled( false ); // no selection yet
    vlay->addWidget( mUpButton );
    connect( mUpButton, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(slotUp()) );
  }

  if ( buttons & Down ) {
    if ( !(buttons & Up) )
      kdDebug(5006) << "Are you sure you want to use a Down button "
	"without an Up button??" << endl;
    mDownButton = new KPushButton( TQString::null, this );
    mDownButton->setIconSet( BarIconSet( "down", KIcon::SizeSmall ) );
    mDownButton->setAutoDefault( false );
    mDownButton->setEnabled( false ); // no selection yet
    vlay->addWidget( mDownButton );
    connect( mDownButton, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(slotDown()) );
  }

  vlay->addStretch( 1 ); // spacer

  connect( mListBox, TQT_SIGNAL(selectionChanged()),
	   this, TQT_SLOT(slotSelectionChanged()) );
}

void SimpleStringListEditor::setStringList( const TQStringList & strings ) {
  mListBox->clear();
  mListBox->insertStringList( strings );
}

void SimpleStringListEditor::appendStringList( const TQStringList & strings ) {
  mListBox->insertStringList( strings );
}

TQStringList SimpleStringListEditor::stringList() const {
  TQStringList result;
  for ( TQListBoxItem * item = mListBox->firstItem() ;
	item ; item = item->next() )
    result << item->text();
  return result;
}

bool SimpleStringListEditor::containsString( const TQString & str ) {
  for ( TQListBoxItem * item = mListBox->firstItem() ;
	item ; item = item->next() ) {
    if ( item->text() == str )
      return true;
  }
  return false;
}

void SimpleStringListEditor::setButtonText( ButtonCode button,
					    const TQString & text ) {
  switch ( button ) {
  case Add:
    if ( !mAddButton ) break;
    mAddButton->setText( text );
    return;
  case Remove:
    if ( !mRemoveButton ) break;
    mRemoveButton->setText( text );
    return;
  case Modify:
    if ( !mModifyButton ) break;
    mModifyButton->setText( text );
    return;
  case Up:
  case Down:
    kdDebug(5006) << "SimpleStringListEditor: Cannot change text of "
      "Up and Down buttons: they don't contains text!" << endl;
    return;
  default:
    if ( button & All )
      kdDebug(5006) << "SimpleStringListEditor::setButtonText: No such button!"
		    << endl;
    else
      kdDebug(5006) << "SimpleStringListEditor::setButtonText: Can only set "
	"text for one button at a time!" << endl;
    return;
  }

  kdDebug(5006) << "SimpleStringListEditor::setButtonText: the requested "
    "button has not been created!" << endl;
}

void SimpleStringListEditor::slotAdd() {
  bool ok = false;
  TQString newEntry = KInputDialog::getText( i18n("New Value"),
                                            mAddDialogLabel, TQString::null,
					    &ok, this );
  // let the user verify the string before adding
  emit aboutToAdd( newEntry );
  if ( ok && !newEntry.isEmpty() && !containsString( newEntry )) {
      mListBox->insertItem( newEntry );
      emit changed();
  }
}

void SimpleStringListEditor::slotRemove() {
  delete findSelectedItem( mListBox ); // delete 0 is well-behaved...
  emit changed();
}

void SimpleStringListEditor::slotModify() {
  TQListBoxItem * item = findSelectedItem( mListBox );
  if ( !item ) return;

  bool ok = false;
  TQString newText = KInputDialog::getText( i18n("Change Value"),
                                           mAddDialogLabel, item->text(),
					   &ok, this );
  emit aboutToAdd( newText );
  if ( !ok || newText.isEmpty() || newText == item->text() ) return;

  int index = mListBox->index( item );
  delete item;
  mListBox->insertItem( newText, index );
  mListBox->setCurrentItem( index );
  emit changed();
}

void SimpleStringListEditor::slotUp() {
  TQListBoxItem * item = findSelectedItem( mListBox );
  if ( !item || !item->prev() ) return;

  // find the item that we want to insert after:
  TQListBoxItem * pprev = item->prev()->prev();
  // take the item from it's current position...
  mListBox->takeItem( item );
  // ...and insert it after the above mentioned item:
  mListBox->insertItem( item, pprev );
  // make sure the old item is still the current one:
  mListBox->setCurrentItem( item );
  // enable and disable controls:
  if ( mRemoveButton )
    mRemoveButton->setEnabled( true );
  if ( mModifyButton )
    mModifyButton->setEnabled( true );
  if ( mUpButton )
    mUpButton->setEnabled( item->prev() );
  if ( mDownButton )
    mDownButton->setEnabled( true );
  emit changed();
}

void SimpleStringListEditor::slotDown() {
  TQListBoxItem * item  = findSelectedItem( mListBox );
  if ( !item || !item->next() ) return;

  // find the item that we want to insert after:
  TQListBoxItem * next = item->next();
  // take the item from it's current position...
  mListBox->takeItem( item );
  // ...and insert it after the above mentioned item:
  if ( next )
    mListBox->insertItem( item, next );
  else
    mListBox->insertItem( item );
  // make sure the old item is still the current one:
  mListBox->setCurrentItem( item );
  // enable and disable controls:
  if ( mRemoveButton )
    mRemoveButton->setEnabled( true );
  if ( mModifyButton )
    mModifyButton->setEnabled( true );
  if ( mUpButton )
    mUpButton->setEnabled( true );
  if ( mDownButton )
    mDownButton->setEnabled( item->next() );
  emit changed();
}

void SimpleStringListEditor::slotSelectionChanged() {
  // try to find a selected item:
  TQListBoxItem * item = findSelectedItem( mListBox );

  // if there is one, item will be non-null (ie. true), else 0
  // (ie. false):
  if ( mRemoveButton )
    mRemoveButton->setEnabled( item );
  if ( mModifyButton )
    mModifyButton->setEnabled( item );
  if ( mUpButton )
    mUpButton->setEnabled( item && item->prev() );
  if ( mDownButton )
    mDownButton->setEnabled( item && item->next() );
}



#include "simplestringlisteditor.moc"
