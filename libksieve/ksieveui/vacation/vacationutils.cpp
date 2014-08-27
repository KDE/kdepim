/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "vacationutils.h"
#include "vacationscriptextractor.h"
#include "sieve-vacation.h"
#include <KIdentityManagement/IdentityManager>
#include <KIdentityManagement/Identity>

#include <KLocalizedString>
#include <KLocale>
#include <QDate>
#include <QLocale>

using KMime::Types::AddrSpecList;

static inline QString dotstuff(QString s)     // krazy:exclude=passbyvalue
{
    if (s.startsWith(QLatin1Char('.'))) {
        return QLatin1Char('.') + s.replace(QLatin1String("\n."), QLatin1String("\n.."));
    } else {
        return s.replace(QLatin1String("\n."), QLatin1String("\n.."));
    }
}

QString KSieveUi::VacationUtils::defaultMessageText()
{
    return i18n("I am out of office till %1.\n"
                "\n"
                "In urgent cases, please contact Mrs. \"vacation replacement\"\n"
                "\n"
                "email: \"email address of vacation replacement\"\n"
                "phone: +49 711 1111 11\n"
                "fax.:  +49 711 1111 12\n"
                "\n"
                "Yours sincerely,\n"
                "-- \"enter your name and email address here\"\n",
                QLocale().toString(QDate::currentDate().addDays(1)));
}

int KSieveUi::VacationUtils::defaultNotificationInterval()
{
    return 7; // days
}

QStringList KSieveUi::VacationUtils::defaultMailAliases()
{
    QStringList sl;
    KIdentityManagement::IdentityManager manager(true);
    KIdentityManagement::IdentityManager::ConstIterator end(manager.end());
    for (KIdentityManagement::IdentityManager::ConstIterator it = manager.begin(); it != end ; ++it) {
        if (!(*it).primaryEmailAddress().isEmpty()) {
            sl.push_back((*it).primaryEmailAddress());
        }
        sl += (*it).emailAliases();
    }
    return sl;
}

bool KSieveUi::VacationUtils::defaultSendForSpam()
{
    return VacationSettings::outOfOfficeReactToSpam();
}

QString KSieveUi::VacationUtils::defaultDomainName()
{
    return VacationSettings::outOfOfficeDomain();
}

bool KSieveUi::VacationUtils::parseScript(const QString &script, QString &messageText,
        int &notificationInterval, QStringList &aliases,
        bool &sendForSpam, QString &domainName)
{
    if (script.trimmed().isEmpty()) {
        messageText = VacationUtils::defaultMessageText();
        notificationInterval = VacationUtils::defaultNotificationInterval();
        aliases = VacationUtils::defaultMailAliases();
        sendForSpam = VacationUtils::defaultSendForSpam();
        domainName = VacationUtils::defaultDomainName();
        return true;
    }

    // The trimmed() call below prevents parsing errors. The
    // slave somehow omits the last \n, which results in a lone \r at
    // the end, leading to a parse error.
    const QByteArray scriptUTF8 = script.trimmed().toUtf8();
    qDebug() << "scriptUtf8 = \"" + scriptUTF8 + "\"";
    KSieve::Parser parser(scriptUTF8.begin(),
                          scriptUTF8.begin() + scriptUTF8.length());
    VacationDataExtractor vdx;
    SpamDataExtractor sdx;
    DomainRestrictionDataExtractor drdx;
    KSieveExt::MultiScriptBuilder tsb(&vdx, &sdx, &drdx);
    parser.setScriptBuilder(&tsb);
    if (!parser.parse()) {
        return false;
    }
    messageText = vdx.messageText().trimmed();
    notificationInterval = vdx.notificationInterval();
    aliases = vdx.aliases();
    if (!VacationSettings::allowOutOfOfficeUploadButNoSettings()) {
        sendForSpam = !sdx.found();
        domainName = drdx.domainName();
    }
    return true;
}

QString KSieveUi::VacationUtils::composeScript(const QString &messageText,
        int notificationInterval,
        const AddrSpecList &addrSpecs,
        bool sendForSpam, const QString &domain)
{
    QString addressesArgument;
    QStringList aliases;
    if (!addrSpecs.empty()) {
        addressesArgument += QLatin1String(":addresses [ ");
        QStringList sl;
        AddrSpecList::const_iterator end = addrSpecs.constEnd();
        for (AddrSpecList::const_iterator it = addrSpecs.begin() ; it != end; ++it) {
            sl.push_back(QLatin1Char('"') + (*it).asString().replace(QLatin1Char('\\'), QLatin1String("\\\\")).replace(QLatin1Char('"'), QLatin1String("\\\"")) + QLatin1Char('"'));
            aliases.push_back((*it).asString());
        }
        addressesArgument += sl.join(QLatin1String(", ")) + QLatin1String(" ] ");
    }
    QString script = QString::fromLatin1("require \"vacation\";\n\n");
    if (!sendForSpam)
        script += QString::fromLatin1("if header :contains \"X-Spam-Flag\" \"YES\""
                                      " { keep; stop; }\n");  // FIXME?

    if (!domain.isEmpty()) { // FIXME
        script += QString::fromLatin1("if not address :domain :contains \"from\" \"%1\" { keep; stop; }\n").arg(domain);
    }

    script += QLatin1String("vacation ");
    script += addressesArgument;
    if (notificationInterval > 0) {
        script += QString::fromLatin1(":days %1 ").arg(notificationInterval);
    }
    script += QString::fromLatin1("text:\n");
    script += dotstuff(messageText.isEmpty() ? VacationUtils::defaultMessageText() : messageText);
    script += QString::fromLatin1("\n.\n;\n");
    return script;
}

