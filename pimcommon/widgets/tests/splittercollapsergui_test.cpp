/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "splittercollapsergui_test.h"
#include "widgets/splittercollapser.h"
#include <QSplitter>
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocalizedString>
#include <QTextEdit>
#include <QHBoxLayout>

SplitterCollapserGui_test::SplitterCollapserGui_test(QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    setLayout(lay);
    QSplitter *splitter = new QSplitter;
    lay->addWidget(splitter);
    QTextEdit *rightTextEdit = new QTextEdit;
    splitter->addWidget(rightTextEdit);
    QTextEdit *leftTextEdit = new QTextEdit;
    splitter->addWidget(leftTextEdit);
    PimCommon::SplitterCollapser *collapser = new PimCommon::SplitterCollapser(splitter, leftTextEdit);
}

SplitterCollapserGui_test::~SplitterCollapserGui_test()
{

}

int main (int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "splittercollapser_gui", 0, ki18n("SplitterCollapser_Gui"),
                       "1.0", ki18n("Test for splitter collapser widget"));

    KApplication app;

    SplitterCollapserGui_test *w = new SplitterCollapserGui_test;
    w->resize(800, 600);
    w->show();
    app.exec();
    delete w;
    return 0;
}
