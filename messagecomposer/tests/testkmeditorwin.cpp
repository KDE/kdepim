/*
    Copyright (C) 2007 Laurent Montel <montel@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <messagecomposer/composer/kmeditor.h>
#include <KApplication>
#include <kcmdlineargs.h>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <testkmeditorwin.h>

testKMeditorWindow::testKMeditorWindow()
{
  setWindowTitle(QLatin1String("test kmeditor window"));
  editor = new KMeditor;
  editor->setAcceptRichText(false);
  setCentralWidget(editor);

  QMenu *editMenu = menuBar()->addMenu(tr("Edit"));

  QAction *act = new QAction(tr("Paste as quote"), this);
  connect(act, SIGNAL(triggered()), editor, SLOT(slotPasteAsQuotation()));
  editMenu->addAction(act);

  act = new QAction(tr("Remove  quote"), this);
  connect(act, SIGNAL(triggered()), editor, SLOT(slotRemoveQuotes()));
  editMenu->addAction(act);

  act = new QAction(tr("Add quote"), this);
  connect(act, SIGNAL(triggered()), editor, SLOT(slotAddQuotes()));
  editMenu->addAction(act);

}

testKMeditorWindow::~testKMeditorWindow()
{
}


int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, "testkmeditorwin", 0, ki18n("KMeditorTestWin"), "1.0" , ki18n("kmeditor test win app"));
    KApplication app;
    testKMeditorWindow *edit = new testKMeditorWindow();
    edit->show();
    return app.exec();
}

