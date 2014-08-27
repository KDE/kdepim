/*
Copyright 2014  Abhijeet Nikam connect08nikam@gmail.com

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

#ifndef COMPOSER_H
#define COMPOSER_H

#include <KDateTime>
#include <KMime/Message>
#include <QObject>
#include <QTextCodec>
#include <QTextEncoder>

#include "sender/akonadisender.h"
#include "messagecomposer/helper/messagefactory.h"

#include <AkonadiCore/ItemFetchJob>
#include <kidentitymanagement/identitymanager.h>

#include "receivermodel.h"

namespace KIdentityManagement {
class IdentityManager;
}

class Composer : public QObject
{

Q_OBJECT

    Q_PROPERTY (QString subject READ subject WRITE setSubject NOTIFY subjectChanged)
    Q_PROPERTY (QString body READ body WRITE setBody NOTIFY bodyChanged)
    Q_PROPERTY (QString cc READ cc WRITE setCC NOTIFY ccChanged)
    Q_PROPERTY (QString to READ to WRITE setTo NOTIFY toChanged)
    Q_PROPERTY (QString bcc READ bcc WRITE setBCC NOTIFY bccChanged)
    Q_PROPERTY (QString from READ from WRITE setFrom NOTIFY fromChanged)
    Q_PROPERTY (ReceiverModel* receiverModel READ receiverModel CONSTANT)

public:

    explicit Composer ( QObject *parent = 0 );

    QString cc() const;
    QString bcc() const;
    QString from() const;
    QString to() const;
    QString subject() const;
    QString body() const;
    ReceiverModel *receiverModel() const;

    void setFrom( const QString &from );
    void setTo( const QString &replyTo );
    void setCC( const QString &cc );
    void setBCC( const QString &bcc );
    void setSubject( const QString &subject );
    void setBody ( const QString &body );

    void setMessage ( const KMime::Message::Ptr &message );

    QByteArray convert ( const QString &body );

signals:

    void subjectChanged();
    void bodyChanged();
    void fromChanged();
    void toChanged();
    void bccChanged();
    void ccChanged();

public slots:

    void send();
    void saveDraft();
    void sendLater();
    void addRecipient( const QString &email , int type );

    void forwardMessage (const QUrl &url);
    void replyToMessage(const QUrl &url);
    void replyToAll(const QUrl &url);
    void replyToAuthor(const QUrl &url);
    void replyToMailingList(const QUrl &url);

private slots:

    void replyFetchResult(KJob *job);
    void forwardFetchResult (KJob *job);

private:

    void reply(const QUrl &url, MessageComposer::ReplyStrategy replyStrategy, bool quoteOriginal = true);

    QString m_subject;
    QString m_from;
    QString m_body;
    QString m_cc;
    QString m_to;
    QString m_bcc;
    ReceiverModel *m_receiverModel;

    KIdentityManagement::IdentityManager *m_IdentityManager;

};





#endif
