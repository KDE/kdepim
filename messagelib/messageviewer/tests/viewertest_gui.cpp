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

#include "messageviewer/viewer.h"
#include "messageviewer/headerstylepluginmanager.h"
#include <KAboutData>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include "messageviewer_debug.h"

using namespace MessageViewer;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);

    KAboutData aboutData(QStringLiteral("viewertest_gui"), i18n("Viewertest_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for MessageViewer"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("+[file]"), i18n("File containing an email")));
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("headerstyleplugin"), i18n("Header Style Plugin"), QStringLiteral("headerstyleplugin")));

    QCommandLineOption pluginlistnameOption(QStringList() << QStringLiteral("list"), QStringLiteral("Show list of plugin installed."));
    parser.addOption(pluginlistnameOption);

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (parser.isSet(pluginlistnameOption)) {
        qDebug() << "List of Plugin :" << MessageViewer::HeaderStylePluginManager::self()->pluginListName();
        return 0;
    }

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
            qCWarning(MESSAGEVIEWER_LOG) << "Couldn't read" << fileName;
        }
    }
    msg->parse();

    Viewer *viewer = new Viewer(0);
    viewer->setPluginName(parser.value(QStringLiteral("headerstyleplugin")));
    viewer->setMessage(KMime::Message::Ptr(msg));

    viewer->show();

    return app.exec();
}
