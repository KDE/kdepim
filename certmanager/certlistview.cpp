#include <config.h>
#include "certlistview.h"
#include <kurldrag.h>
#include <kdebug.h>

CertKeyListView::CertKeyListView( const ColumnStrategy * strategy,
                                  const DisplayStrategy * display,
                                  QWidget * parent, const char * name, WFlags f )
  : Kleo::KeyListView( strategy, display, parent, name, f )
{
  viewport()->setAcceptDrops( true );
}

void CertKeyListView::contentsDragEnterEvent( QDragEnterEvent * event )
{
  //const char* fmt;
  //for (int i=0; (fmt = event->format(i)); i++)
  //  kdDebug() << fmt << endl;

  // We only accept URL drops. We'll check the mimetype later on.
  event->accept( QUriDrag::canDecode( event ) );
}

void CertKeyListView::contentsDragMoveEvent( QDragMoveEvent * event )
{
  event->accept( QUriDrag::canDecode( event ) );
}


void CertKeyListView::contentsDragLeaveEvent( QDragLeaveEvent * )
{
    // Don't let QListView do its stuff
}

void CertKeyListView::contentsDropEvent( QDropEvent * event )
{
  KURL::List lst;
  if ( KURLDrag::decode( event, lst ) ) {
    event->accept();
    emit dropped( lst );
  }
}

#include "certlistview.moc"
