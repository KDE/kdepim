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

#include "gravatarcreatejob.h"
#include "pimcommon/gravatar/gravatarresolvurljob.h"

using namespace KABGravatar;

GravatarCreateJob::GravatarCreateJob(QObject *parent)
    : QObject(parent)
{

}

GravatarCreateJob::~GravatarCreateJob()
{

}

bool GravatarCreateJob::canStart()
{
    return !mEmail.trimmed().isEmpty() && (mEmail.contains(QLatin1Char('@')));
}

void GravatarCreateJob::start()
{
    PimCommon::GravatarResolvUrlJob *job = new PimCommon::GravatarResolvUrlJob(this);
    job->setEmail(mEmail);
    if (job->canStart()) {
        connect(job, SIGNAL(finished(PimCommon::GravatarResolvUrlJob*)), this, SLOT(slotGravatarResolvUrlFinished(PimCommon::GravatarResolvUrlJob*)));
        connect(job, &PimCommon::GravatarResolvUrlJob::resolvUrl, this, &GravatarCreateJob::resolvedUrl);
        job->start();
    } else {
        deleteLater();
    }
}

QString GravatarCreateJob::email() const
{
    return mEmail;
}

void GravatarCreateJob::setEmail(const QString &email)
{
    mEmail = email;
}

void GravatarCreateJob::slotGravatarResolvUrlFinished(PimCommon::GravatarResolvUrlJob *job)
{
    if (job) {
        Q_EMIT gravatarPixmap(job->pixmap());
    }
    deleteLater();
}
