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
    KCal::Todo* todo = entry->incidence();

    str += todo->categories().join(";");
    str += QString::number( todo->isCompleted() );
    str += QString::number( todo->percentComplete() );
    str += todo->summary();
    if ( todo->hasDueDate() ) {
        str += todo->dtDue().toString("dd.MM.yyyy");
    }
    str += QString::number( todo->priority() );
    str += todo->description();

    return str;
}
