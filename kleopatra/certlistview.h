#ifndef CERTLISTVIEW_H
#define CERTLISTVIEW_H

#include <libkleoui/keylistview.h>
#include <kurl.h>
//Added by qt3to4:
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

/// We need to derive from Kleo::KeyListView simply to add support for drop events
class CertKeyListView : public Kleo::KeyListView {
  Q_OBJECT

public:
  CertKeyListView( const ColumnStrategy * strategy,
                   const DisplayStrategy * display=0,
                   QWidget * parent=0, Qt::WFlags f=0 );

signals:
  void dropped( const KUrl::List& urls );

protected:
  virtual void contentsDragEnterEvent ( QDragEnterEvent * );
  virtual void contentsDragMoveEvent( QDragMoveEvent * );
  virtual void contentsDragLeaveEvent( QDragLeaveEvent * );
  virtual void contentsDropEvent ( QDropEvent * );

};


#endif /* CERTLISTVIEW_H */
