/* -*- mode: c++; c-basic-offset:4 -*-
    uiserver/sessiondata.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "sessiondata.h"

#include <QDebug>

#include <QMutex>

#include <boost/bind.hpp>

using namespace Kleo;
using namespace boost;

static const int GARBAGE_COLLECTION_INTERVAL = 60000; // 1min

static QMutex mutex;

SessionData::SessionData()
    : mementos(),
      ref(0),
      ripe(false)
{

}

// static
shared_ptr<SessionDataHandler> SessionDataHandler::instance()
{
    mutex.lock();
    static SessionDataHandler handler;
    return shared_ptr<SessionDataHandler>(&handler, boost::bind(&QMutex::unlock, &mutex));
}

SessionDataHandler::SessionDataHandler()
    : QObject(),
      data(),
      timer()
{
    timer.setInterval(GARBAGE_COLLECTION_INTERVAL);
    timer.setSingleShot(false);
}

void SessionDataHandler::enterSession(unsigned int id)
{
    qDebug() << id;
    const shared_ptr<SessionData> sd = sessionDataInternal(id);
    assert(sd);
    ++sd->ref;
    sd->ripe = false;
}

void SessionDataHandler::exitSession(unsigned int id)
{
    qDebug() << id;
    const shared_ptr<SessionData> sd = sessionDataInternal(id);
    assert(sd);
    if (--sd->ref <= 0) {
        sd->ref = 0;
        sd->ripe = false;
        if (!timer.isActive()) {
            QMetaObject::invokeMethod(&timer, "start", Qt::QueuedConnection);
        }
    }
}

shared_ptr<SessionData> SessionDataHandler::sessionDataInternal(unsigned int id) const
{
    std::map< unsigned int, shared_ptr<SessionData> >::iterator
    it = data.lower_bound(id);
    if (it == data.end() || it->first != id) {
        const shared_ptr<SessionData> sd(new SessionData);
        it = data.insert(it, std::make_pair(id, sd));
    }
    return it->second;
}

shared_ptr<SessionData> SessionDataHandler::sessionData(unsigned int id) const
{
    return sessionDataInternal(id);
}

void SessionDataHandler::clear()
{
    data.clear();
}

void SessionDataHandler::slotCollectGarbage()
{
    const QMutexLocker locker(&mutex);
    unsigned int alive = 0;
    std::map< unsigned int, shared_ptr<SessionData> >::iterator it = data.begin(), end = data.end();
    while (it != end)
        if (it->second->ripe) {
            data.erase(it++);
        } else if (!it->second->ref) {
            it->second->ripe = true;
            ++it;
        } else {
            ++alive;
            ++it;
        }
    if (alive == data.size()) {
        QMetaObject::invokeMethod(&timer, "stop", Qt::QueuedConnection);
    }
}
