/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#ifndef CONFIGURECOLLECTIONS_H
#define CONFIGURECOLLECTIONS_H

#include "abstractconfigurewidget.h"

#include <KViewStateMaintainer>

#include <QWidget>

namespace Akonadi {
  class EntityTreeModel;
  class ChangeRecorder;
  class ETMViewStateSaver;
}

class KCheckableProxyModel;
class QTreeView;

namespace PimActivity {

class ConfigureCollections : public QWidget, public AbstractConfigureWidget
{
    Q_OBJECT
public:
    explicit ConfigureCollections(QWidget *parent = 0);
    ~ConfigureCollections();

    void setDefault();
    void writeConfig(const QString &id);

private:
    void readConfig(const QString &id);

private Q_SLOTS:
    void slotDataChanged();

Q_SIGNALS:
    void changed(bool b = true);

private:
    void initCollections();
    QTreeView *mFolderView;
    QItemSelectionModel *mSelectionModel;
    Akonadi::EntityTreeModel *mModel;
    Akonadi::ChangeRecorder *mChangeRecorder;
    KCheckableProxyModel *mCheckProxy;
    KViewStateMaintainer<Akonadi::ETMViewStateSaver> *mModelState;
};
}

#endif // CONFIGURECOLLECTIONS_H
