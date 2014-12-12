/* Copyright (C) 2012,2013,2014 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "contactdisplaymessagememento.h"
#include "messageviewer_debug.h"
#include <Akonadi/Contact/ContactSearchJob>


using namespace MessageViewer;

ContactDisplayMessageMemento::ContactDisplayMessageMemento(const QString &emailAddress)
    : QObject(Q_NULLPTR),
      mFinished(false),
      mMailAllowToRemoteContent(false),
      mForceDisplayTo(Viewer::UseGlobalSetting)
{
    if (!emailAddress.isEmpty()) {
        Akonadi::ContactSearchJob *searchJob = new Akonadi::ContactSearchJob();
        searchJob->setQuery(Akonadi::ContactSearchJob::Email, emailAddress.toLower(), Akonadi::ContactSearchJob::ExactMatch);
        connect(searchJob, &Akonadi::ContactSearchJob::result, this, &ContactDisplayMessageMemento::slotSearchJobFinished);
    } else {
        mFinished = true;
    }
}

ContactDisplayMessageMemento::~ContactDisplayMessageMemento()
{
}

void ContactDisplayMessageMemento::slotSearchJobFinished(KJob *job)
{
    mFinished = true;
    Akonadi::ContactSearchJob *searchJob = static_cast<Akonadi::ContactSearchJob *>(job);
    if (searchJob->error()) {
        qWarning() << "Unable to fetch contact:" << searchJob->errorText();
        emit update(Viewer::Delayed);
        return;
    }

    const int contactSize(searchJob->contacts().size());
    if (contactSize >= 1) {
        searchPhoto(searchJob->contacts());
        KContacts::Addressee addressee = searchJob->contacts().first();
        processAddress(addressee);
        emit update(Viewer::Delayed);
        if (contactSize > 1) {
            qCDebug(MESSAGEVIEWER_LOG) << " more than 1 contact was found we return first contact";
        }
    }
}

bool ContactDisplayMessageMemento::finished() const
{
    return mFinished;
}

void ContactDisplayMessageMemento::detach()
{
    disconnect(this, SIGNAL(update(MessageViewer::Viewer::UpdateMode)), 0, 0);
    disconnect(this, SIGNAL(changeDisplayMail(Viewer::DisplayFormatMessage,bool)), 0 , 0);
}

bool ContactDisplayMessageMemento::allowToRemoteContent() const
{
    return mMailAllowToRemoteContent;
}

void ContactDisplayMessageMemento::searchPhoto(const KContacts::AddresseeList &list)
{
    Q_FOREACH (const KContacts::Addressee &addressee, list) {
        if (!addressee.photo().isEmpty()) {
            mPhoto = addressee.photo();
            break;
        }
    }
}

void ContactDisplayMessageMemento::processAddress(const KContacts::Addressee &addressee)
{
    const QStringList customs = addressee.customs();
    Q_FOREACH (const QString &custom, customs) {
        if (custom.contains(QLatin1String("MailPreferedFormatting"))) {
            const QString value = addressee.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("MailPreferedFormatting"));
            if (value == QLatin1String("TEXT")) {
                mForceDisplayTo = Viewer::Text;
            } else if (value == QLatin1String("HTML")) {
                mForceDisplayTo = Viewer::Html;
            } else {
                mForceDisplayTo = Viewer::UseGlobalSetting;
            }
        } else if (custom.contains(QLatin1String("MailAllowToRemoteContent"))) {
            const QString value = addressee.custom(QLatin1String("KADDRESSBOOK"), QLatin1String("MailAllowToRemoteContent"));
            mMailAllowToRemoteContent = (value == QLatin1String("TRUE"));
        }
    }
    emit changeDisplayMail(mForceDisplayTo, mMailAllowToRemoteContent);
}

KContacts::Picture ContactDisplayMessageMemento::photo() const
{
    return mPhoto;
}

