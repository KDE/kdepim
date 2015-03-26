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

QStringList KSieveUi::VacationUtils::defaultMailAliases()
{
    QStringList sl;
    KPIMIdentities::IdentityManager manager( true );
    KPIMIdentities::IdentityManager::ConstIterator end(manager.end());
    for ( KPIMIdentities::IdentityManager::ConstIterator it = manager.begin(); it != end ; ++it ) {
        if ( !(*it).primaryEmailAddress().isEmpty() ) {
            sl.push_back( (*it).primaryEmailAddress() );
        }
        sl += (*it).emailAliases();
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


bool KSieveUi::VacationUtils::parseScript( const QString &script, bool &active, QString &messageText,
                            QString &subject,
                            int & notificationInterval, QStringList &aliases,
                            bool & sendForSpam, QString &domainName,
                            QDate & startDate, QDate & endDate )
{
    if ( script.trimmed().isEmpty() ) {
        active = false;
        messageText = VacationUtils::defaultMessageText();
        subject = VacationUtils::defaultSubject();
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
        active = false;
        return false;
    }
    active = vdx.active();
    messageText = vdx.messageText().trimmed();
    if (!vdx.subject().isEmpty()) {
        subject = vdx.subject().trimmed();
    }
    notificationInterval = vdx.notificationInterval();
    aliases = vdx.aliases();

    if (!active && !vdx.ifComment().isEmpty()) {
        const QByteArray newScript = QString::fromAscii("if ").toUtf8() + vdx.ifComment().toUtf8() + QString::fromLatin1("{vacation;}").toUtf8();
        tsb = KSieveExt::MultiScriptBuilder( &sdx, &drdx, &dx );
        KSieve::Parser parser( newScript.begin(),
                           newScript.begin() + newScript.length() );
        parser.setScriptBuilder( &tsb );
        if ( !parser.parse() ) {
            return false;
        }
    }

    sendForSpam = !sdx.found();
    domainName = drdx.domainName();
    startDate = dx.startDate();
    endDate = dx.endDate();
    return true;
}

bool KSieveUi::VacationUtils::foundVacationScript(const QString &script)
{
    const QByteArray scriptUTF8 = script.trimmed().toUtf8();
    kDebug() << "scriptUtf8 = \"" + scriptUTF8 +"\"";

    if (scriptUTF8.isEmpty()) {
      return false;
    }

    KSieve::Parser parser( scriptUTF8.begin(),
                           scriptUTF8.begin() + scriptUTF8.length() );
    VacationDataExtractor vdx;
    parser.setScriptBuilder(&vdx);
    return parser.parse() && vdx.commandFound();
}

bool KSieveUi::VacationUtils::vacationScriptActive(const QString &script)
{
    const QByteArray scriptUTF8 = script.trimmed().toUtf8();
    kDebug() << "scriptUtf8 = \"" + scriptUTF8 +"\"";

    if (scriptUTF8.isEmpty()) {
      return false;
    }

    KSieve::Parser parser( scriptUTF8.begin(),
                           scriptUTF8.begin() + scriptUTF8.length() );
    VacationDataExtractor vdx;
    parser.setScriptBuilder(&vdx);
    return parser.parse() && vdx.commandFound() && vdx.active();
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

QString KSieveUi::VacationUtils::composeScript( const QString & messageText, bool active,
                                 const QString &subject,
                                 int notificationInterval,
                                 const AddrSpecList & addrSpecs,
                                 bool sendForSpam, const QString & domain,
                                 const QDate & startDate, const QDate & endDate )
{
    QStringList condition;

    if (startDate.isValid()) {
        condition.append(QString::fromLatin1("currentdate :value \"ge\" \"date\" \"%1\"")
            .arg(startDate.toString(Qt::ISODate)));
    }

    if (endDate.isValid()) {
        condition.append(QString::fromLatin1("currentdate :value \"le\" \"date\" \"%1\"")
            .arg(endDate.toString(Qt::ISODate)));
    }

    if (!sendForSpam) {
        condition.append(QString::fromLatin1("not header :contains \"X-Spam-Flag\" \"YES\""));
    }

    if (!domain.isEmpty()) {
        condition.append(QString::fromLatin1("address :domain :contains \"from\" \"%1\"").arg( domain ));
    }

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

    QString vacation(QLatin1String("vacation "));
    vacation += addressesArgument;
    if ( notificationInterval > 0 )
        vacation += QString::fromLatin1(":days %1 ").arg(notificationInterval);

    if (!subject.trimmed().isEmpty()) {
        vacation += QString::fromLatin1(":subject \"%1\" ").arg(stringReplace(subject).trimmed());
    }

    vacation += QString::fromLatin1("text:\n");
    vacation += dotstuff( messageText.isEmpty() ? VacationUtils::defaultMessageText() : messageText );
    vacation += QString::fromLatin1( "\n.\n;" );

    QString script;

    if ( startDate.isValid() || endDate.isValid() ) {
        script = QString::fromLatin1("require [\"vacation\", \"relational\", \"date\"];\n\n" );
    } else {
        script = QString::fromLatin1("require \"vacation\";\n\n" );
    }

    if (condition.count() == 0) {
        if (active) {
            script += vacation;
        } else {
            script += QString::fromLatin1("if false\n{\n\t");
            script += vacation;
            script += QLatin1String("\n}");
        }
    } else {
        if (active) {
            script += QString::fromLatin1("if allof(%1)\n{\n\t").arg(condition.join(QLatin1String(", ")));
        } else {
            script += QString::fromLatin1("if false # allof(%1)\n{\n\t").arg(condition.join(QLatin1String(", ")));
        }
        script += vacation;
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

    int startNew(vdxNew.lineStart());
    int endNew(vdxNew.lineEnd());

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
