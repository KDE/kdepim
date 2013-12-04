/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "ubuntuonestorageservice.h"
#include "ubuntuonejob.h"

#include <KLocale>
#include <KConfig>
#include <KGlobal>
#include <KConfigGroup>


using namespace PimCommon;

UbuntuoneStorageService::UbuntuoneStorageService(QObject *parent)
    : PimCommon::StorageServiceAbstract(parent)
{
    readConfig();
}

UbuntuoneStorageService::~UbuntuoneStorageService()
{
}

void UbuntuoneStorageService::readConfig()
{
    KConfigGroup grp(KGlobal::config(), "Ubuntu One Settings");

    mCustomerSecret = grp.readEntry("Customer Secret");
    mToken = grp.readEntry("Token");
    mCustomerKey = grp.readEntry("Customer Key");
    mTokenSecret = grp.readEntry("Token Secret");
}

void UbuntuoneStorageService::slotAuthorizationDone(const QString &customerSecret, const QString &token, const QString &customerKey, const QString &tokenSecret)
{
    mCustomerSecret = customerSecret;
    mToken = token;
    mCustomerKey = customerKey;
    mTokenSecret = tokenSecret;

    KConfigGroup grp(KGlobal::config(), "Ubuntu One Settings");
    grp.writeEntry("Customer Secret", mCustomerSecret);
    grp.writeEntry("Token", mToken);
    grp.writeEntry("Customer Key", mCustomerKey);
    grp.writeEntry("Token Secret", mTokenSecret);

    grp.sync();
    KGlobal::config()->sync();
}

void UbuntuoneStorageService::removeConfig()
{
    KConfigGroup grp(KGlobal::config(), "Ubuntu One Settings");
    grp.deleteGroup();
    KGlobal::config()->sync();
}

void UbuntuoneStorageService::authentification()
{
    UbuntuOneJob *job = new UbuntuOneJob(this);
    connect(job, SIGNAL(authorizationDone(QString,QString,QString,QString)), this, SLOT(slotAuthorizationDone(QString,QString,QString,QString)));
    job->requestTokenAccess();

    //TODO connect
}

void UbuntuoneStorageService::listFolder()
{

}

void UbuntuoneStorageService::createFolder(const QString &folder)
{

}

void UbuntuoneStorageService::accountInfo()
{
    UbuntuOneJob *job = new UbuntuOneJob(this);
    job->initializeToken(mCustomerSecret, mToken, mCustomerKey, mTokenSecret);
    connect(job,SIGNAL(accountInfoDone(PimCommon::AccountInfo)), this, SLOT(slotAccountInfoDone(PimCommon::AccountInfo)));
    job->accountInfo();
}

void UbuntuoneStorageService::slotAccountInfoDone(const PimCommon::AccountInfo &info)
{
    Q_EMIT accountInfoDone(serviceName(), info);
}

QString UbuntuoneStorageService::name()
{
    return i18n("Ubuntu One");
}

void UbuntuoneStorageService::uploadFile(const QString &filename)
{
    //TODO
}

QString UbuntuoneStorageService::description()
{
    return QString();
}

QUrl UbuntuoneStorageService::serviceUrl()
{
    return QUrl(QLatin1String("https://one.ubuntu.com/"));
}

QString UbuntuoneStorageService::serviceName()
{
    return QLatin1String("ubuntuone");
}

void PimCommon::UbuntuoneStorageService::shareLink(const QString &root, const QString &path)
{

}
