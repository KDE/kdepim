/*
  This file is part of KAddressBook.

  Copyright (c) 2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "gravatarupdatejob.h"

#include <gravatar/gravatarresolvurljob.h>

#include <KContacts/Addressee>
#include <AkonadiCore/ItemModifyJob>
#include "kaddressbook_debug.h"

using namespace KABGravatar;

GravatarUpdateJob::GravatarUpdateJob(QObject *parent)
    : QObject(parent)
{

}

GravatarUpdateJob::~GravatarUpdateJob()
{

}

void GravatarUpdateJob::start()
{
    if (canStart()) {
        PimCommon::GravatarResolvUrlJob *job = new PimCommon::GravatarResolvUrlJob(this);
        job->setEmail(mEmail);
        if (job->canStart()) {
            connect(job, &PimCommon::GravatarResolvUrlJob::finished, this, &GravatarUpdateJob::slotGravatarResolvUrlFinished);
            connect(job, &PimCommon::GravatarResolvUrlJob::resolvUrl, this, &GravatarUpdateJob::resolvedUrl);
            job->start();
        } else {
            deleteLater();
        }
    } else {
        deleteLater();
    }

}

bool GravatarUpdateJob::canStart() const
{
    return !mEmail.trimmed().isEmpty() && (mEmail.contains(QLatin1Char('@')));
}

QString GravatarUpdateJob::email() const
{
    return mEmail;
}

void GravatarUpdateJob::setEmail(const QString &email)
{
    mEmail = email;
}

Akonadi::Item GravatarUpdateJob::item() const
{
    return mItem;
}

void GravatarUpdateJob::setItem(const Akonadi::Item &item)
{
    mItem = item;
}

void GravatarUpdateJob::slotGravatarResolvUrlFinished(PimCommon::GravatarResolvUrlJob *job)
{
    if (job) {
        const QPixmap pix = job->pixmap();
        Q_EMIT gravatarPixmap(pix);
        if (mItem.isValid()) {
            updatePixmap(pix);
            return;
        }
    }
    deleteLater();
}

void GravatarUpdateJob::updatePixmap(const QPixmap &pix)
{
    if (mItem.hasPayload<KContacts::Addressee>()) {
        KContacts::Addressee contact = mItem.payload<KContacts::Addressee>();
        contact.photo().setData(pix.toImage());
        mItem.setPayload<KContacts::Addressee>(contact);

        // save the new item in akonadi storage
        Akonadi::ItemModifyJob *modifyJob = new Akonadi::ItemModifyJob(mItem);
        connect(modifyJob, SIGNAL(result(KJob*)), SLOT(slotUpdateGravatarDone(KJob*)));
    } else {
        deleteLater();
    }
}

void GravatarUpdateJob::slotUpdateGravatarDone(KJob *job)
{
    if (job->error()) {
        qCDebug(KADDRESSBOOK_LOG) << "Error during modify item :" << job->errorString();
    }
    deleteLater();
}
