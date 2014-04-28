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

#include "dropboxtoken_gui.h"
#include "../dropboxstorageservice.h"

#include <QWidget>

#include <qdebug.h>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocalizedString>


DropboxTestWidget::DropboxTestWidget(QWidget *parent)
    : ServiceTestWidget(parent)
{
    setStorageService(new PimCommon::DropBoxStorageService(this));
}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "dropboxToken_gui", 0, ki18n("dropboxToken_Gui"),
                       "1.0", ki18n("Test for short dropboxtoken"));

    KApplication app;

    DropboxTestWidget *w = new DropboxTestWidget;
    w->show();
    return app.exec();
}

