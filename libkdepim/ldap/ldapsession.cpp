/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
    Author: Volker Krause <volker.krause@kdab.com>

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

#include "ldapsession.h"

#include <kldap/ldapoperation.h>
#include <kldap/ldif.h>
#include <kldap/ldapcontrol.h>
#include <kldap/ldapdefs.h>

#include "libkdepim_debug.h"
#include "ldapqueryjob.h"

using namespace KLDAP;

LdapSession::LdapSession(QObject *parent) :
    QThread(parent),
    m_state(Disconnected),
    m_currentJob(0)
{
    qCDebug(LIBKDEPIM_LOG);
}

// runs in other thread
void LdapSession::connectToServer(const KLDAP::LdapServer &server)
{
    qCDebug(LIBKDEPIM_LOG);
    if (m_state != Disconnected) {
        return;
    }
    m_server = server;
    start();
}

// runs in this thread
void LdapSession::connectToServerInternal()
{
    qCDebug(LIBKDEPIM_LOG);
    m_conn.setServer(m_server);
    if (m_conn.connect() != 0) {
        qCWarning(LIBKDEPIM_LOG) << "failed to connect: " << m_conn.connectionError();
        return;
    }
    m_state = Connected;
    authenticate();
}

// runs in other threads
void LdapSession::disconnectAndDelete()
{
    qCDebug(LIBKDEPIM_LOG);
    QMetaObject::invokeMethod(this, "quit", Qt::QueuedConnection);
    QMetaObject::invokeMethod(this, "deleteLater", Qt::QueuedConnection);
}

// runs in this thread
void LdapSession::disconnectFromServerInternal()
{
    qCDebug(LIBKDEPIM_LOG);
    m_conn.close();
    m_state = Disconnected;
}

void LdapSession::authenticate()
{
    qCDebug(LIBKDEPIM_LOG);
    LdapOperation op(m_conn);
    while (true) {
        int retval = op.bind_s();
        if (retval == 0) {
            qCDebug(LIBKDEPIM_LOG) << "connected!";
            m_state = Authenticated;
            return;
        }
        if (retval == KLDAP_INVALID_CREDENTIALS ||
                retval == KLDAP_INSUFFICIENT_ACCESS ||
                retval == KLDAP_INAPPROPRIATE_AUTH  ||
                retval == KLDAP_UNWILLING_TO_PERFORM) {

            if (m_server.auth() != LdapServer::SASL) {
                m_server.setBindDn(m_server.user());
            }
            m_conn.setServer(m_server);
        } else {
//       LDAPErr( retval );
            disconnectFromServerInternal();
            qCDebug(LIBKDEPIM_LOG) << "error" << retval;
            return;
        }
    }
}

// called from other thread
LdapQueryJob *LdapSession::get(const KLDAP::LdapUrl &url)
{
    qCDebug(LIBKDEPIM_LOG) << url;
    LdapQueryJob *job = new LdapQueryJob(url, this);
    job->moveToThread(this);   // make sure the job is in the thread so that the result connections are queued
    connect(job, &LdapQueryJob::result, this, &LdapSession::jobDone);
    QMutexLocker locker(&m_mutex);
    m_jobQueue.enqueue(job);
    QMetaObject::invokeMethod(this, "executeNext", Qt::QueuedConnection);
    return job;
}

void LdapSession::run()
{
    connectToServerInternal();
    QMetaObject::invokeMethod(this, "executeNext", Qt::QueuedConnection);
    exec();
    disconnectFromServerInternal();
}

void LdapSession::executeNext()
{
    if (m_state != Authenticated || m_currentJob) {
        return;
    }
    QMutexLocker locker(&m_mutex);
    if (m_jobQueue.isEmpty()) {
        return;
    }
    m_currentJob = m_jobQueue.dequeue();
    locker.unlock();
    QMetaObject::invokeMethod(m_currentJob, "triggerStart", Qt::QueuedConnection);
}

LdapServer LdapSession::server() const
{
    return m_server;
}

LdapConnection &LdapSession::connection()
{
    return m_conn;
}

void LdapSession::jobDone(KJob *job)
{
    if (m_currentJob == job) {
        m_currentJob = 0;
    }
    QMutexLocker locker(&m_mutex);
    m_jobQueue.removeAll(static_cast<LdapQueryJob *>(job));
    locker.unlock();
    QMetaObject::invokeMethod(this, "executeNext", Qt::QueuedConnection);
}

