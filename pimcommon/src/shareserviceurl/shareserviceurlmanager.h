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

#ifndef SHARESERVICEURLMANAGER_H
#define SHARESERVICEURLMANAGER_H

#include <QObject>
#include "pimcommon_export.h"
#include <QUrl>

class KActionMenu;
class QAction;
namespace PimCommon
{
class ShareServiceUrlManagerPrivate;
class PIMCOMMON_EXPORT ShareServiceUrlManager : public QObject
{
    Q_OBJECT
public:
    explicit ShareServiceUrlManager(QObject *parent = Q_NULLPTR);
    ~ShareServiceUrlManager();
    enum ServiceType {
        Fbook = 0,
        Twitter,
        GooglePlus,
        MailTo,
        LinkedIn,
        Evernote,
        Pocket,
        LiveJournal,
        ServiceEndType
    };

    KActionMenu *menu() const;

    QUrl generateServiceUrl(const QString &link, const QString &title, ServiceType type);

    void openUrl(const QUrl &url);

public Q_SLOTS:
    void slotSelectServiceUrl(QAction *act);

Q_SIGNALS:
    void serviceUrlSelected(PimCommon::ShareServiceUrlManager::ServiceType type);

private:
    ShareServiceUrlManagerPrivate *const d;
};
}
Q_DECLARE_METATYPE(PimCommon::ShareServiceUrlManager::ServiceType)
#endif // SHARESERVICEURLMANAGER_H
