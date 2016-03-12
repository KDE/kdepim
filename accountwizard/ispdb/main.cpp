/*
    Copyright (c) 2010 Omat Holding B.V. <info@omat.nl>

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

#include "ispdb.h"
#include "../accountwizard_debug.h"

#include <kaboutdata.h>

#include <KLocalizedString>
#include <QIcon>
#include <QApplication>
#include <KAboutData>
#include <QCommandLineParser>
#include <QCommandLineOption>

QString socketTypeToStr(Ispdb::socketType socketType)
{
    QString enc = QStringLiteral("None");
    if (socketType == Ispdb::SSL) {
        enc = QStringLiteral("SSL");
    } else if (socketType == Ispdb::StartTLS) {
        enc = QStringLiteral("TLS");
    }
    return enc;
}

QString authTypeToStr(Ispdb::authType authType)
{
    QString auth = QStringLiteral("unknown");
    switch (authType) {
    case Ispdb::Plain:
        auth = QStringLiteral("plain");
        break;
    case Ispdb::CramMD5:
        auth = QStringLiteral("CramMD5");
        break;
    case Ispdb::NTLM:
        auth = QStringLiteral("NTLM");
        break;
    case Ispdb::GSSAPI:
        auth = QStringLiteral("GSSAPI");
        break;
    case Ispdb::ClientIP:
        auth = QStringLiteral("ClientIP");
        break;
    case Ispdb::NoAuth:
        auth = QStringLiteral("NoAuth");
        break;
    }
    return auth;
}
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#if QT_VERSION >= 0x050600
    app.setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    KAboutData aboutData(QStringLiteral("ispdb"),
                         i18n("ISPDB Assistant"),
                         QStringLiteral("0.1"),
                         i18n("Helps setting up PIM accounts"),
                         KAboutLicense::LGPL,
                         i18n("(c) 2010 Omat Holding B.V."),
                         QStringLiteral("http://pim.kde.org/akonadi/"));
    aboutData.addAuthor(i18n("Tom Albers"),  i18n("Author"), QStringLiteral("toma@kde.org"));

    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("akonadi")));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QStringLiteral("email"), i18n("Tries to fetch the settings for that email address"), QStringLiteral("emailaddress")));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    QString email(QStringLiteral("blablabla@gmail.com"));
    const QString argEmail = parser.value(QStringLiteral("email"));
    if (!argEmail.isEmpty()) {
        email = argEmail;
    }

    QEventLoop loop;
    Ispdb ispdb;
    ispdb.setEmail(email);

    loop.connect(&ispdb, SIGNAL(finished(bool)), SLOT(quit()));

    ispdb.start();

    loop.exec();
    qCDebug(ACCOUNTWIZARD_LOG) << "Domains" << ispdb.relevantDomains();
    qCDebug(ACCOUNTWIZARD_LOG) << "Name" << ispdb.name(Ispdb::Long) << "(" << ispdb.name(Ispdb::Short) << ")";
    qCDebug(ACCOUNTWIZARD_LOG) << "Imap servers:";
    foreach (const Server &s, ispdb.imapServers()) {
        qCDebug(ACCOUNTWIZARD_LOG) << "\thostname:" << s.hostname
                                   << "- port:" << s.port
                                   << "- encryption:" << socketTypeToStr(s.socketType)
                                   << "- username:" << s.username
                                   << "- authentication:" << authTypeToStr(s.authentication);
    }
    qCDebug(ACCOUNTWIZARD_LOG) << "pop3 servers:";
    foreach (const Server &s, ispdb.pop3Servers()) {
        qCDebug(ACCOUNTWIZARD_LOG) << "\thostname:" << s.hostname
                                   << "- port:" << s.port
                                   << "- encryption:" << socketTypeToStr(s.socketType)
                                   << "- username:" << s.username
                                   << "- authentication:" << authTypeToStr(s.authentication);
    }
    qCDebug(ACCOUNTWIZARD_LOG) << "smtp servers:";
    foreach (const Server &s, ispdb.smtpServers()) {
        qCDebug(ACCOUNTWIZARD_LOG) << "\thostname:" << s.hostname
                                   << "- port:" << s.port
                                   << "- encryption:" << socketTypeToStr(s.socketType)
                                   << "- username:" << s.username
                                   << "- authentication:" << authTypeToStr(s.authentication);
    }

    return 0;
}
