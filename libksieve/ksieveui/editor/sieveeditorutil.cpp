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
    case BodyCondition:
    case ConvertCondition:
    case CurrentdateCondition:
    case DateCondition:
    case EnvelopeCondition:
    case EnvironmentCondition:
    case ExistsCondition:
    case FalseCondition:
    case HasflagCondition:
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
    //TODO
    return VacationAction;
}
