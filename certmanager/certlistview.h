#ifndef CERTLISTVIEW_H
#define CERTLISTVIEW_H

#include <ui/keylistview.h>
#include <kurl.h>

/// We need to derive from Kleo::KeyListView simply to add support for drop events
class CertKeyListView : public Kleo::KeyListView {
  Q_OBJECT

public:
  CertKeyListView( const ColumnStrategy * strategy,
                   const DisplayStrategy * display=0,
                   QWidget * parent=0, const char * name=0, WFlags f=0 );

signals:
  void dropped( const KURL::List& urls );

protected:
  virtual void contentsDragEnterEvent ( QDragEnterEvent * );
  virtual void contentsDragMoveEvent( QDragMoveEvent * );
  virtual void contentsDragLeaveEvent( QDragLeaveEvent * );
  virtual void contentsDropEvent ( QDropEvent * );

};


#endif /* CERTLISTVIEW_H */
