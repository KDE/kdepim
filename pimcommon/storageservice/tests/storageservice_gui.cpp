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

#include "storageservice_gui.h"
#include "storageservice/storageservicemanager.h"
#include "storageservice/settings/storageservicesettingswidget.h"
#include <QWidget>

#include <kdebug.h>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocale>
#include <KDialog>

#include <QVBoxLayout>
#include <QToolBar>
#include <QTextEdit>
#include <QPointer>

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
    QVBoxLayout *lay = new QVBoxLayout;
    mStorageManager = new PimCommon::StorageServiceManager(this);
    QToolBar *bar = new QToolBar;
    lay->addWidget(bar);
    bar->addAction(QLatin1String("Settings..."), this, SLOT(slotSettings()));

    mEdit = new QTextEdit;
    lay->addWidget(mEdit);
    setLayout(lay);
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

