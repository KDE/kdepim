/*
   This file is part of the KDE project
   Copyright (C) 2002-2003 Daniel Molkentin <molkentin@kde.org>
   Copyright (C) 2004-2006 Michael Brade <brade@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqpopupmenu.h>
#include <tqclipboard.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kaction.h>
#include <kmessagebox.h>

#include <libkdepim/infoextension.h>
#include <libkdepim/sidebarextension.h>

#include "knotes/knoteprinter.h"
#include "knotes/resourcemanager.h"

#include "knotes_part.h"
#include "knotes_part_p.h"
#include "knotetip.h"


KNotesPart::KNotesPart( TQObject *parent, const char *name )
  : DCOPObject( "KNotesIface" ), KParts::ReadOnlyPart( parent, name ),
    mNotesView( new KNotesIconView() ),
    mNoteTip( new KNoteTip( mNotesView ) ),
    mNoteEditDlg( 0 ),
    mManager( new KNotesResourceManager() )
{
  mNoteList.setAutoDelete( true );

  setInstance( new KInstance( "knotes" ) );

  // create the actions
  new KAction( i18n( "&New" ), "knotes", CTRL+Key_N, this, TQT_SLOT( newNote() ),
               actionCollection(), "file_new" );
  new KAction( i18n( "Rename..." ), "text", this, TQT_SLOT( renameNote() ),
               actionCollection(), "edit_rename" );
  new KAction( i18n( "Delete" ), "editdelete", Key_Delete, this, TQT_SLOT( killSelectedNotes() ),
               actionCollection(), "edit_delete" );
  new KAction( i18n( "Print Selected Notes..." ), "print", CTRL+Key_P, this, TQT_SLOT( printSelectedNotes() ),
               actionCollection(), "print_note" );

  // TODO icons: s/editdelete/knotes_delete/ or the other way round in knotes

  // set the view up
  mNotesView->setSelectionMode( TQIconView::Extended );
  mNotesView->setItemsMovable( false );
  mNotesView->setResizeMode( TQIconView::Adjust );
  mNotesView->setAutoArrange( true );
  mNotesView->setSorting( true );

  connect( mNotesView, TQT_SIGNAL( executed( TQIconViewItem* ) ),
           this, TQT_SLOT( editNote( TQIconViewItem* ) ) );
  connect( mNotesView, TQT_SIGNAL( returnPressed( TQIconViewItem* ) ),
           this, TQT_SLOT( editNote( TQIconViewItem* ) ) );
  connect( mNotesView, TQT_SIGNAL( itemRenamed( TQIconViewItem* ) ),
           this, TQT_SLOT( renamedNote( TQIconViewItem* ) ) );
  connect( mNotesView, TQT_SIGNAL( contextMenuRequested( TQIconViewItem*, const TQPoint& ) ),
           this, TQT_SLOT( popupRMB( TQIconViewItem*, const TQPoint& ) ) );
  connect( mNotesView, TQT_SIGNAL( onItem( TQIconViewItem* ) ),
           this, TQT_SLOT( slotOnItem( TQIconViewItem* ) ) );
  connect( mNotesView, TQT_SIGNAL( onViewport() ),
           this, TQT_SLOT( slotOnViewport() ) );
  connect( mNotesView, TQT_SIGNAL( currentChanged( TQIconViewItem* ) ),
           this, TQT_SLOT( slotOnCurrentChanged( TQIconViewItem* ) ) );

  slotOnCurrentChanged( 0 );

  new KParts::SideBarExtension( mNotesView, this, "NotesSideBarExtension" );

  setWidget( mNotesView );
  setXMLFile( "knotes_part.rc" );

  // connect the resource manager
  connect( mManager, TQT_SIGNAL( sigRegisteredNote( KCal::Journal* ) ),
           this, TQT_SLOT( createNote( KCal::Journal* ) ) );
  connect( mManager, TQT_SIGNAL( sigDeregisteredNote( KCal::Journal* ) ),
           this, TQT_SLOT( killNote( KCal::Journal* ) ) );

  // read the notes
  mManager->load();
}

KNotesPart::~KNotesPart()
{
  delete mNoteTip;
  mNoteTip = 0;

  delete mManager;
  mManager = 0;
}

void KNotesPart::printSelectedNotes()
{
  TQValueList<KCal::Journal*> journals;

  for ( TQIconViewItem *it = mNotesView->firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      journals.append( static_cast<KNotesIconViewItem *>( it )->journal() );
    }
  }

  if ( journals.isEmpty() ) {
    KMessageBox::information( mNotesView, i18n("To print notes, first select the notes to print from the list."), i18n("Print Notes") );
    return;
  }

  KNotePrinter printer;
  printer.printNotes(journals );

#if 0
    TQString content;
    if ( m_editor->textFormat() == PlainText )
        content = TQStyleSheet::convertFromPlainText( m_editor->text() );
    else
        content = m_editor->text();

    KNotePrinter printer;
    printer.setMimeSourceFactory( m_editor->mimeSourceFactory() );
    //printer.setFont( m_config->font() );
    //printer.setContext( m_editor->context() );
    //printer.setStyleSheet( m_editor->styleSheet() );
    printer.setColorGroup( colorGroup() );
    printer.printNote( , content );
#endif
}

bool KNotesPart::openFile()
{
  return false;
}


// public KNotes DCOP interface implementation

TQString KNotesPart::newNote( const TQString& name, const TQString& text )
{
  // create the new note
  KCal::Journal *journal = new KCal::Journal();

  // new notes have the current date/time as title if none was given
  if ( !name.isEmpty() )
      journal->setSummary( name );
  else
      journal->setSummary( KGlobal::locale()->formatDateTime( TQDateTime::currentDateTime() ) );

  // the body of the note
  journal->setDescription( text );



  // Edit the new note if text is empty
  if ( text.isNull() )
  {
    if ( !mNoteEditDlg )
      mNoteEditDlg = new KNoteEditDlg( widget() );

    mNoteEditDlg->setTitle( journal->summary() );
    mNoteEditDlg->setText( journal->description() );

    if ( mNoteEditDlg->exec() == TQDialog::Accepted )
    {
      journal->setSummary( mNoteEditDlg->title() );
      journal->setDescription( mNoteEditDlg->text() );
    }
    else
    {
      delete journal;
      return "";
    }
  }

  mManager->addNewNote( journal );
  mManager->save();

  KNotesIconViewItem *note = mNoteList[ journal->uid() ];
  mNotesView->ensureItemVisible( note );
  mNotesView->setCurrentItem( note );

  return journal->uid();
}

TQString KNotesPart::newNoteFromClipboard( const TQString& name )
{
  const TQString& text = KApplication::clipboard()->text();
  return newNote( name, text );
}

void KNotesPart::killNote( const TQString& id )
{
  killNote( id, false );
}

void KNotesPart::killNote( const TQString& id, bool force )
{
  KNotesIconViewItem *note = mNoteList[ id ];

  if ( note &&
       ( (!force && KMessageBox::warningContinueCancelList( mNotesView,
                    i18n( "Do you really want to delete this note?" ),
                    mNoteList[ id ]->text(), i18n( "Confirm Delete" ),
                    KStdGuiItem::del() ) == KMessageBox::Continue)
         || force )
     )
  {
    mManager->deleteNote( mNoteList[id]->journal() );
    mManager->save();
  }
}

TQString KNotesPart::name( const TQString& id ) const
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note )
    return note->text();
  else
    return TQString::null;
}

TQString KNotesPart::text( const TQString& id ) const
{
  KNotesIconViewItem *note = mNoteList[id];
  if ( note )
    return note->journal()->description();
  else
    return TQString::null;
}

void KNotesPart::setName( const TQString& id, const TQString& newName )
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note ) {
    note->setText( newName );
    mManager->save();
  }
}

void KNotesPart::setText( const TQString& id, const TQString& newText )
{
  KNotesIconViewItem *note = mNoteList[ id ];
  if ( note ) {
    note->journal()->setDescription( newText );
    mManager->save();
  }
}

TQMap<TQString, TQString> KNotesPart::notes() const
{
  TQMap<TQString, TQString> notes;
  TQDictIterator<KNotesIconViewItem> it( mNoteList );

  for ( ; it.current(); ++it )
    notes.insert( (*it)->journal()->uid(), (*it)->journal()->summary() );

  return notes;
}


// private stuff

void KNotesPart::killSelectedNotes()
{
  TQPtrList<KNotesIconViewItem> items;
  TQStringList notes;

  KNotesIconViewItem *knivi;
  for ( TQIconViewItem *it = mNotesView->firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      knivi = static_cast<KNotesIconViewItem *>( it );
      items.append( knivi );
      notes.append( knivi->text() );
    }
  }

  if ( items.isEmpty() )
    return;

  int ret = KMessageBox::warningContinueCancelList( mNotesView,
            i18n( "Do you really want to delete this note?",
                  "Do you really want to delete these %n notes?", items.count() ),
            notes, i18n( "Confirm Delete" ),
            KStdGuiItem::del() );

  if ( ret == KMessageBox::Continue ) {
    TQPtrListIterator<KNotesIconViewItem> kniviIt( items );
    while ( (knivi = *kniviIt) ) {
      ++kniviIt;
      mManager->deleteNote( knivi->journal() );
    }

    mManager->save();
  }
}

void KNotesPart::popupRMB( TQIconViewItem *item, const TQPoint& pos )
{
  TQPopupMenu *contextMenu = NULL;

  if ( item )
    contextMenu = static_cast<TQPopupMenu *>( factory()->container( "note_context", this ) );
  else
    contextMenu = static_cast<TQPopupMenu *>( factory()->container( "notepart_context", this ) );

  if ( !contextMenu )
    return;

  contextMenu->popup( pos );
}

void KNotesPart::slotOnItem( TQIconViewItem *i )
{
  // TODO: disable (i.e. setNote( TQString::null )) when mouse button pressed

  KNotesIconViewItem *item = static_cast<KNotesIconViewItem *>( i );
  mNoteTip->setNote( item );
}

void KNotesPart::slotOnViewport()
{
  mNoteTip->setNote( 0 );
}

// TODO: also with takeItem, clear(),

// create and kill the icon view item corresponding to the note, edit the note

void KNotesPart::createNote( KCal::Journal *journal )
{
  // make sure all fields are existent, initialize them with default values
  TQString property = journal->customProperty( "KNotes", "BgColor" );
  if ( property.isNull() )
    journal->setCustomProperty( "KNotes", "BgColor", "#ffff00" );

  property = journal->customProperty( "KNotes", "FgColor" );
  if ( property.isNull() )
    journal->setCustomProperty( "KNotes", "FgColor", "#000000" );

  property = journal->customProperty( "KNotes", "RichText" );
  if ( property.isNull() )
    journal->setCustomProperty( "KNotes", "RichText", "true" );

  mNoteList.insert( journal->uid(), new KNotesIconViewItem( mNotesView, journal ) );
}

void KNotesPart::killNote( KCal::Journal *journal )
{
  mNoteList.remove( journal->uid() );
}

void KNotesPart::editNote( TQIconViewItem *item )
{
  if ( !mNoteEditDlg )
    mNoteEditDlg = new KNoteEditDlg( widget() );

  KCal::Journal *journal = static_cast<KNotesIconViewItem *>( item )->journal();

  mNoteEditDlg->setRichText( journal->customProperty( "KNotes", "RichText" ) == "true" );
  mNoteEditDlg->setTitle( journal->summary() );
  mNoteEditDlg->setText( journal->description() );

  if ( mNoteEditDlg->exec() == TQDialog::Accepted ) {
    item->setText( mNoteEditDlg->title() );
    journal->setDescription( mNoteEditDlg->text() );
    mManager->save();
  }
}

void KNotesPart::renameNote()
{
  mOldName = mNotesView->currentItem()->text();
  mNotesView->currentItem()->rename();
}

void KNotesPart::renamedNote( TQIconViewItem* )
{
  if ( mOldName != mNotesView->currentItem()->text() )
    mManager->save();
}

void KNotesPart::slotOnCurrentChanged( TQIconViewItem* )
{
  KAction *renameAction = actionCollection()->action( "edit_rename" );
  KAction *deleteAction = actionCollection()->action( "edit_delete" );

  if ( !mNotesView->currentItem() ) {
    renameAction->setEnabled( false );
    deleteAction->setEnabled( false );
  } else {
    renameAction->setEnabled( true );
    deleteAction->setEnabled( true );
  }
}

#include "knotes_part.moc"
#include "knotes_part_p.moc"

