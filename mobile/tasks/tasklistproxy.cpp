#include "tasklistproxy.h"

TaskListProxy::TaskListProxy( QObject* parent ) : ListProxy( parent )
{ }

QVariant TaskListProxy::data( const QModelIndex& index, int role ) const
{
  // TODO: Implement
  return QVariant();
}
