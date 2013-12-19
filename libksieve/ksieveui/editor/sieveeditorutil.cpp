/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "sieveeditorutil.h"

#include <QDebug>

QString KSieveUi::SieveEditorUtil::helpUrl(KSieveUi::SieveEditorUtil::HelpVariableName type)
{
    switch (type) {
    case AddressCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5228#page-16");
    case BodyCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5173");
    case ConvertCondition:
        return QString(); //TODO
    case CurrentdateCondition:
        return QLatin1String("http://tools.ietf.org/search/rfc5260#page-6");
    case DateCondition:
        return QLatin1String("http://tools.ietf.org/search/rfc5260#page-4");
    case EnvelopeCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-24");
    case EnvironmentCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5183");
    case ExistsCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-25");
    case FalseCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-25");
    case HasflagCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5232#page-5");
    case HeaderCondition:
        return QString(); //TODO
    case IhaveCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5463");
    case MailboxexistsCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5490#page-2");
    case MetadataexistsCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5490#page-6");
    case MetadataCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5490#page-5");
    case ServermetadataexistsCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5490#page-4");
    case ServermetadataCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc5490#page-5");
    case SizeCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-26");
    case SpamtestCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc3685#page-3");
    case TrueCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-26");
    case VirustestCondition:
        return QLatin1String("http://tools.ietf.org/html/rfc3685#page-4");
    case AbstracteditheaderAction:
        return QString(); //TODO
    case AddflagsAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5232#page-5");
    case AddheaderAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5293");
    case BreakAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5703#page-3");
    case ConvertAction:
        return QString(); //TODO
    case DeleteheaderAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5293");
    case DiscardAction:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-22");
    case EncloseAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5703#page-10");
    case ExtracttextAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5703#page-11");
    case FileintoAction:
        return QLatin1String("http://tools.ietf.org/search/rfc3028#page-20");
    case KeepAction:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-21");
    case NotifyAction:
        return QLatin1String("http://tools.ietf.org/search/rfc5435#page-3");
    case RedirectAction:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-20");
    case RejectAction:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-20");
    case RemoveflagsAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5232#page-5");
    case ReplaceAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5703#page-8");
    case ReturnAction:
        return QLatin1String("http://tools.ietf.org/html/rfc6609#page-7");
    case SetflagsAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5232#page-4");
    case SetvariableAction:
        return QLatin1String("http://tools.ietf.org/search/rfc5229");
    case StopAction:
        return QLatin1String("http://tools.ietf.org/html/rfc3028#page-19");
    case VacationAction:
        return QLatin1String("http://tools.ietf.org/html/rfc5230#page-3");
    default:
        break;
    }
    return QString();
}


KSieveUi::SieveEditorUtil::HelpVariableName KSieveUi::SieveEditorUtil::strToVariableName(const QString &str)
{
    if (str == QLatin1String("address")) {
        return AddressCondition;
    } else if (str == QLatin1String("body")) {
        return BodyCondition;
    } else if (str == QLatin1String("currentdate")) {
        return CurrentdateCondition;
    } else if (str == QLatin1String("date")) {
        return DateCondition;
    } else if (str == QLatin1String("envelope")) {
        return EnvelopeCondition;
    } else if (str == QLatin1String("environment")) {
        return EnvironmentCondition;
    } else if (str == QLatin1String("exists")) {
        return ExistsCondition;
    } else if (str == QLatin1String("false")) {
        return FalseCondition;
    } else if (str == QLatin1String("hasflag")) {
        return HasflagCondition;
    }
    //TODO
    return VacationAction;
}
