/*
    Copyright (C) 2010 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Stephen Kelly <stephen@kdab.com>

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

#include "modelselectiontransition.h"

class ModelSelectionTransitionPrivate
{
  ModelSelectionTransitionPrivate(ModelSelectionTransition *qq, QItemSelectionModel *selectionModel, ModelSelectionTransition::Type type)
    : q_ptr(qq), m_selectionModel(selectionModel), m_type(type)
  {

  }
  Q_DECLARE_PUBLIC(ModelSelectionTransition)
  ModelSelectionTransition * const q_ptr;

  QItemSelectionModel * const m_selectionModel;
  ModelSelectionTransition::Type m_type;
};

ModelSelectionTransition::ModelSelectionTransition(QItemSelectionModel* selectionModel, ModelSelectionTransition::Type type, QState* sourceState)
  : QSignalTransition(selectionModel,
                      SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                      sourceState),
    d_ptr(new ModelSelectionTransitionPrivate(this, selectionModel, type))
{

}

ModelSelectionTransition::~ModelSelectionTransition()
{
    delete d_ptr;
}

bool ModelSelectionTransition::eventTest(QEvent* event)
{
  Q_D(ModelSelectionTransition);
  if (!QSignalTransition::eventTest(event))
    return false;

  // Yes, it does feel wrong to use a switch statement in a state machine.
  // Oh well.
  switch (d->m_type) {
  case AcceptNoSelection:
    return !d->m_selectionModel->hasSelection();
  case AcceptMultiSelection:
    return d->m_selectionModel->selectedRows().size() > 1;
  case AcceptSingleSelection:
    return d->m_selectionModel->selectedRows().size() == 1;
  case AcceptSingleTopLevelSelection: {
    const QModelIndexList list = d->m_selectionModel->selectedRows();
    return list.size() == 1 && !list.first().parent().isValid();
  }
  }
  Q_ASSERT(!"Unknown type");
  return false;
}

