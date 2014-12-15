/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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


#ifndef IsGdShortUrl_H
#define IsGdShortUrl_H
#include "shorturl/abstractshorturl.h"

namespace PimCommon
{
class IsGdShortUrl : public PimCommon::AbstractShortUrl
{
    Q_OBJECT
public:
    explicit IsGdShortUrl(QObject *parent = Q_NULLPTR);
    ~IsGdShortUrl();

    void start() Q_DECL_OVERRIDE;
    QString shortUrlName() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotShortUrlFinished(QNetworkReply *reply) Q_DECL_OVERRIDE;
    void slotSslErrors(QNetworkReply *, const QList<QSslError> &error);
};
}

#endif // IsGdShortUrl_H
