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
#include <qapplication.h>

#include <KLocalizedString>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>

SplitterCollapserGui_test::SplitterCollapserGui_test(int indexOfWidgetAssociateToSplitterCollapser, Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent)
{
    QHBoxLayout *lay = new QHBoxLayout;
    setLayout(lay);
    QSplitter *splitter = new QSplitter(orientation);
    lay->addWidget(splitter);
    QTextEdit *rightTextEdit = new QTextEdit;
    splitter->addWidget(rightTextEdit);
    QTextEdit *leftTextEdit = new QTextEdit;
    splitter->addWidget(leftTextEdit);
    if (indexOfWidgetAssociateToSplitterCollapser == 0) {
        new PimCommon::SplitterCollapser(splitter, rightTextEdit, this);
    } else {
        new PimCommon::SplitterCollapser(splitter, leftTextEdit, this);
    }
}

SplitterCollapserGui_test::~SplitterCollapserGui_test()
{

}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    parser.process(app);

    SplitterCollapserGui_test *w = new SplitterCollapserGui_test(0, Qt::Horizontal);
    w->resize(800, 600);
    w->show();

    SplitterCollapserGui_test *w2 = new SplitterCollapserGui_test(1, Qt::Horizontal);
    w2->resize(800, 600);
    w2->show();

    SplitterCollapserGui_test *w3 = new SplitterCollapserGui_test(0, Qt::Vertical);
    w3->resize(800, 600);
    w3->show();

    SplitterCollapserGui_test *w4 = new SplitterCollapserGui_test(1, Qt::Vertical);
    w4->resize(800, 600);
    w4->show();

    app.exec();
    delete w;
    delete w2;
    delete w3;
    delete w4;
    return 0;
}
