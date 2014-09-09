/*
    This file is part of KSendEmail.
    Copyright (c) 2008 Pradeepto Bhattacharya <pradeepto@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mailerservice.h"

#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <KLocalizedString>

static const char description[] =
    I18N_NOOP("KDE Command Line Emailer.");

int main(int argc, char **argv)
{
    KAboutData aboutData(QStringLiteral("ksendemail"), i18n("KSendEmail"), QStringLiteral("0.01"), i18n(description),
                         KAboutLicense::GPL,
                         i18n("(C) 2008 Pradeepto Bhattacharya"),
                         QStringLiteral("http://kontact.kde.org"));

    aboutData.addAuthor(i18n("Pradeepto Bhattacharya"), QString(), QStringLiteral("pradeepto@kde.org"));

    QCommandLineParser parser;
    QApplication app(argc, argv);
    parser.addVersionOption();
    parser.addHelpOption();
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("s") << QLatin1String("subject"), i18n("Set subject of message"), QLatin1String("subject")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("c") << QLatin1String("cc"), i18n("Send CC: to 'address'"), QLatin1String("address")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("b") << QLatin1String("bcc"), i18n("Send BCC: to 'address'"), QLatin1String("address")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("h") << QLatin1String("header"), i18n("Add 'header' to message"), QLatin1String("header")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("msg"), i18n("Read message body from 'file'"), QLatin1String("file")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("body"), i18n("Set body of message"), QLatin1String("text")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("attach"), i18n("Add an attachment to the mail. This can be repeated"), QLatin1String("url")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("composer"), i18n("Only open composer window")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("+[address]"), i18n("Address to send the message to")));

    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    MailerService *ms = new MailerService();
    ms->processArgs(parser);

    delete ms;
    return 0;
}
