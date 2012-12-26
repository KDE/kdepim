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

#ifndef STATEMACHINEBUILDER_H
#define STATEMACHINEBUILDER_H

#include "mobileui_export.h"

#include <QStateMachine>
#include <QSignalTransition>

class QObject;
class QItemSelectionModel;

class StateMachineBuilderPrivate;


class MOBILEUI_EXPORT NotifyingStateMachine : public QStateMachine
{
  Q_OBJECT
public:
  explicit NotifyingStateMachine(QObject *parent = 0);

  void requestState(const QString &state);

signals:
  void stateRequested(const QString &state);
  void stateChanged();
};

class RequestNamedTransition : public QSignalTransition
{
  Q_OBJECT
public:
  explicit RequestNamedTransition(QStateMachine *stateMachine, QState* sourceState = 0);

  virtual bool eventTest(QEvent* event);

};

/**
  Machines must be built by builders.
*/
class MOBILEUI_EXPORT StateMachineBuilder
{
public:
  StateMachineBuilder();
  virtual ~StateMachineBuilder();

  void setNavigationModel(QItemSelectionModel *model);
  void setItemSelectionModel(QItemSelectionModel *model);
  // TODO: Decide on granularity of the interface.
  virtual NotifyingStateMachine* getMachine(QObject *parent) const;

private:
  Q_DECLARE_PRIVATE(StateMachineBuilder)
  StateMachineBuilderPrivate * const d_ptr;
};

#endif
