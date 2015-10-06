/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef TINYURLSHORTURL_H
#define TINYURLSHORTURL_H
#include "shorturl/abstractshorturl.h"

namespace PimCommon
{
class TinyurlShortUrl : public PimCommon::AbstractShortUrl
{
    Q_OBJECT
public:
    explicit TinyurlShortUrl(QObject *parent = Q_NULLPTR);
    ~TinyurlShortUrl();

    void start() Q_DECL_OVERRIDE;

    QString shortUrlName() const Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotShortUrlFinished(QNetworkReply *reply) Q_DECL_OVERRIDE;
};
}

#endif // TINYURLSHORTURL_H
