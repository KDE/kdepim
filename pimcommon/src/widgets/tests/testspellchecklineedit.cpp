/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#include <QWidget>
#include <QVBoxLayout>

#include <KAboutData>

#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "../src/widgets/spellchecklineedit.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("testspellchecklineedit"), i18n("Test SpellCheckLineEdit"), QStringLiteral("0.1"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(w);

    PimCommon::SpellCheckLineEdit *spellCheckLineEdit = new PimCommon::SpellCheckLineEdit(w, QStringLiteral("testspecklineeditrc"));
    vbox->addWidget(spellCheckLineEdit);
    vbox->addStretch();

    w->resize(400, 400);
    w->show();

    return app.exec();

}

