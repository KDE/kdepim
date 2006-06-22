#include "foldermodel.h"


int FolderModel::rowCount(const QModelIndex &parent) const
{
  return backend.conversationCount();
}

QVariant FolderModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() < 0 || index.row() >= backend.conversationCount())
    return QVariant();

  if (role == Qt::DisplayRole)
    return backend.conversationTitle(index.row());
  else
    return QVariant();
}

QVariant FolderModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
    return QString("Column %1").arg(section);
  else
    return QString("Row %1").arg(section);
}
