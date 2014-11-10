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

#include "opencomposerjob.h"

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include <QtCore/QProcess>

#include <klocalizedstring.h>
#include <kdbusservicestarter.h>

using namespace IncidenceEditorNG;

OpenComposerJob::OpenComposerJob(QObject *parent,
                                 const QString &to, const QString &cc, const QString &bcc,
                                 const KMime::Message::Ptr &message, const KIdentityManagement::Identity &identity)
    : KJob(parent)
    , mTo(to)
    , mCc(cc)
    , mBcc(bcc)
    , mMessage(message)
    , mIdentity(identity)
    , mSuccess(false)
{
    connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString,QString,QString)),
            SLOT(slotServiceOwnerChanged(QString,QString,QString)));
}

OpenComposerJob::~OpenComposerJob()
{

}

void OpenComposerJob::start()
{
    mSuccess = false;
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.kmail"))) {
        processMail();
    }
    //Check if Kontact is already running and if not ...
    int result = KDBusServiceStarter::self()->findServiceFor(QLatin1String("DBUS/Mailer"), QString(),
                 &mError, &mDBusService);
    if (result != 0) {
        // ... start Kontact
        result = KDBusServiceStarter::self()->startServiceFor(QLatin1String("DBUS/Mailer"), QString(),
                 &mError, &mDBusService);
        if (result != 0) {
            const bool ok = QProcess::startDetached(QLatin1String("kontact"));
            if (!ok) {
                setError(KJob::UserDefinedError);
                setErrorText(i18nc("errormessage: can't get connection via dbus", "Don't get kmail started"));
                emitResult();
                return;
            }
        } else {
            setError(KJob::UserDefinedError);
            setErrorText(i18nc("errormessage: can't get connection via dbus", "Don't get kmail started"));
            emitResult();
            return;
        }
    }

    QTimer::singleShot(10000, this, SLOT(timeout()));
}

void OpenComposerJob::timeout()
{
    if (!mSuccess) {
        setError(KJob::UserDefinedError);
        setErrorText(i18nc("errormessage: can't get connection via dbus", "Don't get kmail started"));
        emitResult();
    }
}

void OpenComposerJob::slotServiceOwnerChanged(const QString &name, const QString &oldOwner, const QString &newOwner)
{
    Q_UNUSED(oldOwner);
    if (name == QLatin1String("org.kde.kmail") && !newOwner.isEmpty()) {
        processMail();
    }
}

void OpenComposerJob::processMail()
{
    Q_ASSERT(mMessage);
    mSuccess = true;

    bool hidden = false;
    unsigned int identity = mIdentity.uoid();

    QString subject = mMessage->subject()->asUnicodeString();
    QString body = QString::fromUtf8(mMessage->contents()[0]->body());

    QList<QVariant> messages;

    if (mMessage->contents().count() == 1) {
        const QString messageFile;
        const QStringList attachmentPaths;
        const QStringList customHeaders;
        const QString replyTo;
        const QString inReplyTo;

        messages << mTo << mCc << mBcc << subject << body << hidden
                 << messageFile << attachmentPaths << customHeaders << replyTo << inReplyTo;
    } else {
        KMime::Content *attachment(mMessage->contents().at(1));
        QString attachName = attachment->contentType()->name();
        QByteArray attachCte = attachment->contentTransferEncoding()->as7BitString(false);
        QByteArray attachType = attachment->contentType()->mediaType();
        QByteArray attachSubType = attachment->contentType()->subType();
        QByteArray attachContDisp = attachment->contentDisposition()->as7BitString(false);
        QByteArray attachCharset = attachment->contentType()->charset();

        QByteArray attachParamAttr = "method";
        QString attachParamValue = attachment->contentType()->parameter(QLatin1String("method"));
        QByteArray attachData = attachment->encodedBody();

        messages << mTo << mCc << mBcc << subject << body
                 << attachName << attachCte << attachData << attachType << attachSubType
                 << attachParamAttr << attachParamValue << attachContDisp << attachCharset
                 << identity;
    }
    QDBusInterface kmailObj(QLatin1String("org.kde.kmail"), QLatin1String("/KMail"), QLatin1String("org.kde.kmail.kmail"));

    QDBusReply<int> composerDbusPath = kmailObj.callWithArgumentList(QDBus::AutoDetect, QLatin1String("openComposer"), messages);

    if (!composerDbusPath.isValid()) {
        setError(KJob::UserDefinedError);
        setErrorText(i18nc("errormessage: dbus is running but still no connection kmail", "Cannot connect to email service"));
    }
    emitResult();

}
