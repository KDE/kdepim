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


#ifndef DROPBOXTOKEN_H
#define DROPBOXTOKEN_H

#include <QObject>
#include <QNetworkReply>
class QNetworkAccessManager;
namespace PimCommon {
class DropBoxToken : public QObject
{
    Q_OBJECT
public:
    explicit DropBoxToken(QObject *parent=0);
    ~DropBoxToken();

    void getTokenAccess();

private Q_SLOTS:
    void slotError(QNetworkReply::NetworkError /*error*/);    
    void slotSendDataFinished(QNetworkReply *);

private:
    QNetworkAccessManager *mNetworkAccessManager;
    QString nonce;
    QString mAppKey;
    QString mAppSecret;
    QString mOauthVersion;
    QString mOauthSignatureMethod;
    QString mTimestamp;
    //QString mNonce;
    QString mOauthTokenSecret;
    QString mAccessOauthSignature;
};
}

#endif // DROPBOXTOKEN_H
