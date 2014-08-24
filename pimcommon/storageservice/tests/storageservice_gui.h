/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#ifndef TEST_STORAGESERVICE_GUI_H
#define TEST_STORAGESERVICE_GUI_H

#include <QWidget>
#include <QDialog>
class QTextEdit;
namespace PimCommon {
class StorageServiceManager;
class StorageServiceSettingsWidget;
class StorageServiceAbstract;
}

class StorageServiceSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StorageServiceSettingsDialog(QWidget *parent=0);
    QMap<QString, PimCommon::StorageServiceAbstract *> listService() const;
    void setListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &lst);

private:
    PimCommon::StorageServiceSettingsWidget *mSettings;
};

class StorageServiceTestWidget : public QWidget
{
    Q_OBJECT
public:
    explicit StorageServiceTestWidget(QWidget *parent=0);

private slots:
    void slotSettings();

    void slotServiceMenu();
    void slotUploadFileDone(const QString &serviceName, const QString &fileName);
    void slotuploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total);
    void slotShareLinkDone(const QString &serviceName, const QString &link);
    void slotAuthenticationFailed(const QString &serviceName, const QString &error);
    void slotAuthenticationDone(const QString &serviceName);
    void slotActionFailed(const QString &serviceName, const QString &error);
private:
    QTextEdit *mEdit;
    PimCommon::StorageServiceManager *mStorageManager;
};

#endif
