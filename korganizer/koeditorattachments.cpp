/*
    This file is part of KOrganizer.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "koeditorattachments.h"

#include <libkcal/attachmenthandler.h>
#include <libkcal/incidence.h>
#include <libkdepim/kpimurlrequesterdlg.h>
#include <libkdepim/kfileio.h>
#include <libkdepim/kdepimprotocols.h>
#include <libkdepim/maillistdrag.h>
#include <libkdepim/kvcarddrag.h>
#include <libkdepim/kdepimprotocols.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <krun.h>
#include <kurldrag.h>
#include <ktempfile.h>
#include <ktempdir.h>
#include <kio/netaccess.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <kstdaction.h>
#include <kactioncollection.h>
#include <kpopupmenu.h>
#include <kprotocolinfo.h>
#include <klineedit.h>
#include <kseparator.h>
#include <kurlrequester.h>
#include <libkmime/kmime_message.h>

#include <tqcheckbox.h>
#include <tqfile.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlistview.h>
#include <tqpushbutton.h>
#include <tqdragobject.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>
#include <tqapplication.h>
#include <tqclipboard.h>

#include <cassert>
#include <cstdlib>

class AttachmentListItem : public KIconViewItem
{
  public:
    AttachmentListItem( KCal::Attachment*att, TQIconView *parent ) :
        KIconViewItem( parent )
    {
      if ( att ) {
        mAttachment = new KCal::Attachment( *att );
      } else {
        mAttachment = new KCal::Attachment( '\0' ); //use the non-uri constructor
                                                    //as we want inline by default
      }
      readAttachment();
      setDragEnabled( true );
    }
    ~AttachmentListItem() { delete mAttachment; }
    KCal::Attachment *attachment() const { return mAttachment; }

    const TQString uri() const
    {
      return mAttachment->uri();
    }
    void setUri( const TQString &uri )
    {
      mAttachment->setUri( uri );
      readAttachment();
    }
    void setData( const TQByteArray data )
    {
      mAttachment->setDecodedData( data );
      readAttachment();
    }
    const TQString mimeType() const
    {
      return mAttachment->mimeType();
    }
    void setMimeType( const TQString &mime )
    {
      mAttachment->setMimeType( mime );
      readAttachment();
    }
    const TQString label() const
    {
      return mAttachment->label();
    }
    void setLabel( const TQString &label )
    {
      mAttachment->setLabel( label );
      readAttachment();
    }
    bool isBinary() const
    {
      return mAttachment->isBinary();
    }
    TQPixmap icon() const
    {
      return icon( KMimeType::mimeType( mAttachment->mimeType() ),
                   mAttachment->uri() );
    }
    static TQPixmap icon( KMimeType::Ptr mimeType, const TQString &uri )
    {
      TQString iconStr = mimeType->icon( uri, false );
      return KGlobal::iconLoader()->loadIcon( iconStr, KIcon::Small );
    }
    void readAttachment()
    {
      if ( mAttachment->label().isEmpty() ) {
        if ( mAttachment->isUri() ) {
          setText( mAttachment->uri() );
        } else {
          setText( i18n( "[Binary data]" ) );
        }
      } else {
        setText( mAttachment->label() );
      }
      if ( mAttachment->mimeType().isEmpty() ||
           !( KMimeType::mimeType( mAttachment->mimeType() ) ) ) {
        KMimeType::Ptr mimeType;
        if ( mAttachment->isUri() ) {
          mimeType = KMimeType::findByURL( mAttachment->uri() );
        } else {
          mimeType = KMimeType::findByContent( mAttachment->decodedData() );
        }
        mAttachment->setMimeType( mimeType->name() );
      }

      setPixmap( icon() );
    }

  private:
    KCal::Attachment *mAttachment;
};

AttachmentEditDialog::AttachmentEditDialog( AttachmentListItem *item,
                                            TQWidget *parent )
  : KDialogBase ( Plain, i18n( "Add Attachment" ), Ok|Cancel, Ok, parent, 0, false, false ),
    mItem( item ), mURLRequester( 0 )
{
  TQFrame *topFrame = plainPage();
  TQVBoxLayout *vbl = new TQVBoxLayout( topFrame, 0, spacingHint() );

  TQGridLayout *grid = new TQGridLayout();
  grid->setColStretch( 0, 0 );
  grid->setColStretch( 1, 0 );
  grid->setColStretch( 2, 1 );
  vbl->addLayout( grid );

  mIcon = new TQLabel( topFrame );
  mIcon->setPixmap( item->icon() );
  grid->addWidget( mIcon, 0, 0 );

  mLabelEdit = new KLineEdit( topFrame );
  mLabelEdit->setText( item->label().isEmpty() ? item->uri() : item->label() );
  mLabelEdit->setClickMessage( i18n( "Attachment name" ) );
  TQToolTip::add( mLabelEdit, i18n( "Give the attachment a name" ) );
  TQWhatsThis::add( mLabelEdit,
                   i18n( "Type any string you desire here for the name of the attachment" ) );
  grid->addMultiCellWidget( mLabelEdit, 0, 0, 1, 2 );

  KSeparator *sep = new KSeparator( TQt::Horizontal, topFrame );
  grid->addMultiCellWidget( sep, 1, 1, 0, 2 );

  TQLabel *label = new TQLabel( i18n( "Type:" ), topFrame );
  grid->addWidget( label, 2, 0 );
  TQString typecomment = item->mimeType().isEmpty() ?
                        i18n( "Unknown" ) :
                        KMimeType::mimeType( item->mimeType() )->comment();
  mTypeLabel = new TQLabel( typecomment, topFrame );
  grid->addWidget( mTypeLabel, 2, 1 );
  mMimeType = KMimeType::mimeType( item->mimeType() );

  mInline = new TQCheckBox( i18n( "Store attachment inline" ), topFrame );
  grid->addMultiCellWidget( mInline, 3, 3, 0, 2 );
  mInline->setChecked( item->isBinary() );
  TQToolTip::add( mInline, i18n( "Store the attachment file inside the calendar" ) );
  TQWhatsThis::add(
    mInline,
    i18n( "Checking this option will cause the attachment to be stored inside "
          "your calendar, which can take a lot of space depending on the size "
          "of the attachment. If this option is not checked, then only a link "
          "pointing to the attachment will be stored.  Do not use a link for "
          "attachments that change often or may be moved (or removed) from "
          "their current location." ) );

  if ( item->attachment()->isUri() || !item->attachment()->data() ) {
    label = new TQLabel( i18n( "Location:" ), topFrame );
    grid->addWidget( label, 4, 0 );
    mURLRequester = new KURLRequester( item->uri(), topFrame );
    TQToolTip::add( mURLRequester, i18n( "Provide a location for the attachment file" ) );
    TQWhatsThis::add(
      mURLRequester,
      i18n( "Enter the path to the attachment file or use the "
            "file browser by pressing the adjacent button" ) );
    grid->addMultiCellWidget( mURLRequester, 4, 4, 1, 2 );
    connect( mURLRequester, TQT_SIGNAL(urlSelected(const TQString &)),
             TQT_SLOT(urlSelected(const TQString &)) );
    connect( mURLRequester, TQT_SIGNAL( textChanged( const TQString& ) ),
             TQT_SLOT( urlChanged( const TQString& ) ) );
    urlChanged( item->uri() );
  } else {
    uint size = item->attachment()->size();
    grid->addWidget( new TQLabel( i18n( "Size:" ), topFrame ), 4, 0 );
    grid->addWidget( new TQLabel( TQString::fromLatin1( "%1 (%2)" ).
                                 arg( KIO::convertSize( size ) ).
                                 arg( KGlobal::locale()->formatNumber(
                                        size, 0 ) ), topFrame ), 4, 2 );
  }
  vbl->addStretch( 10 );
}

void AttachmentEditDialog::slotApply()
{
  if ( !mLabelEdit->text().isEmpty() ) {
    mItem->setLabel( mLabelEdit->text() );
  } else {
    if ( mURLRequester ) {
      KURL url( mURLRequester->url() );
      if ( url.isLocalFile() ) {
        mItem->setLabel( url.fileName() );
      } else {
        mItem->setLabel( url.url() );
      }
    }
  }
  if ( mItem->label().isEmpty() ) {
    mItem->setLabel( i18n( "New attachment" ) );
  }
  mItem->setMimeType( mMimeType->name() );
  if ( mURLRequester ) {
    KURL url( mURLRequester->url() );

    TQString correctedUrl = mURLRequester->url();
    if ( !url.isValid() ) {
      // If the user used KURLRequester's KURLCompletion
      // (used the line edit instead of the file dialog)
      // the returned url is not absolute and is always relative
      // to the home directory (not pwd), so we must prepend home

      correctedUrl = TQDir::home().filePath( mURLRequester->url() );
      url = KURL( correctedUrl );
      if ( url.isValid() ) {
        urlSelected( correctedUrl );
        mItem->setMimeType( mMimeType->name() );
      }
    }

    if ( mInline->isChecked() ) {
      TQString tmpFile;
      if ( KIO::NetAccess::download( correctedUrl, tmpFile, this ) ) {
        TQFile f( tmpFile );
        if ( !f.open( IO_ReadOnly ) ) {
          return;
        }
        TQByteArray data = f.readAll();
        f.close();
        mItem->setData( data );
      }
      KIO::NetAccess::removeTempFile( tmpFile );
    } else {
      mItem->setUri( url.url() );
    }
  }
}

void AttachmentEditDialog::accept()
{
  slotApply();
  KDialog::accept();
}

void AttachmentEditDialog::urlChanged( const TQString &url )
{
  enableButton( Ok, !url.isEmpty() );
}

void AttachmentEditDialog::urlSelected( const TQString &url )
{
  KURL kurl( url );
  mMimeType = KMimeType::findByURL( kurl );
  mTypeLabel->setText( mMimeType->comment() );
  mIcon->setPixmap( AttachmentListItem::icon( mMimeType, kurl.path() ) );
}

AttachmentIconView::AttachmentIconView( KOEditorAttachments* parent )
  : KIconView( parent ),
    mParent( parent )
{
  setSelectionMode( TQIconView::Extended );
  setMode( KIconView::Select );
  setItemTextPos( TQIconView::Right );
  setArrangement( TQIconView::LeftToRight );
  setMaxItemWidth( QMAX(maxItemWidth(), 250) );
  setMinimumHeight( QMAX(fontMetrics().height(), 16) + 12 );

  connect( this, TQT_SIGNAL( dropped ( TQDropEvent *, const TQValueList<TQIconDragItem> & ) ),
           this, TQT_SLOT( handleDrop( TQDropEvent *, const TQValueList<TQIconDragItem> & ) ) );
}

KURL AttachmentIconView::tempFileForAttachment( KCal::Attachment *attachment )
{
  if ( mTempFiles.contains( attachment ) ) {
    return mTempFiles[attachment];
  }
  TQStringList patterns = KMimeType::mimeType( attachment->mimeType() )->patterns();

  KTempFile *file;
  if ( !patterns.empty() ) {
    file = new KTempFile( TQString::null,
                          TQString( patterns.first() ).remove( '*' ),0600 );
  } else {
    file = new KTempFile( TQString::null, TQString::null, 0600 );
  }
  file->setAutoDelete( true );
  file->file()->open( IO_WriteOnly );
  TQTextStream stream( file->file() );
  stream.writeRawBytes( attachment->decodedData().data(), attachment->size() );
  KURL url( file->name() );
  mTempFiles.insert( attachment, url );
  file->close();
  return mTempFiles[attachment];
}

TQDragObject *AttachmentIconView::mimeData()
{
  // create a list of the URL:s that we want to drag
  KURL::List urls;
  TQStringList labels;
  for ( TQIconViewItem *it = firstItem(); it; it = it->nextItem() ) {
    if ( it->isSelected() ) {
      AttachmentListItem *item = static_cast<AttachmentListItem *>( it );
      if ( item->isBinary() ) {
        urls.append( tempFileForAttachment( item->attachment() ) );
      } else {
        urls.append( item->uri() );
      }
      labels.append( KURL::encode_string( item->label() ) );
    }
  }
  if ( selectionMode() == TQIconView::NoSelection ) {
    AttachmentListItem *item = static_cast<AttachmentListItem *>( currentItem() );
    if ( item ) {
      urls.append( item->uri() );
      labels.append( KURL::encode_string( item->label() ) );
    }
  }

  TQMap<TQString, TQString> metadata;
  metadata["labels"] = labels.join( ":" );

  KURLDrag *drag = new KURLDrag( urls, metadata );
  return drag;
}

AttachmentIconView::~AttachmentIconView()
{
  for ( std::set<KTempDir*>::iterator it = mTempDirs.begin() ; it != mTempDirs.end() ; ++it ) {
    delete *it;
  }
}

TQDragObject * AttachmentIconView::dragObject()
{
  KURL::List urls;
  for ( TQIconViewItem *it = firstItem( ); it; it = it->nextItem( ) ) {
    if ( !it->isSelected() ) continue;
    AttachmentListItem * item = dynamic_cast<AttachmentListItem*>( it );
    if ( !item ) return 0;
    KCal::Attachment * att = item->attachment();
    assert( att );
    KURL url;
    if ( att->isUri() ) {
      url.setPath( att->uri() );
    } else {
      KTempDir *tempDir = new KTempDir(); // will be deleted on editor close
      tempDir->setAutoDelete( true );
      mTempDirs.insert( tempDir );
      TQByteArray encoded;
      encoded.duplicate( att->data(), strlen( att->data() ) );
      TQByteArray decoded;
      KCodecs::base64Decode( encoded, decoded );
      const TQString fileName = tempDir->name( ) + '/' + att->label();
      KPIM::kByteArrayToFile( decoded, fileName, false, false, false );
      url.setPath( fileName );
    }
    urls << url;
  }
  KURLDrag *drag  = new KURLDrag( urls, this );
  return drag;
}

void AttachmentIconView::handleDrop( TQDropEvent *event, const TQValueList<TQIconDragItem> & list )
{
  Q_UNUSED( list );
  mParent->handlePasteOrDrop( event );
}


void AttachmentIconView::dragMoveEvent( TQDragMoveEvent *event )
{
  mParent->dragMoveEvent( event );
}

void AttachmentIconView::contentsDragMoveEvent( TQDragMoveEvent *event )
{
  mParent->dragMoveEvent( event );
}

void AttachmentIconView::contentsDragEnterEvent( TQDragEnterEvent *event )
{
  mParent->dragMoveEvent( event );
}

void AttachmentIconView::dragEnterEvent( TQDragEnterEvent *event )
{
  mParent->dragEnterEvent( event );
}

KOEditorAttachments::KOEditorAttachments( int spacing, TQWidget *parent,
                                          const char *name )
  : TQWidget( parent, name )
{
  TQBoxLayout *topLayout = new TQHBoxLayout( this );
  topLayout->setSpacing( spacing );

  TQLabel *label = new TQLabel( i18n("Attachments:"), this );
  topLayout->addWidget( label );

  mAttachments = new AttachmentIconView( this );
  TQWhatsThis::add( mAttachments,
                   i18n("Displays a list of current items (files, mail, etc.) "
                        "that have been associated with this event or to-do. ") );
  topLayout->addWidget( mAttachments );
  connect( mAttachments, TQT_SIGNAL( doubleClicked( TQIconViewItem * ) ),
           TQT_SLOT( showAttachment( TQIconViewItem * ) ) );
  connect( mAttachments, TQT_SIGNAL(selectionChanged()),
           TQT_SLOT(selectionChanged()) );
  connect( mAttachments, TQT_SIGNAL(contextMenuRequested(TQIconViewItem*,const TQPoint&)),
           TQT_SLOT(contextMenu(TQIconViewItem*,const TQPoint&)) );

    TQPushButton *addButton = new TQPushButton( this );
  addButton->setIconSet( SmallIconSet( "add" ) );
  TQToolTip::add( addButton, i18n( "Add an attachment" ) );
  TQWhatsThis::add( addButton,
                   i18n( "Shows a dialog used to select an attachment "
                         "to add to this event or to-do as link or as "
                         "inline data." ) );
  topLayout->addWidget( addButton );
  connect( addButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotAdd()) );

  mRemoveBtn = new TQPushButton( this );
  mRemoveBtn->setIconSet( SmallIconSet( "remove" ) );
  TQToolTip::add( mRemoveBtn, i18n("&Remove") );
  TQWhatsThis::add( mRemoveBtn,
                   i18n("Removes the attachment selected in the list above "
                        "from this event or to-do.") );
  topLayout->addWidget( mRemoveBtn );
  connect( mRemoveBtn, TQT_SIGNAL(clicked()), TQT_SLOT(slotRemove()) );

  mContextMenu = new KPopupMenu( this );

  KActionCollection* ac = new KActionCollection( this, this );

  mOpenAction = new KAction( i18n("Open"), 0, this, TQT_SLOT(slotShow()), ac );
  mOpenAction->plug( mContextMenu );

  mSaveAsAction = new KAction( i18n( "Save As..." ), 0, this, TQT_SLOT(slotSaveAs()), ac );
  mSaveAsAction->plug( mContextMenu );
  mContextMenu->insertSeparator();

  mCopyAction = KStdAction::copy(this, TQT_SLOT(slotCopy()), ac );
  mCopyAction->plug( mContextMenu );
  mCutAction = KStdAction::cut(this, TQT_SLOT(slotCut()), ac );
  mCutAction->plug( mContextMenu );
  KAction *action = KStdAction::paste(this, TQT_SLOT(slotPaste()), ac );
  action->plug( mContextMenu );
  mContextMenu->insertSeparator();

  mDeleteAction = new KAction( i18n( "&Remove" ), 0, this, TQT_SLOT(slotRemove()),  ac );
  mDeleteAction->plug( mContextMenu );
  mDeleteAction->setShortcut( Key_Delete );
  mContextMenu->insertSeparator();

  mEditAction = new KAction( i18n( "&Properties..." ), 0, this, TQT_SLOT(slotEdit()), ac );
  mEditAction->plug( mContextMenu );

  selectionChanged();
  setAcceptDrops( true );
}

KOEditorAttachments::~KOEditorAttachments()
{
}

bool KOEditorAttachments::hasAttachments()
{
  return mAttachments->count() != 0;
}

void KOEditorAttachments::dragMoveEvent( TQDragMoveEvent *event )
{
  event->accept( KURLDrag::canDecode( event ) ||
                 TQTextDrag::canDecode( event ) ||
                 KPIM::MailListDrag::canDecode( event ) ||
                 KVCardDrag::canDecode( event ) );
}

void KOEditorAttachments::dragEnterEvent( TQDragEnterEvent* event )
{
  dragMoveEvent( event );
}

void KOEditorAttachments::handlePasteOrDrop( TQMimeSource* source )
{
  KURL::List urls;
  bool probablyWeHaveUris = false;
  bool weCanCopy = true;
  TQStringList labels;

  if ( KVCardDrag::canDecode( source ) ) {
    KABC::Addressee::List addressees;
    KVCardDrag::decode( source, addressees );
    for ( KABC::Addressee::List::ConstIterator it = addressees.constBegin();
          it != addressees.constEnd(); ++it ) {
      urls.append( KDEPIMPROTOCOL_CONTACT + ( *it ).uid() );
      // there is some weirdness about realName(), hence fromUtf8
      labels.append( TQString::fromUtf8( ( *it ).realName().latin1() ) );
    }
    probablyWeHaveUris = true;
  } else if ( KURLDrag::canDecode( source ) ) {
    TQMap<TQString,TQString> metadata;
    if ( KURLDrag::decode( source, urls, metadata ) ) {
      probablyWeHaveUris = true;
      labels = TQStringList::split( ':', metadata["labels"], FALSE );
      for ( TQStringList::Iterator it = labels.begin(); it != labels.end(); ++it ) {
        *it = KURL::decode_string( (*it).latin1() );
      }

    }
  } else if ( TQTextDrag::canDecode( source ) ) {
    TQString text;
    TQTextDrag::decode( source, text );
    TQStringList lst = TQStringList::split( '\n', text, FALSE );
    for ( TQStringList::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
      urls.append( *it );
      labels.append( TQString::null );
    }
    probablyWeHaveUris = true;
  }

  KPopupMenu menu;
  int items=0;
  if ( probablyWeHaveUris ) {
    menu.insertItem( i18n( "&Link here" ), DRAG_LINK, items++ );
    // we need to check if we can reasonably expect to copy the objects
    for ( KURL::List::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it ) {
      if ( !( weCanCopy = KProtocolInfo::supportsReading( *it ) ) ) {
        break; // either we can copy them all, or no copying at all
      }
    }
    if ( weCanCopy ) {
      menu.insertItem( SmallIcon( "editcopy" ), i18n( "&Copy Here" ), DRAG_COPY, items++ );
    }
  } else {
      menu.insertItem( SmallIcon( "editcopy" ), i18n( "&Copy Here" ), DRAG_COPY, items++ );
  }

  menu.insertSeparator();
  items++;
  menu.insertItem( SmallIcon( "cancel" ), i18n( "C&ancel" ), DRAG_CANCEL, items );
  int action = menu.exec( TQCursor::pos(), 0 );

  if ( action == DRAG_LINK ) {
    TQStringList::ConstIterator jt = labels.constBegin();
    for ( KURL::List::ConstIterator it = urls.constBegin();
          it != urls.constEnd(); ++it ) {
      TQString label = (*jt++);
      if ( mAttachments->findItem( label ) ) {
        label += '~' + randomString( 3 );
      }
      addUriAttachment( (*it).url(), TQString::null, label, true );
    }
  } else if ( action != DRAG_CANCEL ) {
    if ( probablyWeHaveUris ) {
      for ( KURL::List::ConstIterator it = urls.constBegin();
            it != urls.constEnd(); ++it ) {
        TQString label = (*it).fileName();
        if ( label.isEmpty() ) {
          label = (*it).prettyURL();
        }
        if ( mAttachments->findItem( label ) ) {
          label += '~' + randomString( 3 );
        }
        addUriAttachment( (*it).url(), TQString::null, label, true );
      }
    } else { // we take anything
      addDataAttachment( source->encodedData( source->format() ),
                         source->format(),
                         KMimeType::mimeType( source->format() )->name() );
    }
  }
}

void KOEditorAttachments::dropEvent( TQDropEvent* event )
{
    handlePasteOrDrop( event );
}

void KOEditorAttachments::showAttachment( TQIconViewItem *item )
{
  AttachmentListItem *attitem = static_cast<AttachmentListItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  KCal::AttachmentHandler::view( this, att );
}

void KOEditorAttachments::saveAttachment( TQIconViewItem *item )
{
  AttachmentListItem *attitem = static_cast<AttachmentListItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  KCal::AttachmentHandler::saveAs( this, att );
}

void KOEditorAttachments::slotAdd()
{
  AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );

  AttachmentEditDialog *dlg = new AttachmentEditDialog( item, mAttachments )
;
  if ( dlg->exec() == KDialog::Rejected ) {
    delete item;
  }
  delete dlg;
}

void KOEditorAttachments::slotAddData()
{
  KURL uri = KFileDialog::getOpenFileName( TQString(), TQString(), this, i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    TQString label = uri.fileName();
    if ( label.isEmpty() ) {
      label = uri.prettyURL();
    }
    addUriAttachment( uri.url(), TQString::null, label, true );
  }
}

void KOEditorAttachments::slotEdit()
{
  for ( TQIconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      AttachmentListItem *attitem = static_cast<AttachmentListItem*>( item );
      if ( !attitem || !attitem->attachment() ) {
        return;
      }

      AttachmentEditDialog *dialog = new AttachmentEditDialog( attitem, mAttachments );
      dialog->mInline->setEnabled( false );
      dialog->setModal( false );
      connect( dialog, TQT_SIGNAL(hidden()), dialog, TQT_SLOT(delayedDestruct()) );
      dialog->show();
    }
  }
}

void KOEditorAttachments::slotRemove()
{
  TQValueList<TQIconViewItem*> selected;
  TQStringList labels;
  for ( TQIconViewItem *it = mAttachments->firstItem( ); it; it = it->nextItem( ) ) {
    if ( !it->isSelected() ) continue;
    selected << it;

    AttachmentListItem *attitem = static_cast<AttachmentListItem*>(it);
    KCal::Attachment *att = attitem->attachment();
    labels << att->label();
  }

  if ( selected.isEmpty() ) {
    return;
  }

  TQString labelsStr = labels.join( "<br>" );

  if ( KMessageBox::questionYesNo(
         this,
         i18n( "<qt>Do you really want to remove these attachments?<p>%1</qt>" ).arg( labelsStr ),
         i18n( "Remove Attachment?" ),
         KStdGuiItem::yes(), KStdGuiItem::no(),
         "calendarRemoveAttachments" ) != KMessageBox::Yes ) {
    return;
  }

  for ( TQValueList<TQIconViewItem*>::iterator it( selected.begin() ), end( selected.end() );
        it != end ; ++it ) {
    if ( (*it)->nextItem() ) {
      (*it)->nextItem()->setSelected( true );
    } else if ( (*it)->prevItem() ) {
      (*it)->prevItem()->setSelected( true );
    }
    delete *it;
  }
  mAttachments->slotUpdate();
}

void KOEditorAttachments::slotShow()
{
  for ( TQIconViewItem *it = mAttachments->firstItem(); it; it = it->nextItem() ) {
    if ( !it->isSelected() )
      continue;
    showAttachment( it );
  }
}

void KOEditorAttachments::slotSaveAs()
{
  for ( TQIconViewItem *it = mAttachments->firstItem(); it; it = it->nextItem() ) {
    if ( !it->isSelected() )
      continue;
    saveAttachment( it );
  }
}

void KOEditorAttachments::setDefaults()
{
  mAttachments->clear();
}

TQString KOEditorAttachments::randomString(int length) const
{
   if (length <=0 ) return TQString();

   TQString str; str.setLength( length );
   int i = 0;
   while (length--)
   {
      int r=random() % 62;
      r+=48;
      if (r>57) r+=7;
      if (r>90) r+=6;
      str[i++] =  char(r);
      // so what if I work backwards?
   }
   return str;
}

void KOEditorAttachments::addUriAttachment( const TQString &uri,
                                            const TQString &mimeType,
                                            const TQString &label,
                                            bool inLine )
{
  if ( !inLine ) {
    AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );
    item->setUri( uri );
    item->setLabel( label );
    if ( mimeType.isEmpty() ) {
      if ( uri.startsWith( KDEPIMPROTOCOL_CONTACT ) ) {
        item->setMimeType( "text/directory" );
      } else if ( uri.startsWith( KDEPIMPROTOCOL_EMAIL ) ) {
        item->setMimeType( "message/rfc822" );
      } else if ( uri.startsWith( KDEPIMPROTOCOL_INCIDENCE ) ) {
        item->setMimeType( "text/calendar" );
      } else if ( uri.startsWith( KDEPIMPROTOCOL_NEWSARTICLE ) ) {
        item->setMimeType( "message/news" );
      } else {
        item->setMimeType( KMimeType::findByURL( uri )->name() );
      }
    }
  } else {
    TQString tmpFile;
    if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
      TQFile f( tmpFile );
      if ( !f.open( IO_ReadOnly ) ) {
        return;
      }
      const TQByteArray data = f.readAll();
      f.close();
      addDataAttachment( data, mimeType, label );
    }
    KIO::NetAccess::removeTempFile( tmpFile );
  }
}

void KOEditorAttachments::addDataAttachment( const TQByteArray &data,
                                             const TQString &mimeType,
                                             const TQString &label )
{
  AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );

  TQString nlabel = label;
  if ( mimeType == "message/rfc822" ) {
    // mail message. try to set the label from the mail Subject:
    KMime::Message msg;
    msg.setContent( data.data() );
    msg.parse();
    nlabel = msg.subject()->asUnicodeString();
  }

  item->setData( data );
  item->setLabel( nlabel );
  if ( mimeType.isEmpty() ) {
    item->setMimeType( KMimeType::findByContent( data )->name() );
  } else {
    item->setMimeType( mimeType );
  }
}

void KOEditorAttachments::addAttachment( KCal::Attachment *attachment )
{
  new AttachmentListItem( attachment, mAttachments );
}

void KOEditorAttachments::readIncidence( KCal::Incidence *i )
{
  mAttachments->clear();

  KCal::Attachment::List attachments = i->attachments();
  KCal::Attachment::List::ConstIterator it;
  for( it = attachments.begin(); it != attachments.end(); ++it ) {
    addAttachment( (*it) );
  }
  if ( mAttachments->count() > 0 ) {
    TQTimer::singleShot( 0, mAttachments, TQT_SLOT(arrangeItemsInGrid()) );
  }
}

void KOEditorAttachments::writeIncidence( KCal::Incidence *i )
{
  i->clearAttachments();

  TQIconViewItem *item;
  AttachmentListItem *attitem;
  for( item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    attitem = static_cast<AttachmentListItem*>(item);
    if ( attitem )
      i->addAttachment( new KCal::Attachment( *(attitem->attachment() ) ) );
  }
}


void KOEditorAttachments::slotCopy()
{
    TQApplication::clipboard()->setData( mAttachments->mimeData(), QClipboard::Clipboard );
}

void KOEditorAttachments::slotCut()
{
    slotCopy();
    slotRemove();
}

void KOEditorAttachments::slotPaste()
{
    handlePasteOrDrop( TQApplication::clipboard()->data() );
}

void KOEditorAttachments::selectionChanged()
{
  bool selected = false;
  for ( TQIconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      selected = true;
      break;
    }
  }
  mRemoveBtn->setEnabled( selected );
}

void KOEditorAttachments::contextMenu(TQIconViewItem * item, const TQPoint & pos)
{
  const bool enable = item != 0;

  int numSelected = 0;
  for ( TQIconViewItem *item = mAttachments->firstItem(); item; item = item->nextItem() ) {
    if ( item->isSelected() ) {
      numSelected++;
    }
  }

  mOpenAction->setEnabled( enable );
  //TODO: support saving multiple attachments into a directory
  mSaveAsAction->setEnabled( enable && numSelected == 1 );
  mCopyAction->setEnabled( enable && numSelected == 1 );
  mCutAction->setEnabled( enable && numSelected == 1 );
  mDeleteAction->setEnabled( enable );
  mEditAction->setEnabled( enable );
  mContextMenu->exec( pos );
}

#include "koeditorattachments.moc"
