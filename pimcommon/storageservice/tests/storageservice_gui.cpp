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
#include "storageservice/settings/storageservicesettingswidget.h"
#include <QWidget>

#include <kdebug.h>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocale>

#include <QVBoxLayout>
#include <QToolBar>
#include <QTextEdit>

StorageServiceTestWidget::StorageServiceTestWidget(QWidget *parent)
    : QWidget(parent)
{
    mEdit = new QTextEdit;
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "storageservice_gui", 0, ki18n("storageservice_Gui"),
                       "1.0", ki18n("Test for storageservice"));

    KApplication app;
    PimCommon::StorageServiceSettingsWidget *w = new PimCommon::StorageServiceSettingsWidget;
    w->show();
    return app.exec();
}

