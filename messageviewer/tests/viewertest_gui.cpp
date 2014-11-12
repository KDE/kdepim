/* Copyright 2010 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QFile>
#include "viewer/viewer.h"
#include "header/headerstyle.h"
#include "header/headerstrategy.h"

#include <qdebug.h>

#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

using namespace MessageViewer;

int main(int argc, char **argv)
{
    KAboutData aboutData(QLatin1String("viewertest_gui"), i18n("Viewertest_Gui"), QLatin1String("1.0"));
    aboutData.setShortDescription(i18n("Test for MessageViewer"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("+[file]"), i18n("File containing an email")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("headerstrategy"), i18n("Header Strategy: [all|rich|standard|brief|custom]"), QLatin1String("strategy")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("headerstyle"), i18n("Header Style: [brief|plain|enterprise|mobile|fancy]"), QLatin1String("style")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    KMime::Message *msg = new KMime::Message;
    if (parser.positionalArguments().count() == 0) {
        QByteArray mail = "From: dfaure@example.com\n"
                          "To: kde@example.com\n"
                          "Sender: dfaure@example.com\n"
                          "MIME-Version: 1.0\n"
                          "Date: 02 Jul 2010 23:58:21 -0000\n"
                          "Subject: Akademy\n"
                          "Content-Type: text/plain\n"
                          "X-Length: 0\n"
                          "X-UID: 6161\n"
                          "\n"
                          "Hello this is a test mail\n";
        msg->setContent(mail);
    } else {
        const QString fileName = parser.positionalArguments().at(0);
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly)) {
            msg->setContent(file.readAll());
        } else {
            qWarning() << "Couldn't read" << fileName;
        }
    }
    msg->parse();

    Viewer *viewer = new Viewer(0);
    viewer->setMessage(KMime::Message::Ptr(msg));

    const QString headerStrategy = parser.value(QLatin1String("headerstrategy"));
    const QString headerStyle = parser.value(QLatin1String("headerstyle"));
    viewer->setHeaderStyleAndStrategy(HeaderStyle::create(headerStyle),
                                      HeaderStrategy::create(headerStrategy));

    viewer->show();

    return app.exec();
}
