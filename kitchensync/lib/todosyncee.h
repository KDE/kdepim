
#ifndef KSYNC_TODO_SYNCEE_H
#define KSYNC_TODO_SYNCEE_H

#include <todo.h>

#include "synctemplate.h"

namespace KSync {
    class TodoSyncEntry : public SyncEntry {
    public:
        TodoSyncEntry( KCal::Todo* );
        TodoSyncEntry( const TodoSyncEntry& );
        ~TodoSyncEntry();
        KCal::Todo* todo();

        QString type() const;
        QString name();
        QString id();
        SyncEntry* clone();
        bool equals( SyncEntry* entry );
        QString timestamp();
    private:
        KCal::Todo* mTodo;
    };

    class TodoSyncee : public SyncTemplate<TodoSyncEntry> {

    public:
        TodoSyncee() : SyncTemplate<TodoSyncEntry>() {};
        QString type() const { return QString::fromLatin1("TodoSyncee"); }
    };

};

#endif
