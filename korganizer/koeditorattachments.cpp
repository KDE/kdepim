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

#include <libkcal/incidence.h>
#include <libkdepim/kpimurlrequesterdlg.h>
#include <libkdepim/kfileio.h>

#include <klocale.h>
#include <kdebug.h>
#include <kmdcodec.h>
#include <kmessagebox.h>
#include <kiconview.h>
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
#include <set>

class AttachmentListItem : public KIconViewItem
{
  public:
    AttachmentListItem( KCal::Attachment*att, TQIconView *parent ) :
        KIconViewItem( parent )
    {
      if ( att ) {
        mAttachment = new KCal::Attachment( *att );
      } else {
        mAttachment = new KCal::Attachment( TQString::null );
      }
      readAttachment();
      setDragEnabled( true );
    }
    ~AttachmentListItem() { delete mAttachment; }
    KCal::Attachment *attachment() const { return mAttachment; }

    void setUri( const TQString &uri )
    {
      mAttachment->setUri( uri );
      readAttachment();
    }
    void setData( const char *base64 )
    {
      mAttachment->setData( base64 );
      readAttachment();
    }
    void setMimeType( const TQString &mime )
    {
      mAttachment->setMimeType( mime );
      readAttachment();
    }
    void setLabel( const TQString &label )
    {
      mAttachment->setLabel( label );
      readAttachment();
    }

    void readAttachment()
    {
      if ( mAttachment->isUri() )
        setText( mAttachment->uri() );
      else {
        if ( mAttachment->label().isEmpty() )
          setText( i18n("[Binary data]") );
        else
          setText( mAttachment->label() );
      }
      KMimeType::Ptr mt = KMimeType::mimeType( mAttachment->mimeType() );
      if ( mt ) {
          const TQString iconName( mt->icon( TQString(), false ) );
          TQPixmap pix = KGlobal::iconLoader( )->loadIcon( iconName, KIcon::Small );
          if ( pix.isNull() )
            pix = KGlobal::iconLoader( )->loadIcon( "unknown", KIcon::Small );
            if ( !pix.isNull() )
              setPixmap( pix );
      }
    }

  private:
    KCal::Attachment *mAttachment;
};

class AttachmentIconView : public KIconView
{
    friend class KOEditorAttachments;
    public:
        AttachmentIconView( KOEditorAttachments* parent=0 )
            :KIconView( parent ),
             mParent( parent )
        {
            setAcceptDrops( true );
            setSelectionMode( TQIconView::Extended );
            setMode( KIconView::Select );
            setItemTextPos( TQIconView::Right );
            setArrangement( TQIconView::LeftToRight );
            setMaxItemWidth( QMAX(maxItemWidth(), 250) );
            setMinimumHeight( QMAX(fontMetrics().height(), 16) + 12 );
        }
        ~AttachmentIconView()
        {
            for ( std::set<KTempDir*>::iterator it = mTempDirs.begin() ; it != mTempDirs.end() ; ++it ) {
                delete *it;
            }
        }
    protected:
        TQDragObject * dragObject()
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
                    KTempDir * tempDir = new KTempDir(); // will be deleted on editor close
                    tempDir->setAutoDelete( true );
                    mTempDirs.insert( tempDir );
                    TQByteArray encoded;
                    encoded.duplicate( att->data(), strlen(att->data()) );
                    TQByteArray decoded;
                    KCodecs::base64Decode( encoded, decoded );
                    const TQString fileName = tempDir->name( ) + "/" + att->label();
                    KPIM::kByteArrayToFile( decoded, fileName, false, false, false );
                    url.setPath( fileName );
                }
                urls << url;
            }
            KURLDrag *drag  = new KURLDrag( urls, this );
            return drag;
        }
        void contentsDropEvent( TQDropEvent* event )
        {
          mParent->handlePasteOrDrop( event );
        }
    private:
        std::set<KTempDir*> mTempDirs;
        KOEditorAttachments* mParent;
};

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

  mAddMenu = new KPopupMenu( this );
  mContextMenu = new KPopupMenu( this );

  KActionCollection* ac = new KActionCollection( this, this );

  mOpenAction = new KAction( i18n("View"), 0, this, TQT_SLOT(slotShow()), ac );
  mOpenAction->plug( mContextMenu );
  mContextMenu->insertSeparator();

  mCopyAction = KStdAction::copy(this, TQT_SLOT(slotCopy( ) ), ac );
  mCopyAction->plug( mContextMenu );
  mCutAction = KStdAction::cut(this, TQT_SLOT(slotCut( ) ), ac );
  mCutAction->plug( mContextMenu );
  KAction *action = KStdAction::paste(this, TQT_SLOT(slotPaste( ) ), ac );
  action->plug( mContextMenu );

  action = new KAction( i18n("&Attach File..."), 0, this, TQT_SLOT(slotAddData()), ac );
  action->setWhatsThis( i18n("Shows a dialog used to select an attachment "
                        "to add to this event or to-do as link as inline data.") );
  action->plug( mAddMenu );
  action = new KAction( i18n("Attach &Link..."), 0, this, TQT_SLOT(slotAdd()), ac );
  action->setWhatsThis( i18n("Shows a dialog used to select an attachment "
                        "to add to this event or to-do as link.") );
  action->plug( mAddMenu );

  TQPushButton *addButton = new TQPushButton( this );
  addButton->setIconSet( SmallIconSet( "add" ) );
  addButton->setPopup( mAddMenu );
  topLayout->addWidget( addButton );

  mRemoveBtn = new TQPushButton( this );
  mRemoveBtn->setIconSet( SmallIconSet( "remove" ) );
  TQToolTip::add( mRemoveBtn, i18n("&Remove") );
  TQWhatsThis::add( mRemoveBtn,
                   i18n("Removes the attachment selected in the list above "
                        "from this event or to-do.") );
  topLayout->addWidget( mRemoveBtn );
  connect( mRemoveBtn, TQT_SIGNAL( clicked() ), TQT_SLOT( slotRemove() ) );

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

void KOEditorAttachments::dragEnterEvent( TQDragEnterEvent* event )
{
  event->accept( KURLDrag::canDecode( event ) | TQTextDrag::canDecode( event ) );
}

void KOEditorAttachments::handlePasteOrDrop( TQMimeSource* source )
{
  KURL::List urls;
  TQString text;
  if ( KURLDrag::decode( source, urls ) ) {
    const bool asUri = KMessageBox::questionYesNo( this,
            i18n("Do you want to link to the attachments, or include them in the event?"),
            i18n("Attach as link?"), i18n("As Link"), i18n("As File") ) == KMessageBox::Yes;
    for ( KURL::List::ConstIterator it = urls.begin(); it != urls.end(); ++it ) {
      addAttachment( (*it).url(), TQString::null, asUri );
    }
  } else if ( TQTextDrag::decode( source, text ) ) {
    TQStringList lst = TQStringList::split( '\n', text );
    for ( TQStringList::ConstIterator it = lst.begin(); it != lst.end(); ++it ) {
      addAttachment( (*it)  );
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
  if ( att->isUri() ) {
    emit openURL( att->uri() );
  } else {
    KTempFile f;
    if ( !f.file() )
      return;
    TQByteArray encoded;
    encoded.duplicate( att->data(), strlen(att->data()) );
    TQByteArray decoded;
    KCodecs::base64Decode( encoded, decoded );
    f.file()->writeBlock( decoded );
    f.file()->close();
    KRun::runURL( f.name(), att->mimeType(), true, false );
  }
}

void KOEditorAttachments::slotAdd()
{
  KURL uri = KPimURLRequesterDlg::getURL( TQString::null, i18n(
         "URL (e.g. a web page) or file to be attached (only "
         "the link will be attached, not the file itself):"), this,
                                       i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    addAttachment( uri );
  }
}

void KOEditorAttachments::slotAddData()
{
  KURL uri = KFileDialog::getOpenFileName( TQString(), TQString(), this, i18n("Add Attachment") );
  if ( !uri.isEmpty() ) {
    addAttachment( uri, TQString::null, false );
  }
}

void KOEditorAttachments::slotEdit()
{
  TQIconViewItem *item = mAttachments->currentItem();
  AttachmentListItem *attitem = static_cast<AttachmentListItem*>(item);
  if ( !attitem || !attitem->attachment() ) return;

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    KURL uri = KPimURLRequesterDlg::getURL( att->uri(), i18n(
         "URL (e.g. a web page) or file to be attached (only "
         "the link will be attached, not the file itself):"), this,
                                         i18n("Edit Attachment") );

    if ( !uri.isEmpty() )
      attitem->setUri( uri.url() );
  } else {
    KURL uri = KPimURLRequesterDlg::getURL( TQString::null, i18n(
         "File to be attached:"), this, i18n("Add Attachment") );
    if ( !uri.isEmpty() ) {
          TQString tmpFile;
      if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
        TQFile f( tmpFile );
        if ( !f.open( IO_ReadOnly ) )
          return;
        TQByteArray data = f.readAll();
        f.close();
        attitem->setData( KCodecs::base64Encode( data ) );
        attitem->setMimeType( KIO::NetAccess::mimetype( uri, this ) );
        TQString label = uri.fileName();
        if ( label.isEmpty() )
          label = uri.prettyURL();
        attitem->setLabel( label );
        KIO::NetAccess::removeTempFile( tmpFile );
      }
    }
  }
}

void KOEditorAttachments::slotRemove()
{
    TQValueList<TQIconViewItem*> selected;
    for ( TQIconViewItem *it = mAttachments->firstItem( ); it; it = it->nextItem( ) ) {
        if ( !it->isSelected() ) continue;
        selected << it;
    }
    if ( selected.isEmpty() || KMessageBox::warningContinueCancel(this,
                    selected.count() == 1?i18n("This item will be permanently deleted."):
                    i18n("The selected items will be permanently deleted."),
                    i18n("KOrganizer Confirmation"),KStdGuiItem::del()) != KMessageBox::Continue )
        return;

    for ( TQValueList<TQIconViewItem*>::iterator it( selected.begin() ), end( selected.end() ); it != end ; ++it ) {
        delete *it;
    }
}

void KOEditorAttachments::slotShow()
{
  for ( TQIconViewItem *it = mAttachments->firstItem(); it; it = it->nextItem() ) {
    if ( !it->isSelected() )
      continue;
    showAttachment( it );
  }
}

void KOEditorAttachments::setDefaults()
{
  mAttachments->clear();
}

void KOEditorAttachments::addAttachment( const KURL &uri,
                                         const TQString &mimeType, bool asUri )
{
  AttachmentListItem *item = new AttachmentListItem( 0, mAttachments );
  if ( asUri ) {
    item->setUri( uri.url() );
    if ( !mimeType.isEmpty() ) item->setMimeType( mimeType );
  } else {
    TQString tmpFile;
    if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
      TQFile f( tmpFile );
      if ( !f.open( IO_ReadOnly ) )
        return;
      TQByteArray data = f.readAll();
      f.close();
      item->setData( KCodecs::base64Encode( data ) );
      if ( !mimeType.isEmpty() )
        item->setMimeType( mimeType );
      else
        item->setMimeType( KIO::NetAccess::mimetype( uri, this ) );
      TQString label = uri.fileName();
      if ( label.isEmpty() )
        label = uri.prettyURL();
      item->setLabel( label );
      KIO::NetAccess::removeTempFile( tmpFile );
    }
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
    TQApplication::clipboard()->setData( mAttachments->dragObject(), QClipboard::Clipboard );
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
  mOpenAction->setEnabled( enable );
  mCopyAction->setEnabled( enable );
  mCutAction->setEnabled( enable );
  mContextMenu->exec( pos );
}

#include "koeditorattachments.moc"
