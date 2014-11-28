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

#include "createcolorgui_test.h"
#include "prefs.h"
#include <kapplication.h>
#include <KCmdLineArgs>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <QListWidget>
#include <QDebug>

CreateColorGui_test::CreateColorGui_test(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *vbox = new QVBoxLayout;
    setLayout(vbox);
    mListWidget = new QListWidget;
    vbox->addWidget(mListWidget);
    createListWidgetItem();
}

CreateColorGui_test::~CreateColorGui_test()
{

}

void CreateColorGui_test::createListWidgetItem()
{
    EventViews::Prefs prefs;
    mListWidget->clear();
    for (int i = 0; i < 100; ++i) {
        QListWidgetItem *item = new QListWidgetItem;
        QColor defColor(0x37, 0x7A, 0xBC);
        prefs.createNewColor(defColor, i);
        item->setBackgroundColor(defColor);
        mListWidget->addItem(item);
    }
}

int main(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, "CreateColorGui_test", 0, ki18n("CreateColorGui_test"),
                       "1.0", ki18n("Test creating color"));

    KApplication app;

    CreateColorGui_test *createColor = new CreateColorGui_test;
    createColor->resize(800, 600);
    createColor->show();

    app.exec();
    return 0;
}
