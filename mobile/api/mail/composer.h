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

#include <messagecomposer/sender/messagesender.h>
#include <messagecomposer/composer/composerviewbase.h>
#include <KMime/Message>
#include <QObject>

class Composer : public QObject
{

Q_OBJECT

    Q_PROPERTY (QString subject READ subject WRITE setSubject NOTIFY subjectChanged)
    Q_PROPERTY (QString body READ body WRITE setBody NOTIFY bodyChanged)
    Q_PROPERTY (QStringList cc READ cc WRITE setCC NOTIFY ccChanged)
    Q_PROPERTY (QStringList to READ to WRITE setTo NOTIFY toChanged)
    Q_PROPERTY (QStringList bcc READ bcc WRITE setBCC NOTIFY bccChanged)
    Q_PROPERTY (QString from READ from WRITE setFrom NOTIFY fromChanged)

public:

    QStringList cc() const;
    QStringList bcc() const;
    QString from() const;
    QStringList to() const;
    QString subject() const;
    QString body() const;

    void setFrom( const QString &from );
    void setTo( const QStringList &replyTo );
    void setCC( const QStringList &cc );
    void setBCC( const QStringList &bcc );
    void setSubject( const QString &subject );
    void setBody ( const QString &body );

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

private slots:


private:

    QString m_subject;
    QString m_from;
    QString m_body;
    QStringList m_cc;
    QStringList m_to;
    QStringList m_bcc;


};





#endif