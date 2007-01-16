#include "certlistview.h"
#include <kurl.h>
#include <kdebug.h>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

CertKeyListView::CertKeyListView( const ColumnStrategy * strategy,
                                  const DisplayStrategy * display,
                                  QWidget * parent, Qt::WFlags f )
  : Kleo::KeyListView( strategy, display, parent, f )
{
  viewport()->setAcceptDrops( true );
}

void CertKeyListView::contentsDragEnterEvent( QDragEnterEvent * event )
{
  //const char* fmt;
  //for (int i=0; (fmt = event->format(i)); i++)
  //  kDebug() << fmt << endl;

  // We only accept URL drops. We'll check the mimetype later on.
  event->setAccepted( KUrl::List::canDecode( event->mimeData() ) );
}

void CertKeyListView::contentsDragMoveEvent( QDragMoveEvent * event )
{
  event->setAccepted( KUrl::List::canDecode( event->mimeData() ) );
}


void CertKeyListView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    // Don't let QListView do its stuff
}

void CertKeyListView::contentsDropEvent( QDropEvent * event )
{
  KUrl::List lst = KUrl::List::fromMimeData( event->mimeData() );
  if ( !lst.isEmpty() ) {
    event->accept();
    emit dropped( lst );
  }
}

#include "certlistview.moc"
