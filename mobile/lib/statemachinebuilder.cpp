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

#include "statemachinebuilder.h"

#include "modelselectiontransition.h"

#include <QStateMachine>

const char stateIdentifier[] = "state";

class StateMachineBuilderPrivate
{
  StateMachineBuilderPrivate(StateMachineBuilder *qq)
    : q_ptr(qq),
      m_navigationModel(0),
      m_itemSelectionModel(0)
  {

  }
  Q_DECLARE_PUBLIC(StateMachineBuilder)
  StateMachineBuilder * const q_ptr;

  QItemSelectionModel *m_navigationModel;
  QItemSelectionModel *m_itemSelectionModel;
};

void StateMachineBuilder::setItemSelectionModel(QItemSelectionModel* model)
{
  Q_D(StateMachineBuilder);
  d->m_itemSelectionModel = model;
}

void StateMachineBuilder::setNavigationModel(QItemSelectionModel* model)
{
  Q_D(StateMachineBuilder);
  d->m_navigationModel = model;
}
#if 0
QStateMachine* StateMachineBuilder::getMachine(QObject *parent) const
{
  Q_D(StateMachineBuilder);
  QStateMachine *machine = new QStateMachine(parent);
  QState *homeState = new QState(machine);
  machine->setInitialState(homeState);
  homeState->assignProperty(parent, stateIdentifier, "Home");
  QState *accountState = new QState(machine);
  accountState->assignProperty(parent, stateIdentifier, "Account");
  QState *folderState = new QState(machine);
  folderState->assignProperty(parent, stateIdentifier, "Folder");
  QState *multiFolderState = new QState(machine);
  multiFolderState->assignProperty(parent, stateIdentifier, "MultiFolder");
  QState *singleItemState = new QState(machine);
  singleItemState->assignProperty(parent, stateIdentifier, "SingleItem");

  QState *selectState = new QState(machine);
  selectState->assignProperty(parent, stateIdentifier, "Select");
  QState *bulkActionState = new QState(machine);
  bulkActionState->assignProperty(parent, stateIdentifier, "BulkAction");

  {
  ModelSelectionTransition *homeToAccount = new ModelSelectionTransition(d->m_navigationModel,
                                                                         ModelSelectionTransition::AcceptSingleTopLevelSelection,
                                                                         homeState);
  homeToAccount->setTargetState(accountState);
  }
  {
  ModelSelectionTransition *accountToFolder = new ModelSelectionTransition(d->m_navigationModel,
                                                                           ModelSelectionTransition::AcceptSingleSelection,
                                                                           accountState);
  accountToFolder->setTargetState(folderState);
  }
  {
  ModelSelectionTransition *accountToHome = new ModelSelectionTransition(d->m_navigationModel,
                                                                         ModelSelectionTransition::AcceptNoSelection,
                                                                         accountState);
  accountToHome->setTargetState(homeState);
  }
  {
  ModelSelectionTransition *accountToSingleItem = new ModelSelectionTransition(d->m_itemSelectionModel,
                                                                               ModelSelectionTransition::AcceptSingleSelection,
                                                                               accountState);
  accountToSingleItem->setTargetState(singleItemState);
  }
  {
  ModelSelectionTransition *singleItemToAccount = new ModelSelectionTransition(d->m_itemSelectionModel,
                                                                               ModelSelectionTransition::AcceptNoSelection,
                                                                               singleItemState);
  singleItemToAccount->setTargetState(accountState);
  }
  {
  ModelSelectionTransition *folderToSingleItem = new ModelSelectionTransition(d->m_itemSelectionModel,
                                                                              ModelSelectionTransition::AcceptSingleSelection,
                                                                              folderState);
  folderToSingleItem->setTargetState(singleItemState);
  }
  {
  ModelSelectionTransition *singleItemToFolder = new ModelSelectionTransition(d->m_itemSelectionModel,
                                                                              ModelSelectionTransition::AcceptNoSelection,
                                                                              singleItemState);
  singleItemToFolder->setTargetState(folderState);
  }
  {
  ModelSelectionTransition *folderToHome = new ModelSelectionTransition(d->m_navigationModel,
                                                                         ModelSelectionTransition::AcceptNoSelection,
                                                                         folderState);
  folderToHome->setTargetState(homeState);
  }
  {
  ModelSelectionTransition *homeToMulti = new ModelSelectionTransition(d->m_navigationModel,
                                                                         ModelSelectionTransition::AcceptMultiSelection,
                                                                         homeState);
  homeToMulti->setTargetState(accountState);
  }
  {
  ModelSelectionTransition *multiToHome = new ModelSelectionTransition(d->m_navigationModel,
                                                                       ModelSelectionTransition::AcceptNoSelection,
                                                                       multiFolderState);
  multiToHome->setTargetState(homeState);
  }
  {
  ModelSelectionTransition *multiToSingleItem = new ModelSelectionTransition(d->m_itemSelectionModel,
                                                                             ModelSelectionTransition::AcceptSingleSelection,
                                                                             multiFolderState);
  multiToSingleItem->setTargetState(singleItemState);
  }
  {
  ModelSelectionTransition *singleItemToMulti = new ModelSelectionTransition(d->m_itemSelectionModel,
                                                                             ModelSelectionTransition::AcceptNoSelection,
                                                                             singleItemState);
  singleItemToMulti->setTargetState(multiFolderState);
  }
  {
  ModelSelectionTransition *multiToFolder = new ModelSelectionTransition(d->m_navigationModel,
                                                                         ModelSelectionTransition::AcceptSingleSelection,
                                                                         multiFolderState);
  multiToFolder->setTargetState(folderState);
  }
  {
  ModelSelectionTransition *multiToAccount = new ModelSelectionTransition(d->m_navigationModel,
                                                                         ModelSelectionTransition::AcceptSingleTopLevelSelection,
                                                                         multiFolderState);
  multiToAccount->setTargetState(folderState);
  }


}

#endif