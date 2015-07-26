/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/
#include <QWidget>
#include <QVBoxLayout>

#include <klocalizedstring.h>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include "composer/composerlineedit.h"

using namespace MessageComposer;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("testcomposerlineedit"), i18n("ComposerLineEdit"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("composerlineedit test app"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(w);

    ComposerLineEdit *kale1 = new ComposerLineEdit(0);
    // Add menu for completion
    kale1->enableCompletion(true);
    vbox->addWidget(kale1);
    ComposerLineEdit *kale2 = new ComposerLineEdit(0);
    vbox->addWidget(kale2);
    vbox->addStretch();

    w->resize(400, 400);
    w->show();

    return app.exec();
}

