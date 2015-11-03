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
#include <QStandardPaths>
#include "../widgets/gravatarupdatedialog.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QStandardPaths::setTestModeEnabled(true);
    KAboutData aboutData(QStringLiteral("gravatarupdatedialog_gui"), i18n("Gravatarupdatedialog_Gui"), QStringLiteral("1.0"));
    aboutData.setShortDescription(i18n("Test for Gravatar update dialog"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("email"), i18n("Email address"), QStringLiteral("emailaddress")));

    parser.process(app);
    aboutData.processCommandLine(&parser);
    QString email;
    if (!parser.value(QStringLiteral("email")).isEmpty()) {
        email = parser.value(QStringLiteral("email"));
    }

    KABGravatar::GravatarUpdateDialog dlg;
    dlg.setEmail(email);
    dlg.show();

    app.exec();
    return 0;
}
