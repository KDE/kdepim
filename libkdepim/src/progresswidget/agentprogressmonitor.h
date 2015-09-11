/*
    Copyright (c) 2009 Constantin Berzan <exit3219@gmail.com>

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

#ifndef KDEPIM_AGENTPROGRESSMONITOR_H
#define KDEPIM_AGENTPROGRESSMONITOR_H

#include "progressmanager.h" // ProgressItem

#include <AgentInstance>

namespace KPIM
{

/**
 * @internal
 *
 * This class automatically updates a ProgressItem based on
 * Akonadi::AgentManager's signals, and places the abort() call if the
 * ProgressItem has been cancelled.
 */
class AgentProgressMonitor : public QObject
{
    Q_OBJECT
    friend class ProgressManager;

protected:
    // used by our friend ProgressManager
    AgentProgressMonitor(const Akonadi::AgentInstance &agent, ProgressItem *item);
    ~AgentProgressMonitor();

private:
    class Private;
    Private *const d;

    Q_PRIVATE_SLOT(d, void abort())
    Q_PRIVATE_SLOT(d, void instanceProgressChanged(const Akonadi::AgentInstance &))
    Q_PRIVATE_SLOT(d, void instanceStatusChanged(const Akonadi::AgentInstance &))
    Q_PRIVATE_SLOT(d, void instanceRemoved(const Akonadi::AgentInstance &))
    Q_PRIVATE_SLOT(d, void instanceNameChanged(const Akonadi::AgentInstance &))

};

}

#endif
