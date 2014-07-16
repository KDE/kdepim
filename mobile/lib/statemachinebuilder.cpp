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

const char stateIdentifier[] = "state";

NotifyingStateMachine::NotifyingStateMachine(QObject* parent)
  : QStateMachine(parent)
{

}

void NotifyingStateMachine::requestState(const QString& state)
{
  emit stateRequested(state);
}

RequestNamedTransition::RequestNamedTransition(QStateMachine *stateMachine, QState* sourceState)
  : QSignalTransition(stateMachine, SIGNAL(stateRequested(QString)), sourceState)
{

}


bool RequestNamedTransition::eventTest(QEvent* event)
{
  if (!QSignalTransition::eventTest(event))
    return false;

  QStateMachine::SignalEvent *se = static_cast<QStateMachine::SignalEvent*>(event);
  const QString name = se->arguments().first().toString();
  return !name.isEmpty() && name == targetState()->objectName();
}


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

StateMachineBuilder::StateMachineBuilder()
  : d_ptr(new StateMachineBuilderPrivate(this))
{

}

StateMachineBuilder::~StateMachineBuilder()
{
  delete d_ptr;
}


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

NotifyingStateMachine* StateMachineBuilder::getMachine(QObject *parent) const
{
  Q_D(const StateMachineBuilder);
  NotifyingStateMachine *machine = new NotifyingStateMachine(parent);
  QState *mainWorkState = new QState(machine);
  machine->setInitialState(mainWorkState);
  mainWorkState->setObjectName(QLatin1String("MainWork"));
  QObject::connect(mainWorkState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  QState *homeState = new QState(mainWorkState);
  mainWorkState->setInitialState(homeState);
  homeState->setObjectName(QLatin1String("Home"));
  homeState->assignProperty(parent, stateIdentifier, QLatin1String("Home"));
  QObject::connect(homeState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  QState *accountState = new QState(mainWorkState);
  accountState->assignProperty(parent, stateIdentifier, QLatin1String("Account"));
  QObject::connect(accountState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  accountState->setObjectName(QLatin1String("Account"));
  QState *folderState = new QState(mainWorkState);
  folderState->assignProperty(parent, stateIdentifier, QLatin1String("Folder"));
  QObject::connect(folderState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  folderState->setObjectName(QLatin1String("Folder"));
  QState *multiFolderState = new QState(mainWorkState);
  multiFolderState->assignProperty(parent, stateIdentifier, QLatin1String("MultiFolder"));
  QObject::connect(multiFolderState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  multiFolderState->setObjectName(QLatin1String("MultiFolder"));
  QState *singleItemState = new QState(mainWorkState);
  singleItemState->assignProperty(parent, stateIdentifier, QLatin1String("SingleItem"));
  QObject::connect(singleItemState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  singleItemState->setObjectName(QLatin1String("SingleItem"));

  QState *selectState = new QState(machine);
  selectState->assignProperty(parent, stateIdentifier, QLatin1String("Select"));
  QObject::connect(selectState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  selectState->setObjectName(QLatin1String("Select"));
  QState *bulkActionState = new QState(machine);
  bulkActionState->assignProperty(parent, stateIdentifier, QLatin1String("BulkAction"));
  QObject::connect(bulkActionState, SIGNAL(entered()), machine, SIGNAL(stateChanged()));
  bulkActionState->setObjectName(QLatin1String("BulkAction"));

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
  {
    RequestNamedTransition *transition = new RequestNamedTransition(machine, mainWorkState);
    transition->setTargetState(selectState);
  }
  {
    RequestNamedTransition *transition = new RequestNamedTransition(machine, selectState);
    transition->setTargetState(mainWorkState);
  }
  {
    RequestNamedTransition *transition = new RequestNamedTransition(machine, mainWorkState);
    transition->setTargetState(bulkActionState);
  }
  {
    RequestNamedTransition *transition = new RequestNamedTransition(machine, bulkActionState);
    transition->setTargetState(mainWorkState);
  }
  return machine;
}

