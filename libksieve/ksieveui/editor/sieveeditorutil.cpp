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

#include "sieveeditorutil.h"

QString KSieveUi::SieveEditorUtil::helpUrl(KSieveUi::SieveEditorUtil::HelpVariableName type)
{
    switch (type) {
    case AddressCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5228#page-16");
    case BodyCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5173");
    case ConvertCondition:
        return QString(); //TODO
    case CurrentdateCondition:
        return QStringLiteral("http://tools.ietf.org/search/rfc5260#page-6");
    case DateCondition:
        return QStringLiteral("http://tools.ietf.org/search/rfc5260#page-4");
    case EnvelopeCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-24");
    case EnvironmentCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5183");
    case ExistsCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-25");
    case FalseCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-25");
    case HasflagCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5232#page-5");
    case HeaderCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5228#page-9");
    case IhaveCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5463");
    case MailboxexistsCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5490#page-2");
    case MetadataexistsCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5490#page-6");
    case MetadataCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5490#page-5");
    case ServermetadataexistsCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5490#page-4");
    case ServermetadataCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc5490#page-5");
    case SizeCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-26");
    case SpamtestCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc3685#page-3");
    case TrueCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-26");
    case VirustestCondition:
        return QStringLiteral("http://tools.ietf.org/html/rfc3685#page-4");
    case AbstracteditheaderAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5228#page-9");
    case AddflagsAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5232#page-5");
    case AddheaderAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5293");
    case BreakAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5703#page-3");
    case ConvertAction:
        return QString(); //TODO
    case DeleteheaderAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5293");
    case DiscardAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-22");
    case EncloseAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5703#page-10");
    case ExtracttextAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5703#page-11");
    case FileintoAction:
        return QStringLiteral("http://tools.ietf.org/search/rfc3028#page-20");
    case KeepAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-21");
    case NotifyAction:
        return QStringLiteral("http://tools.ietf.org/search/rfc5435#page-3");
    case RedirectAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-20");
    case RejectAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-20");
    case RemoveflagsAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5232#page-5");
    case ReplaceAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5703#page-8");
    case ReturnAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc6609#page-7");
    case SetflagsAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5232#page-4");
    case SetvariableAction:
        return QStringLiteral("http://tools.ietf.org/search/rfc5229");
    case StopAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc3028#page-19");
    case VacationAction:
        return QStringLiteral("http://tools.ietf.org/html/rfc5230#page-3");
    case GlobalVariable:
        return QStringLiteral("http://tools.ietf.org/search/rfc5229");
    case Includes:
        return QStringLiteral("http://tools.ietf.org/html/rfc6609#page-4");
    case ForEveryPart:
        return QStringLiteral("http://tools.ietf.org/html/rfc5703#page-3");
    case CopyExtension:
        return QStringLiteral("http://tools.ietf.org/html/rfc3894");
    case MBoxMetaDataExtension:
        return QStringLiteral("http://tools.ietf.org/html/rfc5490#page-2");
    case SubAddressExtension:
        return QStringLiteral("https://tools.ietf.org/html/rfc5233");
    case UnknownHelp:
        break;
    }
    return QString();
}

KSieveUi::SieveEditorUtil::HelpVariableName KSieveUi::SieveEditorUtil::strToVariableName(const QString &str)
{
    if (str == QStringLiteral("address")) {
        return AddressCondition;
    } else if (str == QStringLiteral("body")) {
        return BodyCondition;
    } else if (str == QStringLiteral("currentdate")) {
        return CurrentdateCondition;
    } else if (str == QStringLiteral("date")) {
        return DateCondition;
    } else if (str == QStringLiteral("envelope")) {
        return EnvelopeCondition;
    } else if (str == QStringLiteral("environment")) {
        return EnvironmentCondition;
    } else if (str == QStringLiteral("exists")) {
        return ExistsCondition;
    } else if (str == QStringLiteral("false")) {
        return FalseCondition;
    } else if (str == QStringLiteral("hasflag")) {
        return HasflagCondition;
    } else if (str == QStringLiteral("header")) {
        return HeaderCondition;
    } else if (str == QStringLiteral("ihave")) {
        return IhaveCondition;
    } else if (str == QStringLiteral("mailboxexists")) {
        return MailboxexistsCondition;
    } else if (str == QStringLiteral("metadata")) {
        return MetadataCondition;
    } else if (str == QStringLiteral("metadataexists")) {
        return MetadataexistsCondition;
    } else if (str == QStringLiteral("servermetadata")) {
        return ServermetadataCondition;
    } else if (str == QStringLiteral("servermetadataexists")) {
        return ServermetadataexistsCondition;
    } else if (str == QStringLiteral("size")) {
        return SizeCondition;
    } else if (str == QStringLiteral("spamtest")) {
        return SpamtestCondition;
    } else if (str == QStringLiteral("true")) {
        return TrueCondition;
    } else if (str == QStringLiteral("virustest")) {
        return VirustestCondition;
    } else if (str == QStringLiteral("break")) {
        return BreakAction;
    } else if (str == QStringLiteral("convert")) {
        return ConvertAction; //TODO
    } else if (str == QStringLiteral("discard")) {
        return DiscardAction;
    } else if (str == QStringLiteral("enclose")) {
        return EncloseAction;
    } else if (str == QStringLiteral("extracttext")) {
        return ExtracttextAction;
    } else if (str == QStringLiteral("fileinto")) {
        return FileintoAction;
    } else if (str == QStringLiteral("keep")) {
        return KeepAction;
    } else if (str == QStringLiteral("notify")) {
        return NotifyAction;
    } else if (str == QStringLiteral("redirect")) {
        return RedirectAction;
    } else if (str == QStringLiteral("reject")) {
        return RejectAction;
    } else if (str == QStringLiteral("replace")) {
        return ReplaceAction;
    } else if (str == QStringLiteral("return")) {
        return ReturnAction;
    } else if (str == QStringLiteral("set")) {
        return SetvariableAction;
    } else if (str == QStringLiteral("stop")) {
        return StopAction;
    } else if (str == QStringLiteral("vacation")) {
        return VacationAction;
    } else if (str == QStringLiteral("include")) {
        return Includes;
    } else if (str == QStringLiteral("globalvariable")) {
        return GlobalVariable;
    } else if (str == QStringLiteral("foreverypart")) {
        return ForEveryPart;
    } else if (str == QStringLiteral("copy")) {
        return CopyExtension;
    } else if (str == QStringLiteral("mboxmetadata")) {
        return MBoxMetaDataExtension;
    } else if (str == QStringLiteral("subaddress")) {
        return SubAddressExtension;
    }

    //TODO
    return UnknownHelp;
}
