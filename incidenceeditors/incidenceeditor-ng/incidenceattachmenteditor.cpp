/*
    Copyright (C) 2010  Bertjan Broeksema b.broeksema@home.nl

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "incidenceattachmenteditor.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtCore/QPointer>
#include <QtGui/QClipboard>


#include <KDE/KABC/VCardDrag>
#include <KDE/KAction>
#include <KDE/KActionCollection>
#include <KDE/KFileDialog>
#include <KDE/KIO/Job>
#include <KDE/KIO/NetAccess>
#include <KDE/KMime/Message>
#include <KDE/KMenu>
#include <KDE/KMessageBox>
#include <KDE/KProtocolManager>
#include <KDE/KRun>

#include <libkdepimdbusinterfaces/urihandler.h>

#include "attachmenticonview.h"
#include "attachmenteditdialog.h"
#include "ui_incidenceattachmenteditor.h"

using namespace IncidenceEditorsNG;

IncidenceAttachmentEditor::IncidenceAttachmentEditor( QWidget *parent )
  : IncidenceEditor( parent )
  , mUi( new Ui::IncidenceAttachmentEditor )
  , mPopupMenu( new KMenu( this ) )
{
  mUi->setupUi( this );
  mUi->mAddButton->setIcon( KIcon( "list-add" ) );
  mUi->mRemoveButton->setIcon( KIcon( "list-remove" ) );

  setupActions();
  setupAttachmentIconView();
  
  connect( mUi->mAddButton, SIGNAL(clicked()), SLOT(addAttachment()) );
  connect( mUi->mRemoveButton, SIGNAL(clicked()), SLOT(removeSelectedAttachments()) );
}

void IncidenceAttachmentEditor::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;
  mAttachmentView->clear();

  KCal::Attachment::List attachments = incidence->attachments();
  KCal::Attachment::List::ConstIterator it;
  for ( it = attachments.constBegin(); it != attachments.constEnd(); ++it )
    new AttachmentIconItem( (*it), mAttachmentView );
}

void IncidenceAttachmentEditor::save( KCal::Incidence::Ptr incidence )
{
  incidence->clearAttachments();

  for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachmentView->item( itemIndex );
    AttachmentIconItem *attitem = dynamic_cast<AttachmentIconItem*>(item);
    Q_ASSERT( item );
    incidence->addAttachment( new KCal::Attachment( *( attitem->attachment() ) ) );
  }
}

bool IncidenceAttachmentEditor::isDirty() const
{
  if ( mLoadedIncidence ) {
    if ( mAttachmentView->count() != mLoadedIncidence->attachments().count() )
      return true;

    KCal::Attachment::List origAttachments = mLoadedIncidence->attachments();
    for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
      QListWidgetItem *item = mAttachmentView->item( itemIndex );
      Q_ASSERT( dynamic_cast<AttachmentIconItem*>( item ) );

      KCal::Attachment * const listAttachment
        = static_cast<AttachmentIconItem*>( item )->attachment();
      
      bool found = false;
      for ( int i = 0; i < origAttachments.size() && !found; ++i ) {
        KCal::Attachment * const attachment = origAttachments.at( i );

        // Check for changed label first
        if ( attachment->label() != listAttachment->label() )
          continue;

        if ( attachment->isBinary() && listAttachment->isBinary()
          && attachment->decodedData() == listAttachment->decodedData() ) {
          // Not sure about this. Might be too expensive.
          origAttachments.removeAt( i );
          found = true;
        } else if ( attachment->isUri() && listAttachment->isUri()
           && attachment->uri() == listAttachment->uri() ) {
          origAttachments.removeAt( i );
          found = true;
        }
      }
    }

    // All attachments are removed from the list, meaning, the items in mAttachmentView
    // are equal to the attachements set on mLoadedIncidence.
    return !origAttachments.isEmpty();

    
  } else {
    // No incidence loaded, so if the user added attachments we're dirty.
    return mAttachmentView->count() != 0;
  }
  
  return false;
}


/// Private slots

void IncidenceAttachmentEditor::addAttachment()
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachmentView );

  QPointer<AttachmentEditDialog> dlg = new AttachmentEditDialog( item, mAttachmentView );
  dlg->setCaption( i18nc( "@title", "Add Attachment" ) );
  if ( dlg->exec() == KDialog::Rejected )
    delete item;

  delete dlg;
}

void IncidenceAttachmentEditor::copyToClipboard()
{
  QApplication::clipboard()->setMimeData( mAttachmentView->mimeData(), QClipboard::Clipboard );
}

void IncidenceAttachmentEditor::openURL( const KUrl &url )
{
  QString uri = url.url();
  UriHandler::process( uri );
}

void IncidenceAttachmentEditor::pasteFromClipboard()
{
  handlePasteOrDrop( QApplication::clipboard()->mimeData() );
}

void IncidenceAttachmentEditor::removeSelectedAttachments()
{
  QList<QListWidgetItem *> selected;
  QStringList labels;

  for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
    QListWidgetItem *it = mAttachmentView->item( itemIndex );
    if ( it->isSelected() ) {
      AttachmentIconItem *attitem = static_cast<AttachmentIconItem *>( it );
      if ( attitem ) {
        KCal::Attachment *att = attitem->attachment();
        labels << att->label();
        selected << it;
      }
    }
  }

  if ( selected.isEmpty() )
    return;

  QString labelsStr = labels.join( "<br>" );

  if ( KMessageBox::questionYesNo(
         this,
         i18nc( "@info",
                "Do you really want to remove these attachments?<nl>%1</nl>", labelsStr ),
         i18nc( "@title:window", "Remove Attachments?" ),
         KStandardGuiItem::yes(), KStandardGuiItem::no(),
         "calendarRemoveAttachments" ) != KMessageBox::Yes ) {
    return;
  }

  for ( QList<QListWidgetItem *>::iterator it( selected.begin() ), end( selected.end() );
        it != end ; ++it ) {
    int row = mAttachmentView->row( *it );
    QListWidgetItem *next = mAttachmentView->item( ++row );
    QListWidgetItem *prev = mAttachmentView->item( --row );
    if ( next ) {
      next->setSelected( true );
    } else if ( prev ) {
      prev->setSelected( true );
    }
    delete *it;
  }

  mAttachmentView->update();
  checkDirtyStatus();
}

void IncidenceAttachmentEditor::saveAttachment( QListWidgetItem *item )
{
  Q_ASSERT( item );
  Q_ASSERT( dynamic_cast<AttachmentIconItem*>( item ) );

  AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
  if ( !attitem->attachment() )
    return;

  KCal::Attachment *att = attitem->attachment();

  // get the saveas file name
  QString saveAsFile =  KFileDialog::getSaveFileName(
    att->label(),
    QString(), 0,
    i18nc( "@title", "Save Attachment" ) );

  if ( saveAsFile.isEmpty() ||
       ( QFile( saveAsFile ).exists() &&
         ( KMessageBox::warningYesNo(
           0,
           i18nc( "@info", "%1 already exists. Do you want to overwrite it?",
                  saveAsFile ) ) == KMessageBox::No ) ) ) {
    return;
  }

  KUrl sourceUrl;
  if ( att->isUri() ) {
    sourceUrl = att->uri();
  } else {
    sourceUrl = mAttachmentView->tempFileForAttachment( att );
  }
  // save the attachment url
  if ( !KIO::NetAccess::file_copy( sourceUrl, KUrl( saveAsFile ) ) &&
       KIO::NetAccess::lastError() ) {
    KMessageBox::error( this, KIO::NetAccess::lastErrorString() );
  }
}

void IncidenceAttachmentEditor::saveSelectedAttachments()
{
  for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachmentView->item( itemIndex );
    if ( item->isSelected() )
      saveAttachment( item );
  }
}

void IncidenceAttachmentEditor::showAttachment( QListWidgetItem *item )
{
  Q_ASSERT( item );
  Q_ASSERT( dynamic_cast<AttachmentIconItem*>( item ) );
  AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
  if ( !attitem->attachment() )
    return;

  KCal::Attachment *att = attitem->attachment();
  if ( att->isUri() ) {
    emit openURL( att->uri() );
  } else {
    KRun::runUrl( mAttachmentView->tempFileForAttachment( att ), att->mimeType(), 0, true );
  }
}

void IncidenceAttachmentEditor::showContextMenu( const QPoint &pos )
{
  QListWidgetItem *item = mAttachmentView->itemAt( pos );
  const bool enable = item != 0;

  int numSelected = 0;
  for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachmentView->item( itemIndex );
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
  mPopupMenu->exec( mAttachmentView->mapToGlobal( pos ) );
}

void IncidenceAttachmentEditor::showSelectedAttachments()
{
  for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachmentView->item( itemIndex );
    if ( item->isSelected() )
      showAttachment( item );
  }
}

void IncidenceAttachmentEditor::cutToClipboard()
{
  copyToClipboard();
  removeSelectedAttachments();
}

void IncidenceAttachmentEditor::editSelectedAttachments()
{
  for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachmentView->item( itemIndex );
    if ( item->isSelected() ) {
      Q_ASSERT( dynamic_cast<AttachmentIconItem*>( item ) );

      AttachmentIconItem *attitem = static_cast<AttachmentIconItem*>( item );
      if ( !attitem->attachment() )
        return;

      AttachmentEditDialog *dialog = new AttachmentEditDialog( attitem, mAttachmentView, false );
      dialog->setModal( false );
      connect( dialog, SIGNAL(hidden()), dialog, SLOT(delayedDestruct()) );
      dialog->show();
    }
  }
}

void IncidenceAttachmentEditor::slotItemRenamed ( QListWidgetItem *item )
{
  Q_ASSERT( item );
  Q_ASSERT( dynamic_cast<AttachmentIconItem *>( item ) );
  static_cast<AttachmentIconItem *>( item )->setLabel( item->text() );
  checkDirtyStatus();
}

void IncidenceAttachmentEditor::slotSelectionChanged()
{
  bool selected = false;
  for ( int itemIndex = 0; itemIndex < mAttachmentView->count(); ++itemIndex ) {
    QListWidgetItem *item = mAttachmentView->item( itemIndex );
    if ( item->isSelected() ) {
      selected = true;
      break;
    }
  }
  mUi->mRemoveButton->setEnabled( selected );
}


/// Private functions

void IncidenceAttachmentEditor::handlePasteOrDrop( const QMimeData *mimeData )
{
  KUrl::List urls;
  bool probablyWeHaveUris = false;
  bool weCanCopy = true;
  QStringList labels;

  if ( KABC::VCardDrag::canDecode( mimeData ) ) {
    KABC::Addressee::List addressees;
    KABC::VCardDrag::fromMimeData( mimeData, addressees );
    for ( KABC::Addressee::List::ConstIterator it = addressees.constBegin();
          it != addressees.constEnd(); ++it ) {
      urls.append( QLatin1String( "uid:" ) + ( *it ).uid() );
      // there is some weirdness about realName(), hence fromUtf8
      labels.append( QString::fromUtf8( ( *it ).realName().toLatin1() ) );
    }
    probablyWeHaveUris = true;
  } else if ( KUrl::List::canDecode( mimeData ) ) {
    QMap<QString,QString> metadata;

    urls = KUrl::List::fromMimeData( mimeData, &metadata );
    probablyWeHaveUris = true;
    labels = metadata["labels"].split( ':', QString::SkipEmptyParts );
    for ( QStringList::Iterator it = labels.begin(); it != labels.end(); ++it ) {
      *it = KUrl::fromPercentEncoding( (*it).toLatin1() );
    }
  } else if ( mimeData->hasText() ) {
    QString text = mimeData->text();
    QStringList lst = text.split( '\n', QString::SkipEmptyParts );
    for ( QStringList::ConstIterator it = lst.constBegin(); it != lst.constEnd(); ++it ) {
      urls.append( *it );
    }
    probablyWeHaveUris = true;
  }
  KMenu menu( this );
  QAction *linkAction = 0, *cancelAction;
  if ( probablyWeHaveUris ) {
    linkAction = menu.addAction( KIcon( "insert-link" ), i18nc( "@action:inmenu", "&Link here" ) );
    // we need to check if we can reasonably expect to copy the objects
    for ( KUrl::List::ConstIterator it = urls.constBegin(); it != urls.constEnd(); ++it ) {
      if ( !( weCanCopy = KProtocolManager::supportsReading( *it ) ) ) {
        break; // either we can copy them all, or no copying at all
      }
    }
    if ( weCanCopy ) {
      menu.addAction( KIcon( "edit-copy" ), i18nc( "@action:inmenu", "&Copy here" ) );
    }
  } else {
    menu.addAction( KIcon( "edit-copy" ), i18nc( "@action:inmenu", "&Copy here" ) );
  }

  menu.addSeparator();
  cancelAction = menu.addAction( KIcon( "process-stop" ) , i18nc( "@action:inmenu", "C&ancel" ) );

  QByteArray data;
  QString mimeType;
  QString label;

  if(!mimeData->formats().isEmpty() && !probablyWeHaveUris) {
    data=mimeData->data( mimeData->formats().first() );
    mimeType = mimeData->formats().first();
    if( KMimeType::mimeType( mimeData->formats().first() ) )
       label = KMimeType::mimeType( mimeData->formats().first() )->name();

  }

  QAction *ret = menu.exec( QCursor::pos() );
  if ( linkAction == ret ) {
    QStringList::ConstIterator jt = labels.constBegin();
    for ( KUrl::List::ConstIterator it = urls.constBegin();
          it != urls.constEnd(); ++it ) {
      addUriAttachment( (*it).url(), QString(), ( jt == labels.constEnd() ?
                                                  QString() : *( jt++ ) ), true );
    }
  } else if ( cancelAction != ret ) {
    if ( probablyWeHaveUris ) {
      for ( KUrl::List::ConstIterator it = urls.constBegin();
            it != urls.constEnd(); ++it ) {
        KIO::Job *job = KIO::storedGet( *it );
        connect( job, SIGNAL(result(KJob *)), SLOT(downloadComplete(KJob *)) );
      }
    } else { // we take anything
      addDataAttachment( data, mimeType, label );
    }
  }
}

void IncidenceAttachmentEditor::setupActions()
{
  KActionCollection *ac = new KActionCollection( this );
  ac->addAssociatedWidget( this );
  
  mOpenAction = new KAction( i18nc( "@action:inmenu open the attachment in a viewer",
                                    "&Open" ), this );
  connect( mOpenAction, SIGNAL(triggered(bool)), SLOT(showSelectedAttachments()) );
  ac->addAction( "view", mOpenAction );
  mPopupMenu->addAction( mOpenAction );

  mSaveAsAction = new KAction( i18nc( "@action:inmenu save the attachment to a file",
                                      "Save As..." ), this );
  connect( mSaveAsAction, SIGNAL(triggered(bool)), SLOT(saveSelectedAttachments()) );
  mPopupMenu->addAction( mSaveAsAction );
  mPopupMenu->addSeparator();

  mCopyAction = KStandardAction::copy( this, SLOT(copyToClipboard()), ac );
  mPopupMenu->addAction( mCopyAction );
  
  mCutAction = KStandardAction::cut( this, SLOT(cutToClipboard()), ac );
  mPopupMenu->addAction( mCutAction );
  
  KAction *action = KStandardAction::paste( this, SLOT(pasteFromClipboard()), ac );
  mPopupMenu->addAction( action );
  mPopupMenu->addSeparator();

  mDeleteAction = new KAction( i18nc( "@action:inmenu remove the attachment",
                                      "&Remove" ), this );
  connect( mDeleteAction, SIGNAL(triggered(bool)), SLOT(removeSelectedAttachments()) );
  ac->addAction( "remove", mDeleteAction );
  mDeleteAction->setShortcut( Qt::Key_Delete );
  mPopupMenu->addAction( mDeleteAction );
  mPopupMenu->addSeparator();

  mEditAction = new KAction( i18nc( "@action:inmenu show a dialog used to edit the attachment",
                                    "&Properties..." ), this );
  connect( mEditAction, SIGNAL(triggered(bool)), SLOT(editSelectedAttachments()) );
  ac->addAction( "edit", mEditAction );
  mPopupMenu->addAction( mEditAction );
}

void IncidenceAttachmentEditor::setupAttachmentIconView()
{
  mAttachmentView = new AttachmentIconView( this );
  mAttachmentView->setWhatsThis( i18nc( "@info:whatsthis",
                                     "Displays items (files, mail, etc.) that "
                                     "have been associated with this event or to-do." ) );

  connect( mAttachmentView, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
           SLOT(showAttachment(QListWidgetItem*)) );
  connect( mAttachmentView, SIGNAL(itemChanged(QListWidgetItem*)),
           SLOT(slotItemRenamed(QListWidgetItem*)) );
  connect( mAttachmentView, SIGNAL(itemSelectionChanged()),
           SLOT(slotSelectionChanged()) );
  connect( mAttachmentView, SIGNAL(customContextMenuRequested(QPoint)),
           SLOT(showContextMenu(QPoint)) );

                                     
  QGridLayout *layout = new QGridLayout( mUi->mAttachmentViewPlaceHolder );
  layout->addWidget( mAttachmentView );
}

// void IncidenceAttachmentEditor::addAttachment( KCal::Attachment *attachment )
// {
//   new AttachmentIconItem( attachment, mAttachmentView );
// }

void IncidenceAttachmentEditor::addDataAttachment( const QByteArray &data,
                                                   const QString &mimeType,
                                                   const QString &label )
{
  AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachmentView );

  QString nlabel = label;
  if ( mimeType == "message/rfc822" ) {
    // mail message. try to set the label from the mail Subject:
    KMime::Message msg;
    msg.setContent( data );
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

  checkDirtyStatus();
}

void IncidenceAttachmentEditor::addUriAttachment( const QString &uri,
                                                  const QString &mimeType,
                                                  const QString &label,
                                                  bool inLine )
{
  if ( !inLine ) {
    AttachmentIconItem *item = new AttachmentIconItem( 0, mAttachmentView );
    item->setUri( uri );
    item->setLabel( label );
    if ( mimeType.isEmpty() ) {
      if ( uri.startsWith( QLatin1String( "uid:" ) ) ) {
        item->setMimeType( "text/directory" );
      } else if ( uri.startsWith( QLatin1String( "kmail:" ) ) ) {
        item->setMimeType( "message/rfc822" );
      } else if ( uri.startsWith( QLatin1String( "urn:x-ical" ) ) ) {
        item->setMimeType( "text/calendar" );
      } else if ( uri.startsWith( QLatin1String( "news:" ) ) ) {
        item->setMimeType( "message/news" );
      } else {
        item->setMimeType( KMimeType::findByUrl( uri )->name() );
      }
    }
  } else {
    QString tmpFile;
    if ( KIO::NetAccess::download( uri, tmpFile, this ) ) {
      QFile f( tmpFile );
      if ( !f.open( QIODevice::ReadOnly ) ) {
        return;
      }
      const QByteArray data = f.readAll();
      f.close();
      addDataAttachment( data, mimeType, label );
    }
    KIO::NetAccess::removeTempFile( tmpFile );
  }
}
