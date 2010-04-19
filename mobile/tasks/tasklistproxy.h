#ifndef TASKLISTPROXY_H
#define TASKLISTPROXY_H

#include "listproxy.h"

class TaskListProxy : public ListProxy
{
public:
    explicit TaskListProxy( QObject* parent = 0 );

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
};

#endif // TASKLISTPROXY_H
