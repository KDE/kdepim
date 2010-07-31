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
                   TQWidget * parent=0, const char * name=0, WFlags f=0 );

signals:
  void dropped( const KURL::List& urls );

protected:
  virtual void contentsDragEnterEvent ( TQDragEnterEvent * );
  virtual void contentsDragMoveEvent( TQDragMoveEvent * );
  virtual void contentsDragLeaveEvent( TQDragLeaveEvent * );
  virtual void contentsDropEvent ( TQDropEvent * );

};


#endif /* CERTLISTVIEW_H */
