#ifndef __KLEOPATRA_SEARCHBARSTATEHANDLER_H__
#define __KLEOPATRA_SEARCHBARSTATEHANDLER_H__

#include <QObject>

#include "utils/pimpl_ptr.h"

class QAbstractItemView;

class SearchBar;
class TabWidget;

class SearchBarStateHandler : public QObject {
    Q_OBJECT
        public:
    explicit SearchBarStateHandler( TabWidget* tabWidget, SearchBar* bar, QObject * parent=0 );
    ~SearchBarStateHandler();
    
private:
    class Private;
    kdtools::pimpl_ptr<Private> d;


    Q_PRIVATE_SLOT( d, void currentViewChanged( QAbstractItemView* ) );
    Q_PRIVATE_SLOT( d, void viewDestroyed( QObject* ) );
};

#endif /* __KLEOPATRA_SEARCHBARSTATEHANDLER_H__ */

