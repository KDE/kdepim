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

#include "pimcommon/texteditor/richtexteditor/richtexteditorwidget.h"

#include <kdebug.h>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocale>

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "richtexteditor_gui", 0, ki18n("RichTextEditorTest_Gui"),
                       "1.0", ki18n("Test for richtexteditor widget"));

    KApplication app;

    PimCommon::RichTextEditorWidget *w = new PimCommon::RichTextEditorWidget();
    w->resize(800, 600);
    w->show();
    app.exec();
    delete w;
    return 0;
}

