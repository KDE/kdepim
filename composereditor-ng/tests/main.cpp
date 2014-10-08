/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include "composerhtmleditor.h"


#include <KAboutData>

#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

int main(int argc, char **argv)
{
    const QByteArray ba = QByteArray("composerhtmleditor");
    const QString name = i18n("KDE HTML Editor");
    KAboutData aboutData(ba, ba, name, ba, name);
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    ComposerHtmlEditor *mw = new ComposerHtmlEditor();
    mw->show();
    app.exec();
}
