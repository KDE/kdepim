/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#ifndef AGENTSTATUSMONITOR_H
#define AGENTSTATUSMONITOR_H

#include <AkonadiCore/mimetypechecker.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>

class AgentStatusMonitor : public QObject
{
  Q_OBJECT
  Q_FLAGS( AgentStatus )
  Q_PROPERTY( AgentStatus status READ status NOTIFY statusChanged )

  public:
    enum Status {
      Offline = 0,
      Online = 1,
      Receiving = 2,
      Sending = 4
    };
    Q_DECLARE_FLAGS( AgentStatus, Status )

    explicit AgentStatusMonitor( QObject *parent = 0 );

    AgentStatus status() const;
    void setMimeTypeFilter( const QStringList &mimeTypes );

  Q_SIGNALS:
    void statusChanged();

  private Q_SLOTS:
    void updateStatus();

  private:
    AgentStatus m_status;
    Akonadi::MimeTypeChecker m_mimeTypeChecker;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( AgentStatusMonitor::AgentStatus )

#endif
