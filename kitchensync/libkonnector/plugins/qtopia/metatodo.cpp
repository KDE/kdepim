#include <libkcal/todo.h>

#include "metatodo.h"

using namespace OpieHelper;

MetaTodo::MetaTodo()
    : MD5Template<KSync::TodoSyncee, KSync::TodoSyncEntry>() {
}
MetaTodo::~MetaTodo() {
}
QString MetaTodo::string( KSync::TodoSyncEntry* entry ) {
    QString str;
    KCal::Todo* todo = entry->todo();

    str += todo->categories().join(";");
    str += QString::number( todo->isCompleted() );
    str += QString::number( todo->percentComplete() );
    str += todo->summary();
    if ( todo->hasDueDate() ) {
        str += todo->dtDue().toString("dd.MM.yyyy");
    }
    str += QString::number( todo->priority() );
    str += todo->description();
    kdDebug(5227) << "Meta String is " << str << "Todo is " << todo->isCompleted() << QString::number( todo->isCompleted() ) << endl;
    return str;
}
