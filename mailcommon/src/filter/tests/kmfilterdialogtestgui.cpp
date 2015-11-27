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

#include "kmfilterdialogtestgui.h"
#include "dummykernel.h"
#include "../kernel/mailkernel.h"
#include "../kmfilterdialog.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    DummyKernel *kernel = new DummyKernel(0);
    CommonKernel->registerKernelIf(kernel);   //register KernelIf early, it is used by the Filter classes
    CommonKernel->registerSettingsIf(kernel);   //SettingsIf is used in FolderTreeWidget

    QList<KActionCollection *> lstAction;
    MailCommon::KMFilterDialog *dlg = new MailCommon::KMFilterDialog(lstAction, Q_NULLPTR, true);
    dlg->resize(800, 600);
    dlg->show();
    const int ret = app.exec();
    return ret;
}
