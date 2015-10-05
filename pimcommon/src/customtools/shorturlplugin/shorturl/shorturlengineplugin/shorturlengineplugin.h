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

#ifndef SHORTURLENGINEPLUGIN_H
#define SHORTURLENGINEPLUGIN_H

#include "pimcommon_export.h"
#include <QObject>

namespace PimCommon
{
class ShortUrlEnginePluginPrivate;
class PIMCOMMON_EXPORT ShortUrlEnginePlugin : public QObject
{
    Q_OBJECT
public:
    explicit ShortUrlEnginePlugin(QObject *parent = Q_NULLPTR);
    ~ShortUrlEnginePlugin();
    virtual QString engineName() const = 0;
    virtual void setShortUrl(const QString &url) = 0;
    virtual void generateShortUrl() = 0;

Q_SIGNALS:
    void shortUrlGenerated(const QString &url);
    void shortUrlFailed(const QString &error);

private:
    ShortUrlEnginePluginPrivate *const d;
};
}
#endif // SHORTURLENGINEPLUGIN_H

