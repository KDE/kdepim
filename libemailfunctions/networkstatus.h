/*
    This file is part of libkdepim.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef NETWORKSTATUS_H
#define NETWORKSTATUS_H

#include <qobject.h>
#include <dcopobject.h>

namespace KPIM {

/**
 */
class NetworkStatus : public QObject, public DCOPObject
{
  Q_OBJECT

  public:
    /**
     * The possible states.
     */
    enum Status {
      Online,
      Offline
    };

    /**
     * Destructor.
     */
    ~NetworkStatus();

    /**
     * Returns the only instance of this class.
     */
    static NetworkStatus *self();

    /**
     * Sets a new status.
     *
     * @param status The new status.
     */
    void setStatus( Status status );

    /**
     * Returns the current status.
     */
    Status status() const;

  k_dcop:
    /**
     * Called by the network interface watcher in KDED.
     */
    void onlineStatusChanged();

  signals:
    /**
     * Emitted whenever the status has changed.
     *
     * @param status The new status.
     */
    void statusChanged( Status status );

  protected:
    NetworkStatus();

  private:
    Status mStatus;
    static NetworkStatus *mSelf;
};

}

#endif
