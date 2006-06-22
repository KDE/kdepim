#ifndef FOLDERMODEL_H
#define FOLDERMODEL_H

#include <QVariant>
#include <QModelIndex>
#include <QAbstractListModel>
#include <QStringList>

#include "dummykonadiadapter.h"

class FolderModel : public QAbstractListModel
{
  Q_OBJECT
public:
  FolderModel(const DummyKonadiAdapter &dummydata, QObject *parent = 0) : QAbstractListModel(parent), backend(dummydata) {}

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
  DummyKonadiAdapter backend;
};

#endif
