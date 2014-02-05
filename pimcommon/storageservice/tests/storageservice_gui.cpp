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

#include "storageservice_gui.h"
#include "storageservice/storageservicemanager.h"
#include "storageservice/settings/storageservicesettingswidget.h"
#include "pimcommon/storageservice/tests/testsettingsjob.h"
#include "pimcommon/storageservice/storageservicejobconfig.h"
#include <QWidget>

#include <kdebug.h>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocalizedString>
#include <KDialog>
#include <KActionMenu>

#include <QVBoxLayout>
#include <QToolBar>
#include <QTextEdit>
#include <QPointer>
#include <QMenu>

StorageServiceSettingsDialog::StorageServiceSettingsDialog(QWidget *parent)
    : KDialog(parent)
{
    setButtons(Ok|Cancel);
    mSettings = new PimCommon::StorageServiceSettingsWidget;
    setMainWidget(mSettings);
}

QMap<QString, PimCommon::StorageServiceAbstract *> StorageServiceSettingsDialog::listService() const
{
    return mSettings->listService();
}

void StorageServiceSettingsDialog::setListService(const QMap<QString, PimCommon::StorageServiceAbstract *> &lst)
{
    mSettings->setListService(lst);
}

StorageServiceTestWidget::StorageServiceTestWidget(QWidget *parent)
    : QWidget(parent)
{
    PimCommon::TestSettingsJob *settingsJob = new PimCommon::TestSettingsJob(this);
    PimCommon::StorageServiceJobConfig *configJob = PimCommon::StorageServiceJobConfig::self();
    configJob->registerConfigIf(settingsJob);

    QVBoxLayout *lay = new QVBoxLayout;
    mStorageManager = new PimCommon::StorageServiceManager(this);
    connect(mStorageManager, SIGNAL(uploadFileDone(QString,QString)), this, SLOT(slotUploadFileDone(QString,QString)));
    connect(mStorageManager, SIGNAL(uploadDownloadFileProgress(QString,qint64,qint64)), this, SLOT(slotuploadDownloadFileProgress(QString,qint64,qint64)));
    connect(mStorageManager, SIGNAL(shareLinkDone(QString,QString)), this, SLOT(slotShareLinkDone(QString,QString)));
    connect(mStorageManager, SIGNAL(authenticationDone(QString)), this, SLOT(slotAuthenticationDone(QString)));
    connect(mStorageManager, SIGNAL(authenticationFailed(QString,QString)), this, SLOT(slotAuthenticationFailed(QString,QString)));
    connect(mStorageManager, SIGNAL(actionFailed(QString,QString)), this, SLOT(slotActionFailed(QString,QString)));
    QToolBar *bar = new QToolBar;
    lay->addWidget(bar);
    bar->addAction(QLatin1String("Settings..."), this, SLOT(slotSettings()));
    QAction *act = bar->addAction(QLatin1String("service menu"));
    QMenu *menu = new QMenu(QLatin1String("Service"));
    connect(menu, SIGNAL(aboutToShow()), this, SLOT(slotServiceMenu()));
    act->setMenu(menu);

    mEdit = new QTextEdit;
    mEdit->setReadOnly(true);
    lay->addWidget(mEdit);
    setLayout(lay);
}

void StorageServiceTestWidget::slotActionFailed(const QString &serviceName, const QString &error)
{
    mEdit->insertPlainText(QString::fromLatin1("action failed service name: %1 , error :%2\n").arg(serviceName).arg(error));
}

void StorageServiceTestWidget::slotAuthenticationFailed(const QString &serviceName, const QString &error)
{
    mEdit->insertPlainText(QString::fromLatin1("authentication failed service name: %1 , error :%2\n").arg(serviceName).arg(error));
}

void StorageServiceTestWidget::slotAuthenticationDone(const QString &serviceName)
{
    mEdit->insertPlainText(QString::fromLatin1("authentication done service name: %1\n").arg(serviceName));
}

void StorageServiceTestWidget::slotShareLinkDone(const QString &serviceName, const QString &link)
{
    mEdit->insertPlainText(QString::fromLatin1("service name: %1, link %2\n").arg(serviceName).arg(link));
}

void StorageServiceTestWidget::slotuploadDownloadFileProgress(const QString &serviceName, qint64 done, qint64 total)
{
    mEdit->insertPlainText(QString::fromLatin1("service name: %1, upload done: %2 on %3\n").arg(serviceName).arg(done).arg(total));
}

void StorageServiceTestWidget::slotUploadFileDone(const QString &serviceName, const QString &fileName)
{
    mEdit->insertPlainText(QString::fromLatin1("download done on %1. Filename: %2\n").arg(serviceName).arg(fileName));
}

void StorageServiceTestWidget::slotServiceMenu()
{
    QMenu *menu = qobject_cast<QMenu*>(sender());
    if (menu) {
        menu->clear();
        menu->addAction(mStorageManager->menuUploadServices(menu));
        menu->addAction(mStorageManager->menuDownloadServices(menu));
        menu->addAction(mStorageManager->menuShareLinkServices(menu));
        QList<PimCommon::StorageServiceAbstract::Capability> lst;
        lst <<PimCommon::StorageServiceAbstract::DeleteFileCapability;
        menu->addAction(mStorageManager->menuWithCapability(PimCommon::StorageServiceAbstract::DeleteFileCapability, lst, menu));
    }
}

void StorageServiceTestWidget::slotSettings()
{
    QPointer<StorageServiceSettingsDialog> dlg = new StorageServiceSettingsDialog(this);
    dlg->setListService(mStorageManager->listService());
    if (dlg->exec()) {
        mStorageManager->setListService(dlg->listService());
    }
    delete dlg;
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "storageservice_gui", 0, ki18n("storageservice_Gui"),
                       "1.0", ki18n("Test for storageservice"));

    KApplication app;
    StorageServiceTestWidget *w = new StorageServiceTestWidget;
    w->show();
    return app.exec();
}

