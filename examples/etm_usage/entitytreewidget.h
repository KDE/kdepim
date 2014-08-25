/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>

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

// READ THE README FILE

#ifndef ENTITYTREEWIDGET_H
#define ENTITYTREEWIDGET_H

#include <QWidget>
#include <QModelIndex>

class QTreeView;
class QLineEdit;
class QComboBox;

namespace Akonadi
{
class EntityTreeModel;
class ChangeRecorder;
}

class EntityTreeWidget : public QWidget
{
    Q_OBJECT
public:
    EntityTreeWidget(QWidget *parent = 0);
    virtual ~EntityTreeWidget();

    QTreeView *view() const;
    Akonadi::EntityTreeModel *model() const;

    Akonadi::ChangeRecorder *changeRecorder() const;

    virtual void connectTreeToModel(QTreeView *tree, Akonadi::EntityTreeModel *model);
    virtual QModelIndex mapToSource(const QModelIndex &index)
    {
        return index;
    }
    virtual Akonadi::EntityTreeModel *getETM();

public:
    void init();
    QString dumpLevel(const QModelIndex &parent, int level);

public slots:
    void mimeTypesChoiceChanged(int index);
    void mimeTypesChanged(const QString &mimetypeList);
    void dumpTree();

private:
    QTreeView *m_treeView;
    QComboBox *m_typeComboBox;
    QLineEdit *m_typeLineEdit;

    Akonadi::EntityTreeModel *m_etm;
    Akonadi::ChangeRecorder *m_changeRecorder;
};

#endif
