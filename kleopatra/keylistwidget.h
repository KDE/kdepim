#ifndef __KLEOPATRA_KEYLISTWIDGET_H__
#define __KLEOPATRA_KEYLISTWIDGET_H__

#include <QWidget>

#include <utils/pimpl_ptr.h>

class KeyListWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyListWidget( QWidget * parent=0, Qt::WFlags f=0 );
    ~KeyListWidget();
    
private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

#endif // __KLEOPATRA_KEYLISTWIDGET_H__

