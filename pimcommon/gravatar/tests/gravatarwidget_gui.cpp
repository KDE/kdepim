/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#include <KLocalizedString>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include "gravatar/widgets/gravatardownloadpixmapwidget.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    KAboutData aboutData( QLatin1String("gravatar_gui"), i18n("GravatarTest_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for gravatar widget"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);


    PimCommon::GravatarDownloadPixmapWidget *w = new PimCommon::GravatarDownloadPixmapWidget;
    w->show();
    int ret = app.exec();
    delete w;
    return ret;
}

