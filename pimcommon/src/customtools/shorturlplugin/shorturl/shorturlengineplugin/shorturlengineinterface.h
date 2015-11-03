/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef SHORTURLENGINEINTERFACE_H
#define SHORTURLENGINEINTERFACE_H

#include <QObject>
#include "pimcommon_export.h"
#include <QNetworkReply>
class QNetworkAccessManager;
namespace PimCommon
{
class ShortUrlEnginePlugin;
class PIMCOMMON_EXPORT ShortUrlEngineInterface : public QObject
{
    Q_OBJECT
public:
    explicit ShortUrlEngineInterface(ShortUrlEnginePlugin *plugin, QObject *parent = Q_NULLPTR);
    ~ShortUrlEngineInterface();

    void setShortUrl(const QString &url);
    virtual void generateShortUrl() = 0;
    virtual QString engineName() const = 0;
    QString pluginName() const;

protected Q_SLOTS:
    void slotErrorFound(QNetworkReply::NetworkError error);

Q_SIGNALS:
    void shortUrlGenerated(const QString &url);
    void shortUrlFailed(const QString &error);

protected:
    QString mOriginalUrl;
    bool mErrorFound;
    QNetworkAccessManager *mNetworkAccessManager;
    PimCommon::ShortUrlEnginePlugin *mEnginePlugin;
};
}
#endif // SHORTURLENGINEINTERFACE_H
