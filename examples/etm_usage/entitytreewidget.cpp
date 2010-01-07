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

#include "entitytreewidget.h"

#include <QTreeView>

#include <akonadi/entitytreemodel.h>
#include <akonadi/monitor.h>
#include <akonadi/itemfetchscope.h>

using namespace Akonadi;

EntityTreeWidget::EntityTreeWidget(QWidget* parent)
  : QWidget(parent),
    m_treeView(new QTreeView(this)),
    m_typeComboBox(new QComboBox(this)),
    m_typeLineEdit(new QLineEdit(this)),
    m_monitor(new Monitor(this))
{
  m_monitor->itemFetchScope().fetchFullPayload(true);
  m_monitor->itemFetchScope().fetchAllAttributes(true);
  m_etm = new EntityTreeModel(m_monitor, this);


}

