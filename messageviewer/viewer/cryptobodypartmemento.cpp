/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "cryptobodypartmemento.h"


#include <kleo/decryptverifyjob.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/verifyopaquejob.h>
#include <kleo/keylistjob.h>

#include <gpgme++/keylistresult.h>

using namespace Kleo;
using namespace GpgME;
using namespace MessageViewer;

CryptoBodyPartMemento::CryptoBodyPartMemento()
    : QObject(0),
      Interface::BodyPartMemento(),
      m_running(false)
{

}

CryptoBodyPartMemento::~CryptoBodyPartMemento()
{

}

bool CryptoBodyPartMemento::isRunning() const
{
    return m_running;
}

void CryptoBodyPartMemento::setAuditLog(const Error &err, const QString &log)
{
    m_auditLogError = err;
    m_auditLog = log;
}

void CryptoBodyPartMemento::setRunning(bool running)
{
    m_running = running;
}

void CryptoBodyPartMemento::detach()
{
    disconnect(this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0);
}

