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
    case IhaveCondition:
    case MailboxexistsCondition:
    case MetadataexistsCondition:
    case MetadataCondition:
    case ServermetadataexistsCondition:
    case ServermetadataCondition:
    case SizeCondition:
    case SpamtestCondition:
    case TrueCondition:
    case VirustestCondition:
    case AbstracteditheaderAction:
    case AbstractflagsAction:
    case AddflagsAction:
    case AddheaderAction:
    case BreakAction:
    case ConvertAction:
    case DeleteheaderAction:
    case DiscardAction:
    case EncloseAction:
    case ExtracttextAction:
    case FileintoAction:
    case KeepAction:
    case NotifyAction:
    case RedirectAction:
    case RejectAction:
    case RemoveflagsAction:
    case ReplaceAction:
    case ReturnAction:
    case SetflagsAction:
    case SetvariableAction:
    case StopAction:
    case VacationAction:
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
