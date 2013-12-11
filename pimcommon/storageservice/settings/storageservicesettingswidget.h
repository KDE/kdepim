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

#ifndef STORAGESERVICESETTINGSWIDGET_H
#define STORAGESERVICESETTINGSWIDGET_H

#include <QWidget>
#include <QMap>
#include "pimcommon_export.h"
#include "storageservice/storageserviceabstract.h"
#include "storageservice/storageservicemanager.h"
class QListWidget;
class QPushButton;
class QLabel;
class KTextBrowser;
namespace PimCommon {
class StorageListWidgetItem;

class PIMCOMMON_EXPORT StorageServiceSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StorageServiceSettingsWidget(QWidget *parent=0);
    ~StorageServiceSettingsWidget();

    void setListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &lst);
    QMap<QString, PimCommon::StorageServiceAbstract *> listService() const;

private slots:
    void slotServiceSelected();

    void slotAddService();
    void slotRemoveService();
    void slotUpdateAccountInfo(const QString &serviceName, const PimCommon::AccountInfo &info);
    void slotModifyService();
    void slotAuthentificationFailed(const QString &serviceName, const QString &error);
    void slotAuthentificationDone(const QString &serviceName);

private:
    void updateButtons();
    PimCommon::StorageListWidgetItem *createItem(const QString &serviceName, const QString &service, PimCommon::StorageServiceManager::ServiceType type, const KIcon &icon);
    enum ServiceData {
        Name = Qt::UserRole + 1,
        Type = Qt::UserRole + 2
    };
    QMap<QString, PimCommon::StorageServiceAbstract *> mListStorageService;
    QListWidget *mListService;
    KTextBrowser *mDescription;
    QPushButton *mAddService;
    QPushButton *mRemoveService;
    QPushButton *mModifyService;
    QLabel *mAccountSize;
    QLabel *mQuota;
    QLabel *mShared;
};
}
#endif // STORAGESERVICESETTINGSWIDGET_H
