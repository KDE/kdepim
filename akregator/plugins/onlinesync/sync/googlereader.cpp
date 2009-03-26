/*
    This file is part of Akregator.

    Copyright (C) 2008 Didier Hoarau <did.hoarau@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "googlereader.h"

#include <KConfigGroup>
#include <kdebug.h>
#include <KLocalizedString>
#include <kio/job.h>

#include <QDomDocument>


namespace feedsync
{

GoogleReader::GoogleReader(const KConfigGroup& configgroup, QObject* parent) : Aggregator( parent )
{
    kDebug();
    setUser(configgroup.readEntry("Login"));
    setPassword(configgroup.readEntry("Password"));
    _cursor=0;
}

GoogleReader::~GoogleReader() 
{
    kDebug();
}

void GoogleReader::load() 
{
    kDebug();

    // Now: Authentication

    // Data
    QByteArray data;
    data.append(QString(QString("Email=")+getUser()+QString("&Passwd=")+getPassword()).toUtf8());

    KIO::StoredTransferJob *job = KIO::storedHttpPost(data, KUrl("https://www.google.com/accounts/ClientLogin"));
    job->addMetaData("content-type", "application/x-www-form-urlencoded");
    job->addMetaData("cookies", "manual");
    connect(job, SIGNAL(result(KJob*)), SLOT(slotAuthenticationDone(KJob*)));
}

void GoogleReader::add(const SubscriptionList & list) 
{
    // End
    if (_cursor==list.count()) {
        _cursor=0;
        // Emit signal
        emit addDone();
        return;
    }
    _cursorList = list;

    QByteArray data;

    // Add new: if not in the know list of feed and if not added before
    if ( (getSubscriptionList().indexOf(list.getRss(_cursor))<0) && (list.indexOf(list.getRss(_cursor))>=_cursor) ) {
        kDebug() << "New";
        data.append(QString(    QString("s=feed/")+list.getRss(_cursor)
                                +QString("&ac=subscribe")
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());
        if (!list.getCat(_cursor).isEmpty()) {
            data.append(QString(QString("&a=user/-/label/")+list.getCat(_cursor,SubscriptionList::Simple)).toUtf8());
        }

    // Add tag
    } else {
        kDebug() << "Add Tag";
        data.append(QString(    QString("s=feed/")+list.getRss(_cursor)
                                +QString("&ac=edit")
                                +QString("&a=user/-/label/")+list.getCat(_cursor,SubscriptionList::Simple)
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());
    }

    // keep the list of feeds synchronized
    getSubscriptionList().add(list.getRss(_cursor), list.getName(_cursor), list.getCat(_cursor));

    KIO::StoredTransferJob *job =
        KIO::storedHttpPost(data, KUrl("http://www.google.com/reader/api/0/subscription/edit"));
    job->addMetaData("cookies", "manual");
    job->addMetaData("setcookies", "SID="+getSID());
    job->addMetaData("content-type", "application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotAddDone(KJob*)));
    _cursor++;
}

void GoogleReader::update(const SubscriptionList & list) 
{
    Q_UNUSED(list)
    kDebug();

    // Emit signal
    emit updateDone();
}

void GoogleReader::remove(const SubscriptionList & list) 
{
    // End
    if (_cursor==list.count()) {
        _cursor=0;
        // Emit signal
        emit removeDone();
        return;
    }
    _cursorList = list;

    QByteArray data;

    // Revove: if all as to be removed and if this is the first occurrence
    if ( (getSubscriptionList().countRss(list.getRss(_cursor))==list.countRss(list.getRss(_cursor))) && (list.indexOf(list.getRss(_cursor))==_cursor) ) {
        kDebug() << "Remove";
        data.append(QString(    QString("s=feed/")+list.getRss(_cursor)
                                +QString("&ac=unsubscribe")
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());

    // Remove tag
    } else {
        kDebug() << "Remove Tag";
        data.append(QString(    QString("s=feed/")+list.getRss(_cursor)
                                +QString("&ac=edit")
                                +QString("&r=user/-/label/")+list.getCat(_cursor,SubscriptionList::Simple)
                                +QString("&T=")+getToken()
                                +QString("&client=contact:")+getUser()
                    ).toUtf8());
    }

    KIO::StoredTransferJob *job =
        KIO::storedHttpPost(data, KUrl("http://www.google.com/reader/api/0/subscription/edit"));
    job->addMetaData("cookies", "manual");
    job->addMetaData("setcookies", "SID="+getSID());
    job->addMetaData("content-type", "application/x-www-form-urlencoded");
    connect(job, SIGNAL(result(KJob*)), this, SLOT(slotRemoveDone(KJob*)));
    _cursor++;
    kDebug();
}

void GoogleReader::genError(const QString& msg) 
{
    kDebug();

    _cursor=0;
    // Emit signal
    emit error(msg);
}

// SLOTS

void GoogleReader::slotAddDone(KJob *job_)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob*>(job_);
    QByteArray m_data = job->data();
    QString text = QString::fromLatin1(m_data.data());
    // kDebug() << text.left(20);
    kDebug() << text;
    add(_cursorList);
}

void GoogleReader::slotRemoveDone(KJob *job_)
{
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob*>(job_);
    QByteArray m_data = job->data();
    QString text = QString::fromLatin1(m_data.data());
    kDebug() << text.left(20);
    remove(_cursorList);
}

void GoogleReader::slotTokenDone(KJob *job_)
{
    kDebug();
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob*>(job_);

    // Now: read token

    QByteArray m_data = job->data();
    QString text = QString::fromLatin1(m_data.data());
    kDebug() << "Token:" << text.left(20);
    setToken(text);

    // Load finished

    emit loadDone();
}

void GoogleReader::slotListDone(KJob *job_)
{
    kDebug();
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob*>(job_);

    // Now: read the subscription list

    QByteArray m_data = job->data();
    QString text = QString::fromLatin1(m_data.data());
    QDomDocument doc("googlereader");
    doc.setContent(text);
    QDomNode nodeList = doc.documentElement().firstChild().firstChild();
    while(!nodeList.isNull()) {
        QDomNode nodeSub;
        nodeSub = nodeList.firstChild();
        QString m_rss = nodeSub.firstChild().toText().data();
        m_rss = m_rss.right(m_rss.length()-5);
        nodeSub = nodeSub.nextSibling();
        QString m_name = nodeSub.firstChild().toText().data();
        nodeSub = nodeSub.nextSibling();
        nodeSub = nodeSub.nextSibling();

        // 
        QDomNode nodeCat = nodeSub.firstChild();
        bool hasCat = false;
        while(!nodeCat.isNull()) {
            QString m_cat = nodeCat.firstChild().nextSibling().firstChild().toText().data();
            // add to sub list
            _subscriptionList.add(m_rss,m_name,m_cat);
            nodeCat = nodeCat.nextSibling();
            hasCat = true;
        }

        // If there is no categories
        if (hasCat == false) {
            // add to sub list
            _subscriptionList.add(m_rss,m_name,"");
        }

        // next sub
        nodeList = nodeList.nextSibling();
    }

    // Next: get the token
    KIO::StoredTransferJob *getjob =
        KIO::storedGet("http://www.google.com/reader/api/0/token?client=contact:"+getUser());
    getjob->addMetaData("cookies", "manual");
    getjob->addMetaData("setcookies", "SID="+getSID());
    connect(getjob, SIGNAL(result(KJob*)), SLOT(slotTokenDone(KJob*)));
}

void GoogleReader::slotAuthenticationDone(KJob *job_)
{
    kDebug();
    KIO::StoredTransferJob *job = static_cast<KIO::StoredTransferJob *>(job_);

    // Now: read authentication SID

    QByteArray m_data = job->data();
    QString text = QString::fromLatin1(m_data.data());
    // Check wrong account
    if (text.indexOf("SID=")<0) {
        this->genError(i18n("Authentication failed, synchronization aborted."));
        return;
    }
    // Extract SID
    text = text.right(text.length()-text.indexOf("SID=")-4);
    _sid = text.left(text.indexOf("\n"));
    kDebug() << "SID:" << _sid.left(10)+QString("...");

    // Next: get the list
    KIO::StoredTransferJob *getjob = KIO::storedGet(KUrl("http://www.google.com/reader/api/0/subscription/list"));
    getjob->addMetaData("cookies", "manual");
    getjob->addMetaData("setcookies", "SID="+getSID());
    connect(getjob, SIGNAL(result(KJob*)), SLOT(slotListDone(KJob*)));
}

// Getters/Setters

QString GoogleReader::getUser() const 
{
    return _user;
}

QString GoogleReader::getPassword() const 
{
    return _password;
}

SubscriptionList GoogleReader::getSubscriptionList() const 
{
    return _subscriptionList;
}

void GoogleReader::setUser(const QString& user) 
{
    _user=user;
}

void GoogleReader::setPassword(const QString& password) 
{
    _password=password;
}

QString GoogleReader::getSID() const 
{
    return _sid;
}

void GoogleReader::setToken(const QString& token) 
{
    _token=token;
}

QString GoogleReader::getToken() const 
{
    return _token;
}

}
