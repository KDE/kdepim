
#ifndef KSYNC_TODO_SYNCEE_H
#define KSYNC_TODO_SYNCEE_H

#include <todo.h>

#include "synctemplate.h"

namespace KSync {
    class TodoSyncEntry : public SyncEntry {
    public:
        TodoSyncEntry( KCal::Todo* = 0);
        TodoSyncEntry( const TodoSyncEntry& );
        ~TodoSyncEntry();
        KCal::Todo* todo();

        QString type() const;
        QString name();
        QString id();
        void setId(const QString& id );
        SyncEntry* clone();
        bool equals( SyncEntry* entry );
        QString timestamp();
    private:
        KCal::Todo* mTodo;
    };

    class TodoSyncee : public SyncTemplate<TodoSyncEntry> {

    public:
        TodoSyncee();
        QString type() const;
        Syncee* clone();
        QString newId()const;
    };

};

#endif
