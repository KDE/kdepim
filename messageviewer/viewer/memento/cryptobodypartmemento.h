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

#ifndef CRYPTOBODYPARTMEMENTO_H
#define CRYPTOBODYPARTMEMENTO_H

#include <gpgme++/verificationresult.h>
#include <gpgme++/decryptionresult.h>
#include <gpgme++/key.h>

#include <QObject>
#include <QString>

#include "interfaces/bodypart.h"
#include "viewer/viewer.h"

namespace MessageViewer
{

class CryptoBodyPartMemento
    : public QObject,
      public Interface::BodyPartMemento
{
    Q_OBJECT
public:
    CryptoBodyPartMemento();
    ~CryptoBodyPartMemento();

    bool isRunning() const;

    const QString &auditLogAsHtml() const
    {
        return m_auditLog;
    }
    GpgME::Error auditLogError() const
    {
        return m_auditLogError;
    }

    void detach();

Q_SIGNALS:
    void update(MessageViewer::Viewer::UpdateMode);

protected Q_SLOTS:
    void notify()
    {
        emit update(Viewer::Force);
    }

protected:
    void setAuditLog(const GpgME::Error &err, const QString &log);
    void setRunning(bool running);

private:
    bool m_running;
    QString m_auditLog;
    GpgME::Error m_auditLogError;
};
}
#endif // CRYPTOBODYPARTMEMENTO_H
