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
#include <KPIMIdentities/IdentityManager>
#include <KPIMIdentities/Identity>

#include <KLocalizedString>
#include <KLocale>
#include <KGlobal>
#include <QDate>

using KMime::Types::AddrSpecList;

static inline QString dotstuff( QString s ) { // krazy:exclude=passbyvalue
    if ( s.startsWith( QLatin1Char('.') ) )
        return QLatin1Char('.') + s.replace( QLatin1String("\n."), QLatin1String("\n..") );
    else
        return s.replace( QLatin1String("\n."), QLatin1String("\n..") );
}

static inline QString stringReplace(QString s)
{
    s = s.replace(QRegExp(QLatin1String("[\n\t]+")),QLatin1String(" "));
    return s.replace(QLatin1Char('\"'),QLatin1String("\\\""));
}

QString KSieveUi::VacationUtils::defaultSubject()
{
  return i18n("Out of office till %1", QLocale().toString(QDate::currentDate().addDays(1)));
}

QString KSieveUi::VacationUtils::defaultMessageText() {
    return i18n( "I am out of office till %1.\n"
                 "\n"
                 "In urgent cases, please contact Mrs. \"vacation replacement\"\n"
                 "\n"
                 "email: \"email address of vacation replacement\"\n"
                 "phone: +49 711 1111 11\n"
                 "fax.:  +49 711 1111 12\n"
                 "\n"
                 "Yours sincerely,\n"
                 "-- \"enter your name and email address here\"\n",
                 KGlobal::locale()->formatDate( QDate::currentDate().addDays( 1 ) ) );
}

int KSieveUi::VacationUtils::defaultNotificationInterval() {
    return 7; // days
}

KMime::Types::AddrSpecList KSieveUi::VacationUtils::defaultMailAliases()
{
    KMime::Types::AddrSpecList sl;
    KPIMIdentities::IdentityManager manager( true );
    KPIMIdentities::IdentityManager::ConstIterator end(manager.end());
    for ( KPIMIdentities::IdentityManager::ConstIterator it = manager.begin(); it != end ; ++it ) {
        if ( !(*it).primaryEmailAddress().isEmpty() ) {
            KMime::Types::Mailbox a;
            a.fromUnicodeString((*it).primaryEmailAddress());
            sl.push_back(a.addrSpec());
        }
        foreach(const QString &email, (*it).emailAliases()) {
            KMime::Types::Mailbox a;
            a.fromUnicodeString(email);
            sl.push_back(a.addrSpec());
        }
    }

    return sl;
}

bool KSieveUi::VacationUtils::defaultSendForSpam() {
    return VacationSettings::outOfOfficeReactToSpam();
}

QString KSieveUi::VacationUtils::defaultDomainName() {
    return VacationSettings::outOfOfficeDomain();
}

QDate KSieveUi::VacationUtils::defaultStartDate()
{
    return QDate::currentDate();
}


QDate KSieveUi::VacationUtils::defaultEndDate()
{
    return defaultStartDate().addDays(7);
}


KSieveUi::VacationUtils::Vacation KSieveUi::VacationUtils::parseScript(const QString &script)
{
    KSieveUi::VacationUtils::Vacation vacation;
    if ( script.trimmed().isEmpty() ) {
        vacation.valid = false;
        vacation.active = false;
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
    kDebug() << "scriptUtf8 = \"" + scriptUTF8 +"\"";
    KSieve::Parser parser( scriptUTF8.begin(),
                           scriptUTF8.begin() + scriptUTF8.length() );
    VacationDataExtractor vdx;
    SpamDataExtractor sdx;
    DomainRestrictionDataExtractor drdx;
    DateExtractor dx;
    KSieveExt::MultiScriptBuilder tsb( &vdx , &sdx, &drdx, &dx );
    parser.setScriptBuilder( &tsb );
    parser.parse();
    if ( !parser.parse() || !vdx.commandFound() ) {
        vacation.active = false;
        vacation.valid = false;
        return vacation;
    }
    vacation.valid = true;
    vacation.active = vdx.active();
    vacation.messageText = vdx.messageText().trimmed();
    if (!vdx.subject().isEmpty()) {
        vacation.subject = vdx.subject().trimmed();
    }
    vacation.notificationInterval = vdx.notificationInterval();
    vacation.aliases = KMime::Types::AddrSpecList();
    foreach(const QString &alias, vdx.aliases()) {
        KMime::Types::Mailbox a;
        a.fromUnicodeString(alias);
        vacation.aliases.append(a.addrSpec());
    }

    if (!vacation.active && !vdx.ifComment().isEmpty()) {
        const QByteArray newScript = QString::fromAscii("if ").toUtf8() + vdx.ifComment().toUtf8() + QString::fromLatin1("{vacation;}").toUtf8();
        tsb = KSieveExt::MultiScriptBuilder( &sdx, &drdx, &dx );
        KSieve::Parser parser( newScript.begin(),
                           newScript.begin() + newScript.length() );
        parser.setScriptBuilder( &tsb );
        if ( !parser.parse() ) {
            vacation.valid = false;
            return vacation;
        }
    }

    vacation.sendForSpam = !sdx.found();
    vacation.excludeDomain = drdx.domainName();
    vacation.startDate = dx.startDate();
    vacation.endDate = dx.endDate();
    return vacation;
}

QString composeOldScript( const QString & messageText,
                                 const QString &subject,
                                 int notificationInterval,
                                 const AddrSpecList & addrSpecs,
                                 bool sendForSpam, const QString & domain,
                                 const QDate & startDate, const QDate & endDate )
{
    QString addressesArgument;
    QStringList aliases;
    if ( !addrSpecs.empty() ) {
        addressesArgument += QLatin1String(":addresses [ ");
        QStringList sl;
        AddrSpecList::const_iterator end = addrSpecs.constEnd();
        for ( AddrSpecList::const_iterator it = addrSpecs.begin() ; it != end; ++it ) {
            sl.push_back( QLatin1Char('"') + (*it).asString().replace( QLatin1Char('\\'), QLatin1String("\\\\") ).replace( QLatin1Char('"'), QLatin1String("\\\"") ) + QLatin1Char('"') );
            aliases.push_back( (*it).asString() );
        }
        addressesArgument += sl.join( QLatin1String(", ") ) + QLatin1String(" ] ");
    }
    QString script;

    if ( startDate.isValid() && endDate.isValid() ) {
        script = QString::fromLatin1("require [\"vacation\", \"relational\", \"date\"];\n\n" );
    } else {
        script = QString::fromLatin1("require \"vacation\";\n\n" );
    }
    if ( !sendForSpam )
        script += QString::fromLatin1( "if header :contains \"X-Spam-Flag\" \"YES\""
                                       " { keep; stop; }\n" ); // FIXME?

    if ( !domain.isEmpty() ) // FIXME
        script += QString::fromLatin1( "if not address :domain :contains \"from\" \"%1\" { keep; stop; }\n" ).arg( domain );

    if ( startDate.isValid() && endDate.isValid() ) {
        script += QString::fromLatin1( "if not allof(currentdate :value \"ge\" \"date\" \"%1\","
                                       " currentdate :value \"le\" \"date\" \"%2\")"
                                       " { keep; stop; }\n" ).arg( startDate.toString(Qt::ISODate),
                                                                   endDate.toString(Qt::ISODate) );
    }

    script += QLatin1String("vacation ");
    script += addressesArgument;
    if ( notificationInterval > 0 )
        script += QString::fromLatin1(":days %1 ").arg( notificationInterval );

    if (!subject.trimmed().isEmpty()) {
        script += QString::fromLatin1(":subject \"%1\" ").arg(stringReplace(subject).trimmed());
    }

    script += QString::fromLatin1("text:\n");
    script += dotstuff( messageText.isEmpty() ? KSieveUi::VacationUtils::defaultMessageText() : messageText );
    script += QString::fromLatin1( "\n.\n;\n" );
    return script;
}

QString KSieveUi::VacationUtils::composeScript(const Vacation &vacation)
{
    QStringList condition;

    if (vacation.startDate.isValid()) {
        condition.append(QString::fromLatin1("currentdate :value \"ge\" \"date\" \"%1\"")
            .arg(vacation.startDate.toString(Qt::ISODate)));
    }

    if (vacation.endDate.isValid()) {
        condition.append(QString::fromLatin1("currentdate :value \"le\" \"date\" \"%1\"")
            .arg(vacation.endDate.toString(Qt::ISODate)));
    }

    if (!vacation.sendForSpam) {
        condition.append(QString::fromLatin1("not header :contains \"X-Spam-Flag\" \"YES\""));
    }

    if (!vacation.excludeDomain.isEmpty()) {
        condition.append(QString::fromLatin1("address :domain :contains \"from\" \"%1\"").arg( vacation.excludeDomain ));
    }

    QString addressesArgument;
    QStringList aliases;
    if ( !vacation.aliases.empty() ) {
        addressesArgument += QLatin1String(":addresses [ ");
        QStringList sl;
        AddrSpecList::const_iterator end = vacation.aliases.constEnd();
        for ( AddrSpecList::const_iterator it = vacation.aliases.begin() ; it != end; ++it ) {
            sl.push_back( QLatin1Char('"') + (*it).asString().replace( QLatin1Char('\\'), QLatin1String("\\\\") ).replace( QLatin1Char('"'), QLatin1String("\\\"") ) + QLatin1Char('"') );
            aliases.push_back( (*it).asString() );
        }
        addressesArgument += sl.join( QLatin1String(", ") ) + QLatin1String(" ] ");
    }

    QString sVacation(QLatin1String("vacation "));
    sVacation += addressesArgument;
    if ( vacation.notificationInterval > 0 )
        sVacation += QString::fromLatin1(":days %1 ").arg(vacation.notificationInterval);

    if (!vacation.subject.trimmed().isEmpty()) {
        sVacation += QString::fromLatin1(":subject \"%1\" ").arg(stringReplace(vacation.subject).trimmed());
    }

    sVacation += QString::fromLatin1("text:\n");
    sVacation += dotstuff( vacation.messageText.isEmpty() ? VacationUtils::defaultMessageText() : vacation.messageText );
    sVacation += QString::fromLatin1( "\n.\n;" );

    QString script;

    if ( vacation.startDate.isValid() || vacation.endDate.isValid() ) {
        script = QString::fromLatin1("require [\"vacation\", \"relational\", \"date\"];\n\n" );
    } else {
        script = QString::fromLatin1("require \"vacation\";\n\n" );
    }

    if (condition.count() == 0) {
        if (vacation.active) {
            script += sVacation;
        } else {
            script += QString::fromLatin1("if false\n{\n\t");
            script += sVacation;
            script += QLatin1String("\n}");
        }
    } else {
        if (vacation.active) {
            script += QString::fromLatin1("if allof(%1)\n{\n\t").arg(condition.join(QLatin1String(", ")));
        } else {
            script += QString::fromLatin1("if false # allof(%1)\n{\n\t").arg(condition.join(QLatin1String(", ")));
        }
        script += sVacation;
        script += QLatin1String("\n}");
    }

    script += QLatin1String("\n");

    return script;
}


QString KSieveUi::VacationUtils::mergeRequireLine(const QString &script, const QString scriptUpdate)
{
    const QByteArray scriptUTF8 = script.trimmed().toUtf8();
    const QByteArray scriptUpdateUTF8 = scriptUpdate.trimmed().toUtf8();

    if (scriptUTF8.isEmpty()) {
      return scriptUpdate;
    }

    if (scriptUpdateUTF8.isEmpty()) {
        return script;
    }

    KSieve::Parser parser( scriptUTF8.begin(),
                           scriptUTF8.begin() + scriptUTF8.length() );
    KSieve::Parser parserUpdate( scriptUpdateUTF8.begin(),
                           scriptUpdateUTF8.begin() + scriptUpdateUTF8.length() );
    RequireExtractor rx, rxUpdate;
    parser.setScriptBuilder(&rx);
    parserUpdate.setScriptBuilder(&rxUpdate);

    int insert(0);
    QStringList lines = script.split(QLatin1Char('\n'));
    QSet<QString> requirements;

    if (parser.parse() && rx.commandFound()) {
        insert = rx.lineStart();
        const int endOld(rx.lineEnd());
        for (int i=insert; i<=endOld; i++) {
            lines.removeAt(insert);
        }
        requirements = rx.requirements().toSet();
    }

    if (parserUpdate.parse() && rxUpdate.commandFound()) {
        requirements += rxUpdate.requirements().toSet();
    }

    if (requirements.count() > 1) {
        QStringList req = requirements.toList();
        req.sort();
        lines.insert(insert, QString::fromLatin1("require [\"%1\"];").arg(req.join(QLatin1String("\", \""))));
    } else if  (requirements.count() == 1) {
        lines.insert(insert, QString::fromLatin1("require \"%1\";").arg(requirements.toList().first()));
    }

    return lines.join(QLatin1String("\n"));
}

QString KSieveUi::VacationUtils::updateVacationBlock(const QString &oldScript, const QString &newScript)
{
    const QByteArray oldScriptUTF8 = oldScript.trimmed().toUtf8();
    const QByteArray newScriptUTF8 = newScript.trimmed().toUtf8();

    if (oldScriptUTF8.isEmpty()) {
      return newScript;
    }

    if (newScriptUTF8.isEmpty()) {
        return oldScript;
    }

    KSieve::Parser parserOld( oldScriptUTF8.begin(),
                           oldScriptUTF8.begin() + oldScriptUTF8.length() );
    KSieve::Parser parserNew( newScriptUTF8.begin(),
                           newScriptUTF8.begin() + newScriptUTF8.length() );
    VacationDataExtractor vdxOld, vdxNew;
    RequireExtractor rx;
    KSieveExt::MultiScriptBuilder tsb( &vdxOld , &rx );
    parserOld.setScriptBuilder(&tsb);
    parserNew.setScriptBuilder(&vdxNew);

    int startOld(0);

    QStringList lines = oldScript.split(QLatin1Char('\n'));

    QString script;
    if (parserOld.parse() && vdxOld.commandFound()) {
        startOld = vdxOld.lineStart();
        const int endOld(vdxOld.lineEnd());
        for (int i=startOld; i<=endOld; i++) {
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
        for(int i=endNew;i>=startNew;i--) {
            lines.insert(startOld, linesNew.at(i));
        }
    }

    return lines.join(QLatin1String("\n"));
}
