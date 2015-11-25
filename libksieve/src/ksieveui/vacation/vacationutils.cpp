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

#include "vacationutils.h"
#include "vacationscriptextractor.h"
#include "legacy/vacationutils.h"
#include "sieve-vacation.h"
#include <KIdentityManagement/IdentityManager>
#include <KIdentityManagement/Identity>

#include <KLocalizedString>
#include <QDate>
#include <QLocale>

using KMime::Types::AddrSpecList;
using namespace KSieveUi;

static inline QString dotstuff(QString s)     // krazy:exclude=passbyvalue
{
    if (s.startsWith(QLatin1Char('.'))) {
        return QLatin1Char('.') + s.replace(QLatin1String("\n."), QStringLiteral("\n.."));
    } else {
        return s.replace(QLatin1String("\n."), QStringLiteral("\n.."));
    }
}

static inline QString stringReplace(QString s)
{
    s = s.replace(QRegExp(QLatin1String("[\n\t]+")), QStringLiteral(" "));
    return s.replace(QLatin1Char('\"'), QStringLiteral("\\\""));
}

QString VacationUtils::defaultSubject()
{
    return i18n("Out of office till %1", QLocale().toString(QDate::currentDate().addDays(1)));
}

QString KSieveUi::VacationUtils::mailAction(KSieveUi::VacationUtils::MailAction action)
{
    switch (action) {
    case Keep:
        return i18n("Keep");
    case Discard:
        return i18n("Discard");
    case Sendto:
        return i18n("Redirect to");
    case CopyTo:
        return i18n("Copy to");
    default:
        qCWarning(LIBKSIEVE_LOG) << "Unknown mail action" << action;
        return i18n("Unknown action");
    }
}

KSieveUi::VacationUtils::MailAction KSieveUi::VacationUtils::defaultMailAction()
{
    return KSieveUi::VacationUtils::Keep;
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

int VacationUtils::defaultNotificationInterval()
{
    return 7; // days
}

KMime::Types::AddrSpecList VacationUtils::defaultMailAliases()
{
    KMime::Types::AddrSpecList sl;
    KIdentityManagement::IdentityManager manager(true);
    KIdentityManagement::IdentityManager::ConstIterator end(manager.end());
    for (KIdentityManagement::IdentityManager::ConstIterator it = manager.begin(); it != end; ++it) {
        if (!(*it).primaryEmailAddress().isEmpty()) {
            KMime::Types::Mailbox a;
            a.fromUnicodeString((*it).primaryEmailAddress());
            sl.push_back(a.addrSpec());
        }
        foreach (const QString &email, (*it).emailAliases()) {
            KMime::Types::Mailbox a;
            a.fromUnicodeString(email);
            sl.push_back(a.addrSpec());
        }
    }

    return sl;
}

bool VacationUtils::defaultSendForSpam()
{
    return VacationSettings::outOfOfficeReactToSpam();
}

QString VacationUtils::defaultDomainName()
{
    return VacationSettings::outOfOfficeDomain();
}

QDate VacationUtils::defaultStartDate()
{
    return QDate::currentDate();
}

QDate VacationUtils::defaultEndDate()
{
    return defaultStartDate().addDays(7);
}

//TODO: delete this function after 06.2016 until than all vacation scripts should be updated
VacationUtils::Vacation parseScriptLegacy(const QString &script)
{
    KSieveUi::VacationUtils::Vacation vacation;
    vacation.active = true;
    vacation.valid =  Legacy::VacationUtils::parseScript(script, vacation.messageText,
                      vacation.subject,
                      vacation.notificationInterval, vacation.aliases,
                      vacation.sendForSpam, vacation.excludeDomain,
                      vacation.startDate, vacation.endDate);
    return vacation;
}

VacationUtils::Vacation VacationUtils::parseScript(const QString &script)
{
    KSieveUi::VacationUtils::Vacation vacation;
    if (script.trimmed().isEmpty()) {
        vacation.valid = false;
        vacation.active = false;
        vacation.mailAction = VacationUtils::defaultMailAction();
        vacation.messageText = VacationUtils::defaultMessageText();
        vacation.subject = VacationUtils::defaultSubject();
        vacation.notificationInterval = VacationUtils::defaultNotificationInterval();
        vacation.aliases = VacationUtils::defaultMailAliases();
        vacation.sendForSpam = VacationUtils::defaultSendForSpam();
        vacation.excludeDomain = VacationUtils::defaultDomainName();
        return vacation;
    }

    // The trimmed() call below prevents parsing errors. The
    // slave somehow omits the last \n, which results in a lone \r at
    // the end, leading to a parse error.
    const QByteArray scriptUTF8 = script.trimmed().toUtf8();
    qCDebug(LIBKSIEVE_LOG) << "scriptUtf8 = \"" + scriptUTF8 + "\"";
    KSieve::Parser parser(scriptUTF8.begin(),
                          scriptUTF8.begin() + scriptUTF8.length());
    VacationDataExtractor vdx;
    SpamDataExtractor sdx;
    DomainRestrictionDataExtractor drdx;
    DateExtractor dx;
    KSieveExt::MultiScriptBuilder tsb(&vdx, &sdx, &drdx, &dx);
    parser.setScriptBuilder(&tsb);
    if (!parser.parse() || !vdx.commandFound()) {
        const auto vac = parseScriptLegacy(script);
        if (vac.isValid()) {
            return vac;
        }
        vacation.active = false;
        vacation.valid = false;
        return vacation;
    }
    vacation.valid = true;
    vacation.active = vdx.active();
    vacation.mailAction = vdx.mailAction();
    vacation.mailActionRecipient = vdx.mailActionRecipient();
    vacation.messageText = vdx.messageText().trimmed();
    if (!vdx.subject().isEmpty()) {
        vacation.subject = vdx.subject().trimmed();
    }
    vacation.notificationInterval = vdx.notificationInterval();
    vacation.aliases = KMime::Types::AddrSpecList();
    foreach (const QString &alias, vdx.aliases()) {
        KMime::Types::Mailbox a;
        a.fromUnicodeString(alias);
        vacation.aliases.append(a.addrSpec());
    }

    if (!vacation.active && !vdx.ifComment().isEmpty()) {
        const QByteArray newScript = QByteArrayLiteral("if ") + vdx.ifComment().toUtf8() + QByteArrayLiteral("{vacation;}");
        tsb = KSieveExt::MultiScriptBuilder(&sdx, &drdx, &dx);
        KSieve::Parser parser(newScript.begin(),
                              newScript.begin() + newScript.length());
        parser.setScriptBuilder(&tsb);
        if (!parser.parse()) {
            vacation.valid = false;
            return vacation;
        }
    }

    vacation.sendForSpam = !sdx.found();
    vacation.excludeDomain = drdx.domainName();
    vacation.startDate = dx.startDate();
    vacation.startTime = dx.startTime();
    vacation.endDate = dx.endDate();
    vacation.endTime = dx.endTime();

    if (vacation.sendForSpam && vacation.excludeDomain.isEmpty() && !vacation.startDate.isValid() && !vacation.endDate.isValid()) {
        const auto vac = parseScriptLegacy(script);
        if (vac.isValid()) {
            vacation.sendForSpam = vac.sendForSpam;
            vacation.excludeDomain = vac.excludeDomain;
            vacation.startDate = vac.startDate;
            vacation.startTime = vac.startTime;
            vacation.endDate = vac.endDate;
            vacation.endTime = vac.endTime;
        }
    }

    return vacation;
}

QString KSieveUi::VacationUtils::composeScript(const Vacation &vacation)
{
    QStringList condition;
    QStringList require;

    require << QStringLiteral("vacation");

    if (vacation.startDate.isValid() || vacation.endDate.isValid()) {
        require << QStringLiteral("date");
        require << QStringLiteral("relational");
    }

    if (vacation.startDate.isValid()) {
        if (vacation.startTime.isValid()) {
            const QDateTime start(vacation.startDate, vacation.startTime);
            condition.append(QStringLiteral("currentdate :value \"ge\" \"iso8601\" \"%1\"")
                             .arg(start.toString(Qt::ISODate)));
        } else {
            condition.append(QStringLiteral("currentdate :value \"ge\" \"date\" \"%1\"")
                             .arg(vacation.startDate.toString(Qt::ISODate)));
        }
    }

    if (vacation.endDate.isValid()) {
        if (vacation.endTime.isValid()) {
            const QDateTime end(vacation.endDate, vacation.endTime);
            condition.append(QStringLiteral("currentdate :value \"le\" \"iso8601\" \"%1\"")
                             .arg(end.toString(Qt::ISODate)));
        } else {
            condition.append(QStringLiteral("currentdate :value \"le\" \"date\" \"%1\"")
                             .arg(vacation.endDate.toString(Qt::ISODate)));
        }
    }

    if (!vacation.sendForSpam) {
        condition.append(QStringLiteral("not header :contains \"X-Spam-Flag\" \"YES\""));
    }

    if (!vacation.excludeDomain.isEmpty()) {
        condition.append(QStringLiteral("address :domain :contains \"from\" \"%1\"").arg(vacation.excludeDomain));
    }

    QString addressesArgument;
    QStringList aliases;
    if (!vacation.aliases.empty()) {
        addressesArgument += QStringLiteral(":addresses [ ");
        QStringList sl;
        AddrSpecList::const_iterator end = vacation.aliases.constEnd();
        for (AddrSpecList::const_iterator it = vacation.aliases.begin(); it != end; ++it) {
            sl.push_back(QLatin1Char('"') + (*it).asString().replace(QLatin1Char('\\'), QStringLiteral("\\\\")).replace(QLatin1Char('"'), QStringLiteral("\\\"")) + QLatin1Char('"'));
            aliases.push_back((*it).asString());
        }
        addressesArgument += sl.join(QStringLiteral(", ")) + QStringLiteral(" ] ");
    }

    QString sVacation(QStringLiteral("vacation "));
    sVacation += addressesArgument;
    if (vacation.notificationInterval > 0) {
        sVacation += QStringLiteral(":days %1 ").arg(vacation.notificationInterval);
    }

    if (!vacation.subject.trimmed().isEmpty()) {
        sVacation += QStringLiteral(":subject \"%1\" ").arg(stringReplace(vacation.subject).trimmed());
    }

    sVacation += QStringLiteral("text:\n");
    sVacation += dotstuff(vacation.messageText.isEmpty() ? VacationUtils::defaultMessageText() : vacation.messageText);
    sVacation += QStringLiteral("\n.\n;");

    switch (vacation.mailAction) {
    case VacationUtils::Keep:
        break;
    case VacationUtils::Discard:
        sVacation += QStringLiteral("\ndiscard;");
        break;
    case VacationUtils::Sendto:
        sVacation += QStringLiteral("\nredirect \"") + vacation.mailActionRecipient + QStringLiteral("\";");
        break;
    case VacationUtils::CopyTo:
        require << QStringLiteral("copy");
        sVacation += QStringLiteral("\nredirect :copy \"") + vacation.mailActionRecipient + QStringLiteral("\";");
        break;
    }

    QString script = QStringLiteral("require [\"%1\"];\n\n").arg(require.join(QStringLiteral("\", \"")));

    if (condition.isEmpty()) {
        if (vacation.active) {
            script += sVacation;
        } else {
            script += QStringLiteral("if false\n{\n\t");
            script += sVacation;
            script += QStringLiteral("\n}");
        }
    } else {
        if (vacation.active) {
            script += QStringLiteral("if allof(%1)\n{\n\t").arg(condition.join(QStringLiteral(", ")));
        } else {
            script += QStringLiteral("if false # allof(%1)\n{\n\t").arg(condition.join(QStringLiteral(", ")));
        }
        script += sVacation;
        script += QStringLiteral("\n}");
    }

    script += QStringLiteral("\n");

    return script;
}

QString KSieveUi::VacationUtils::mergeRequireLine(const QString &script, const QString &scriptUpdate)
{
    const QByteArray scriptUTF8 = script.trimmed().toUtf8();
    if (scriptUTF8.isEmpty()) {
        return scriptUpdate;
    }

    const QByteArray scriptUpdateUTF8 = scriptUpdate.trimmed().toUtf8();
    if (scriptUpdateUTF8.isEmpty()) {
        return script;
    }

    KSieve::Parser parser(scriptUTF8.begin(),
                          scriptUTF8.begin() + scriptUTF8.length());
    KSieve::Parser parserUpdate(scriptUpdateUTF8.begin(),
                                scriptUpdateUTF8.begin() + scriptUpdateUTF8.length());
    RequireExtractor rx, rxUpdate;
    parser.setScriptBuilder(&rx);
    parserUpdate.setScriptBuilder(&rxUpdate);

    int insert(0);
    QStringList lines = script.split(QLatin1Char('\n'));
    QSet<QString> requirements;

    if (parser.parse() && rx.commandFound()) {
        insert = rx.lineStart();
        const int endOld(rx.lineEnd());
        for (int i = insert; i <= endOld; ++i) {
            lines.removeAt(insert);
        }
        requirements = rx.requirements().toSet();
    }

    if (parserUpdate.parse() && rxUpdate.commandFound()) {
        requirements += rxUpdate.requirements().toSet();
    }

    const int requirementscount = requirements.count();
    if (requirementscount > 1) {
        QStringList req = requirements.toList();
        req.sort();
        lines.insert(insert, QStringLiteral("require [\"%1\"];").arg(req.join(QStringLiteral("\", \""))));
    } else if (requirementscount == 1) {
        lines.insert(insert, QStringLiteral("require \"%1\";").arg(requirements.toList().at(0)));
    }

    return lines.join(QStringLiteral("\n"));
}

QString KSieveUi::VacationUtils::updateVacationBlock(const QString &oldScript, const QString &newScript)
{
    const QByteArray oldScriptUTF8 = oldScript.trimmed().toUtf8();
    if (oldScriptUTF8.isEmpty()) {
        return newScript;
    }

    const QByteArray newScriptUTF8 = newScript.trimmed().toUtf8();
    if (newScriptUTF8.isEmpty()) {
        return oldScript;
    }

    KSieve::Parser parserOld(oldScriptUTF8.begin(),
                             oldScriptUTF8.begin() + oldScriptUTF8.length());
    KSieve::Parser parserNew(newScriptUTF8.begin(),
                             newScriptUTF8.begin() + newScriptUTF8.length());
    VacationDataExtractor vdxOld, vdxNew;
    RequireExtractor rx;
    KSieveExt::MultiScriptBuilder tsb(&vdxOld, &rx);
    parserOld.setScriptBuilder(&tsb);
    parserNew.setScriptBuilder(&vdxNew);

    int startOld(0);

    QStringList lines = oldScript.split(QLatin1Char('\n'));

    QString script;
    if (parserOld.parse() && vdxOld.commandFound()) {
        startOld = vdxOld.lineStart();
        const int endOld(vdxOld.lineEnd());
        for (int i = startOld; i <= endOld; ++i) {
            lines.removeAt(startOld);
        }
    } else {
        if (rx.commandFound()) {                // after require
            startOld = rx.lineEnd() + 1;
        } else {
            startOld = 0;
        }
    }

    if (parserNew.parse() && vdxNew.commandFound()) {
        const int startNew(vdxNew.lineStart());
        const int endNew(vdxNew.lineEnd());
        QStringList linesNew = newScript.split(QLatin1Char('\n'));
        for (int i = endNew; i >= startNew; --i) {
            lines.insert(startOld, linesNew.at(i));
        }
    }

    return lines.join(QStringLiteral("\n"));
}
