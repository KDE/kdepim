/*
 * Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef INCIDENCEEDITOR_OPENCOMPOSERJOB_H
#define INCIDENCEEDITOR_OPENCOMPOSERJOB_H

#include <KIdentityManagement/Identity>
#include <kmime/kmime_message.h>

#include <KJob>
namespace IncidenceEditorNG {

// Opens a Composer with a mail with one attachment (costructed my ITIPHandler)
class OpenComposerJob : public KJob
{
    Q_OBJECT

public:
    explicit OpenComposerJob(QObject *parent,
                             const QString &to, const QString &cc, const QString &bcc,
                             const KMime::Message::Ptr &message, const KIdentityManagement::Identity &identity);
    virtual ~OpenComposerJob();

    virtual void start();

private slots:
    void slotServiceOwnerChanged(const QString &, const QString &, const QString &);
    void timeout();
    void processMail();

private:
    QString mDBusService;
    QString mError;
    QString mTo, mCc, mBcc;
    KMime::Message::Ptr mMessage;
    KIdentityManagement::Identity mIdentity;
    bool mSuccess;
};

}
#endif
